#include "exceptionCallback.h"

#include <string>
#include <map>
#include <iterator>
#include <spdlog.h>

#include "analyzer.h"
#include "LocalVariableTable.h"
#include "Constants.h"
#include "ConstPool.h"
#include "CodeAttribute.h"
#include "util.h"
#include "logging.h"

using std::string;
using std::vector;

static auto logger = getLogger("ExceptionCallback");

/*
 * Some ideas:
 *  Minimize so that native agent only adds extra field to java.lang.Exception, other code to java
 *  If our logger prints exception, see if we have additional info(valid to start griffin without native) and print it
 *
 *  Check if exception is caught in same method/class, print in this case even if opcode[location] == ATHROW as it probably is
 *  if(var == null) {
 *    throw new NullPointerException();
 *  }
 */

//TODO: Maybe something better as it could be a useful if(someVar == null) { throw new NullPointerException(); }
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
bool shouldPrint(const CodeAttribute &codeAttribute, const string &methodName, const string methodSignature, jlocation location) {
  if (codeAttribute.getOpcode((size_t) location) == OpCodes::ATHROW) {

    InstructionIterator iter(codeAttribute);

    // Create a visitPattern(N, array<N> instructions, lamba) -> offset begin
    /*
     * matcher[aload*(n), aconst_null, ...]
     * if(matcher.matches()) ...
     */
    for (; iter.getOffset() + 11 < codeAttribute.getSize(); iter++) {

      logger->debug(formatString("Current opcode: %s", Constants::OpcodeMnemonic[std::get<1>(*iter)]));

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

    logger->debug(formatString("Skipping explicit throw site %s%s", methodName.c_str(), methodSignature.c_str()));
//    return false;
  }
  return true;
}

std::string getExceptionMessage(const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location) {
  return describeNPEInstruction(cp, code, vars, location);
}

void JNICALL callback_Exception(jvmtiEnv *jvmti,
                                JNIEnv *jni,
                                jthread thread,
                                jmethodID method,
                                jlocation location,
                                jobject exception,
                                jmethodID catch_method,
                                jlocation catch_location) {
//  logger->set_level(spdlog::level::trace);

  jvmtiError err;
  jboolean nativeMethod;

  err = jvmti->IsMethodNative(method, &nativeMethod);
  check_jvmti_error(jvmti, err, "Is method native");

  if (nativeMethod || location == 0) {
    return;
  }

  jclass methodClass;
  err = jvmti->GetMethodDeclaringClass(method, &methodClass);
  check_jvmti_error(jvmti, err, "Get method declaring class");

  jclass exceptionClass = jni->GetObjectClass(exception);

  auto [methodName, methodSignature] = getMethodNameAndSignature(jvmti, method);

  string methodClassName = getClassName(jni, methodClass);
  string exceptionClassName = getClassName(jni, exceptionClass);
  string exceptionMessage = getExceptionMessage(jni, exception);

  if (exceptionClassName != "java.lang.NullPointerException") { return; }

  //TODO: JDK9 compiles implicit Objects.requireNonNUll for indy/inner constructor - make this method intrinsic and pop a stack frame

  // If NPE has a message, e.g. when explicitly thrown, don't overwrite it
//  if (!exceptionMessage.empty()) { return; }

  vector<uint8_t> methodBytecode = getMethodBytecode(jvmti, method);

  jint cpCount;
  vector<uint8_t> constPoolBytes;
  std::tie(cpCount, constPoolBytes) = getConstPool(jvmti, methodClass);

  LocalVariableTable localVariables(jvmti, method);

  ConstPool constPool(constPoolBytes);
  CodeAttribute codeAttribute(methodBytecode, localVariables);
  InstructionPrintIterator iter(codeAttribute, constPool);


  if (!shouldPrint(codeAttribute, methodName, methodSignature, location)) { return; }

  std::string exceptionDetail = getExceptionMessage(constPool, codeAttribute, localVariables, location);

  jstring detailMessage = jni->NewStringUTF(exceptionDetail.c_str());
  jfieldID detailMessageField = jni->GetFieldID(exceptionClass, "detailMessage", "Ljava/lang/String;");
  jni->SetObjectField(exception, detailMessageField, detailMessage);
  jni->DeleteLocalRef(detailMessage);

//  return;

  logger->debug("{}: {}", exceptionClassName, exceptionMessage);
  logger->debug("\tat {}.{}[{}]{}", methodClassName, methodName, location, methodSignature);

  size_t beginOffset = location > 200 ? location - 50 : 0;  //Limit printing too much
  logger->debug("Method instructions:");
  for (; iter.getOffset() < codeAttribute.getSize() /*&& iter.getOffset() <= location*/; iter++) {
    if (iter.getOffset() >= beginOffset) {
      logger->debug("{:<6}: {}", iter.getOffset(), (*iter));
    }
  }
}