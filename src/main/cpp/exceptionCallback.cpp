#include "exceptionCallback.h"

#include <string>
#include <map>
#include <iterator>
#include <spdlog.h>
#include <backward.hpp>

#include "Method.h"
#include "analyzer.h"
#include "util.h"
#include "Jvmti.h"

using std::string;
using std::vector;
using fmt::literals::operator ""_format;

static auto logger = getLogger("ExceptionCallback");

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
bool shouldPrint(const CodeAttribute &codeAttribute, const string &methodName, const string methodSignature,
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

void JNICALL exceptionCallback(jvmtiEnv *jvmti,
                               JNIEnv *jni,
                               jthread thread,
                               jmethodID method,
                               jlocation location,
                               jobject exception,
                               jmethodID catch_method,
                               jlocation catch_location) {

  try {
    if (Jvmti::isMethodNative(method) || location == 0) { return; }

    jclass exceptionClass = jni->GetObjectClass(exception);
    string exceptionClassName = getClassName(jni, exceptionClass);
    string exceptionMessage = getExceptionMessage(jni, exception);

    if (exceptionClassName != "java.lang.NullPointerException") { return; }

    // If NPE has a message, e.g. when explicitly thrown, don't overwrite it
    if (!exceptionMessage.empty()) { return; }

    Method currentFrameMethod = Jvmti::getMethod(method);

    //JDK9 compiles implicit Objects.requireNonNull for indy/inner constructor - analyze method in previous frame instead
    if (currentFrameMethod.getClassName() == "java.util.Objects" &&
        currentFrameMethod.getMethodName() == "requireNonNull") {
      std::tie(method, location) = Jvmti::getFrameLocation(thread, 1);
      currentFrameMethod = Jvmti::getMethod(method);
    }

    vector<uint8_t> methodBytecode = Jvmti::getBytecodes(method);
    ConstPool constPool = Jvmti::getConstPool(Jvmti::getMethodDeclaringClass(method));
    LocalVariableTable localVariables(jvmti, method);
    CodeAttribute codeAttribute(methodBytecode, localVariables);

    logger->debug("{}: {}", exceptionClassName, exceptionMessage);
    logger->debug("\tat {}.{}[{}]{}", currentFrameMethod.getClassName(), currentFrameMethod.getMethodName(), location,
                  currentFrameMethod.getMethodSignature());

    printBytecode(location, constPool, codeAttribute);

    std::string exceptionDetail = describeNPEInstruction(currentFrameMethod, constPool, codeAttribute, localVariables,
                                                         location);

    jstring detailMessage = jni->NewStringUTF(exceptionDetail.c_str());
    jfieldID detailMessageField = jni->GetFieldID(exceptionClass, "detailMessage", "Ljava/lang/String;");
    jni->SetObjectField(exception, detailMessageField, detailMessage);
    jni->DeleteLocalRef(detailMessage);
  } catch (const std::exception &e) {
    logger->error("Failed to run exception callback: {}", e.what());
  }
}