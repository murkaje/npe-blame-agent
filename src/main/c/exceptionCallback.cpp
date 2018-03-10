#include "exceptionCallback.h"

#include <string>
#include <map>
#include <iterator>
#include <Analyzer.h>

#include "LocalVariableTable.h"
#include "Constants.h"
#include "ConstPool.h"
#include "CodeAttribute.h"
#include "util.h"
#include "logging.h"

using std::string;
using std::vector;

static Logger log("ExceptionCallback");

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
  if (codeAttribute.getOpcode((size_t) location) == ATHROW) {

    InstructionIterator iter(codeAttribute);

    // Create a visitPattern(N, array<N> instructions, lamba) -> offset begin
    /*
     * matcher[aload*(n), aconst_null, ...]
     * if(matcher.matches()) ...
     */
    for (; iter.getOffset() + 11 < codeAttribute.getSize(); iter++) {

      log.debug(formatString("Current opcode: %s", Constants::OpcodeMnemonic[std::get<1>(*iter)]));

      // Record current stack top via visitor?

      if (std::get<1>(*iter) != ACONST_NULL) { continue; }
      log.debug("aconst_null");
      iter++;
      if (std::get<1>(*iter) != IF_ACMPNE) { continue; }
      log.debug("if_acmpne");
      iter++;
      if (std::get<1>(*iter) != NEW) { continue; }
      log.debug("new");
      iter++;
      if (std::get<1>(*iter) != DUP) { continue; }
      log.debug("dup");
      iter++;
      if (std::get<1>(*iter) != INVOKESPECIAL) { continue; }
      log.debug("invokespecial");
      iter++;
      if (std::get<1>(*iter) != ATHROW) { continue; }

      log.debug("Method checks for null explicitly");
      return true;
    }

    log.debug(formatString("Skipping explicit throw site %s%s", methodName.c_str(), methodSignature.c_str()));
//    return false;
  }
  return true;
}

std::string getExceptionMessage(const ConstPool &constPool, const CodeAttribute &codeAttribute, size_t location) {
  std::string exceptionDetail;
  if (auto op = codeAttribute.getOpcode(location); op >= OpCodes::INVOKEVIRTUAL && op <= OpCodes::INVOKEDYNAMIC) {
    Method method = Method::readFromCodeInvoke(codeAttribute, constPool, location);
    std::string sourceMessage = traceInvokeInstance(constPool, codeAttribute, location);
    exceptionDetail = "Invoking instance method " + method.getClassName() + "#" + method.getMethodName() + " on null " + sourceMessage;
  }
  else {
    exceptionDetail = "NOT INVOKE";
  }
  return exceptionDetail;
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

//  jint lineTableLength;
//  jvmtiLineNumberEntry *lineNumberTable;
//
//  err = jvmti->GetLineNumberTable(method, &lineTableLength, &lineNumberTable);
//  if (err != JVMTI_ERROR_ABSENT_INFORMATION) {
//    check_jvmti_error(jvmti, err, "Get LineNumberTable");
//    err = jvmti->Deallocate((unsigned char *) lineNumberTable);
//    check_jvmti_error(jvmti, err, "Deallocate lineNumberTable");
//  }

  jclass methodClass;
  err = jvmti->GetMethodDeclaringClass(method, &methodClass);
  check_jvmti_error(jvmti, err, "Get method declaring class");

  jclass exceptionClass = jni->GetObjectClass(exception);

  string methodName;
  string methodSignature;
  std::tie(methodName, methodSignature) = getMethodNameAndSignature(jvmti, method);

  string methodClassName = getClassName(jni, methodClass);
  string exceptionClassName = getClassName(jni, exceptionClass);
  string exceptionMessage = getExceptionMessage(jni, exception);

//  log.debug("Exception class name: " + exceptionClassName);
  if (exceptionClassName != "java.lang.NullPointerException") { return; }

  vector<uint8_t> methodBytecode = getMethodBytecode(jvmti, method);

  jint cpCount;
  vector<uint8_t> constPoolBytes;
  std::tie(cpCount, constPoolBytes) = getConstPool(jvmti, methodClass);

  LocalVariableTable localVariables(jvmti, method);

  ConstPool constPool(constPoolBytes);
  CodeAttribute codeAttribute(methodBytecode, localVariables);
  InstructionPrintIterator iter(codeAttribute, constPool);


  if (!shouldPrint(codeAttribute, methodName, methodSignature, location)) { return; }

  /**
   * EXAMPLE: "while invoking instance method [Class&Method] on a null object [obtained from XYZ]"
   * Following options depending on where null originated
   * local variable is null
   *    display local variable name if debug info present
   *    slot number and type if not(bonus: track lvar lifetime, last method/aconst_null/etc that produced value)
   * chained(value only on stack, never stored in slot)
   *    display method call that pushed value to stack
   * method param
   *    param name and type
   *    type and index if debug info not present
   */

  std::string exceptionDetail = getExceptionMessage(constPool, codeAttribute, location);

  jstring detailMessage = jni->NewStringUTF(exceptionDetail.c_str());
  jfieldID detailMessageField = jni->GetFieldID(exceptionClass, "detailMessage", "Ljava/lang/String;");
  jni->SetObjectField(exception, detailMessageField, detailMessage);
  jni->DeleteLocalRef(detailMessage);

  log.info() << exceptionClassName << ": " << exceptionMessage << "\n"
             << formatString("\tat %s.%s[%d]%s", methodClassName.c_str(), methodName.c_str(), location, methodSignature.c_str()) << std::endl;

  size_t beginOffset = location > 200 ? location - 50 : 0;  //Limit printing too much
  std::ostream &out = log.info() << "Method instructions:\n";
  for (; iter.getOffset() < codeAttribute.getSize() && iter.getOffset() <= location; iter++) {
    if (iter.getOffset() >= beginOffset) {
      out << formatString("%-6d: %s\n", iter.getOffset(), (*iter).c_str());
//          out << std::setw(6) << std::setfill(' ') << std::left << iter.getOffset() << ": " << *iter << "\n";
    }
  }


//  log.debug("End exceptionCallback");
}