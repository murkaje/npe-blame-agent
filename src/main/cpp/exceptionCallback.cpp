#include "exceptionCallback.h"

#include <string>
#include <map>
#include <iterator>
#include <spdlog.h>
#include <backward.hpp>

#include "bytecode/Method.h"
#include "analyzer.h"
#include "util.h"
#include "api/Jvmti.h"
#include "api/Jni.h"

using std::string;
using std::vector;
using fmt::literals::operator ""_format;

static auto logger = getLogger("ExceptionCallback");

//TODO: Add info about exception table, e.g. bci | catchBci | op // comments
void printBytecode(jlocation location, const ConstPool &constPool, const CodeAttribute &codeAttribute) {
  InstructionPrintIterator iter(codeAttribute, constPool);

  logger->debug("Method instructions:");
  for (; iter.getOffset() < codeAttribute.getSize(); iter++) {
    auto level = (iter.getOffset() <= location) ? spdlog::level::debug : spdlog::level::trace;
    logger->log(level, "{:<6}: {}", iter.getOffset(), (*iter));
  }
}

/*
 * We could search the previous instructions for
 * new Ljava/lang/NullPointerException;
 *
 * Example forwards:
 * catch (Exception e) {
 *   astore x
 *   doSomething();
 *   aload x
 *   athrow
 * }
 *
 * public someForwardingMethod(Object a, Exception e) {
 *   ifnull x // a
 *   blabla
 * x:aload 2
 *   athrow
 * }
 *
 * Hidden exception warps(native calls, reflection)
 * catch(InvocationTargetException e) {
 *   throw e.getTargetException();
 * }
 */
bool shouldPrint(const CodeAttribute &codeAttribute, const string &methodName, const string &methodSignature,
                 jlocation location) {
  if (codeAttribute.getOpcode((size_t) location) == OpCodes::ATHROW) {

    InstructionIterator iter(codeAttribute);

    // Create a visitPattern(N, array<N> instructions, lamba) -> offset begin
    /*
     * matcher[aload*(n), aconst_null, ...]
     * if(matcher.matches()) ...
     */
    for (; iter.getOffset() + 11 < codeAttribute.getSize(); iter++) {

      logger->debug("Current opcode: {}", Constants::OpcodeMnemonic[std::get<1>(*iter)]);

      // Record current stack top via visitor?

      if (std::get<1>(*iter) != OpCodes::ACONST_NULL) { continue; }
      logger->debug("aconst_null");
      iter++;
      if (std::get<1>(*iter) != OpCodes::IF_ACMPNE) { continue; }
      logger->debug("if_acmpne");
      iter++;
      if (std::get<1>(*iter) != OpCodes::NEW) { continue; }
      logger->debug("new");
      iter++;
      if (std::get<1>(*iter) != OpCodes::DUP) { continue; }
      logger->debug("dup");
      iter++;
      if (std::get<1>(*iter) != OpCodes::INVOKESPECIAL) { continue; }
      logger->debug("invokespecial");
      iter++;
      if (std::get<1>(*iter) != OpCodes::ATHROW) { continue; }

      logger->debug("Method checks for null explicitly");
      return true;
    }

    logger->debug("Skipping explicit throw site {}{}", methodName.c_str(), methodSignature.c_str());
    return false;
  }
  return true;
}

void printMethodParams(jthread thread) {
  uint32_t frameCount = Jvmti::getFrameCount(thread);

  for (uint32_t depth = 0; depth < frameCount; depth++) {
    auto[methodId, location] = Jvmti::getFrameLocation(thread, depth);

    if (Jvmti::isMethodNative(methodId)) continue;

    uint8_t paramSlots = Jvmti::getMethodArgumentsSize(methodId);
    LocalVariableTable variables = Jvmti::getLocalVariableTable(methodId);
    Method method = Jvmti::toMethod(methodId);

    std::ostringstream oss;
    for (uint8_t slot = 0; slot < paramSlots; slot++) {
      if (auto optLocalVar = variables.getEntry(slot); optLocalVar.has_value()) {
        auto[name, type] = *optLocalVar;
        oss << name << "=" << "TODO" << ", ";
        if (type == "long" || type == "double") slot++;
      }
    }
    logger->trace("{}.{} args: [{}]", method.getClassName(), method.getMethodName(), oss.str());
  }
}

void JNICALL exceptionCallback(jvmtiEnv *jvmti,
                               JNIEnv *jni,
                               jthread thread,
                               jmethodID method,
                               jlocation location,
                               jobject exception,
                               jmethodID catch_method,
                               jlocation catch_location) {

  try {
    Jvmti::ensureInit(jvmti);
    Jni::ensureInit(jni);

    if (Jvmti::isMethodNative(method) || location == 0) { return; }

    jclass exceptionClass = Jni::getClass(exception);
    string exceptionClassName = Jni::invokeVirtual(exceptionClass, "getName", jnisig("()Ljava/lang/String;"));
    string exceptionMessage = Jni::getField(exception, "detailMessage", jnisig("Ljava/lang/String;"));

    if (exceptionClassName != "java.lang.NullPointerException") { return; }

    // If NPE has a message, e.g. when explicitly thrown, don't overwrite it
    if (!exceptionMessage.empty()) { return; }

    auto [methodName, signature] = Jvmti::getMethodNameAndSignature(method);
    jclass declaringClass = Jvmti::getMethodDeclaringClass(method);
    string declaringClassName = Jni::invokeVirtual(declaringClass, "getName", jnisig("()Ljava/lang/String;"));

    //JDK9+ compiles implicit Objects.requireNonNull before indy/inner constructor - analyze method in previous frame instead
    if (declaringClassName == "java.util.Objects" && methodName == "requireNonNull") {
      std::tie(method, location) = Jvmti::getFrameLocation(thread, 1);
      std::tie(methodName, signature) = Jvmti::getMethodNameAndSignature(method);
    }

    vector<uint8_t> methodBytecode = Jvmti::getBytecodes(method);
    ConstPool constPool = Jvmti::getConstPool(Jvmti::getMethodDeclaringClass(method));
    LocalVariableTable localVariables = Jvmti::getLocalVariableTable(method);
    CodeAttribute codeAttribute(methodBytecode, localVariables);

    logger->debug("{}: {}", exceptionClassName, exceptionMessage);
    logger->debug("\tat {}.{}[{}]{}", declaringClassName, methodName, location, signature);

    printBytecode(location, constPool, codeAttribute);

    string exceptionDetail = describeNPEInstruction(Jvmti::toMethod(method), constPool, codeAttribute, localVariables, location);
    Jni::putField(exception, "detailMessage", jnisig("Ljava/lang/String;"), exceptionDetail);

    printMethodParams(thread);
  } catch (const std::exception &e) {
    logger->error("Failed to run exception callback: {}", e.what());
  }
}