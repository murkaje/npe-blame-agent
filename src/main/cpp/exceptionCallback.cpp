#include "exceptionCallback.h"

#include <string>
#include <map>
#include <iterator>
#include <spdlog.h>

#include "Method.h"
#include "analyzer.h"
#include "util.h"

using std::string;
using std::vector;

static auto logger = getLogger("ExceptionCallback");

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
    return false;
  }
  return true;
}

void JNICALL callback_Exception(jvmtiEnv *jvmti,
                                JNIEnv *jni,
                                jthread thread,
                                jmethodID method,
                                jlocation location,
                                jobject exception,
                                jmethodID catch_method,
                                jlocation catch_location) {

  jvmtiError err;
  jboolean nativeMethod;

  err = jvmti->IsMethodNative(method, &nativeMethod);
  check_jvmti_error(jvmti, err, "Is method native");

  if (nativeMethod || location == 0) {
    return;
  }

  jclass exceptionClass = jni->GetObjectClass(exception);
  string exceptionClassName = getClassName(jni, exceptionClass);
  string exceptionMessage = getExceptionMessage(jni, exception);

  if (exceptionClassName != "java.lang.NullPointerException") { return; }

  // If NPE has a message, e.g. when explicitly thrown, don't overwrite it
//  if (!exceptionMessage.empty()) { return; }

  auto[methodName, methodSignature] = getMethodNameAndSignature(jvmti, method);

  jclass methodClass;
  err = jvmti->GetMethodDeclaringClass(method, &methodClass);
  check_jvmti_error(jvmti, err, "Get method declaring class");

  string methodClassName = getClassName(jni, methodClass);

  //TODO: JDK9 compiles implicit Objects.requireNonNUll for indy/inner constructor - make this method intrinsic and analyze method in previous frame
  if (methodClassName == "java.util.Objects" && methodName == "requireNonNull") {
    jvmti->GetFrameLocation(thread, 1, &method, &location);

    std::tie(methodName, methodSignature) = getMethodNameAndSignature(jvmti, method);
    err = jvmti->GetMethodDeclaringClass(method, &methodClass);
    check_jvmti_error(jvmti, err, "Get method declaring class");
    methodClassName = getClassName(jni, methodClass);
  }

  vector<uint8_t> methodBytecode = getMethodBytecode(jvmti, method);

  auto[cpCount, constPoolBytes] = getConstPool(jvmti, methodClass);

  LocalVariableTable localVariables(jvmti, method);

  ConstPool constPool(constPoolBytes);
  CodeAttribute codeAttribute(methodBytecode, localVariables);
  InstructionPrintIterator iter(codeAttribute, constPool);

//  if (!shouldPrint(codeAttribute, methodName, methodSignature, location)) { return; }

  jint modifiers;
  err = jvmti->GetMethodModifiers(method, &modifiers);
  check_jvmti_error(jvmti, err, "Get method modifiers");

  //struct/class StackFrame(Method, location)?
  Method currentFrameMethod = Method{methodClassName, methodName, methodSignature, (modifiers & Modifier::STATIC) !=0};
  std::string exceptionDetail = describeNPEInstruction(currentFrameMethod, constPool, codeAttribute, localVariables, location);

  jstring detailMessage = jni->NewStringUTF(exceptionDetail.c_str());
  jfieldID detailMessageField = jni->GetFieldID(exceptionClass, "detailMessage", "Ljava/lang/String;");
  jni->SetObjectField(exception, detailMessageField, detailMessage);
  jni->DeleteLocalRef(detailMessage);

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