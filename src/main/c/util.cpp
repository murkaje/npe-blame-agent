#include <cstdarg>
#include "util.h"

#include "logging.h"

static Logger log("Util");

void check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum, const std::string &actionDescription) {
  if (errnum != JVMTI_ERROR_NONE) {
    char *errnum_str = nullptr;
    jvmti->GetErrorName(errnum, &errnum_str);

    log.error() << errnum << "(" << errnum_str << "): " << actionDescription << std::endl;

    jvmti->Deallocate((unsigned char *) errnum_str);
  }
}

bool checkJniException(JNIEnv *jni, const std::string &actionDescription) {
  if (jni->ExceptionCheck()) {
    log.error(actionDescription);
    jni->ExceptionDescribe();
    jni->ExceptionClear();
    return true;
  }

  return false;
}

std::string jstringToString(JNIEnv *jni, jstring str) {
  if (str == nullptr) {
    return std::string();
  }

  //Returns length without null terminator space
  jint stringByteLength = jni->GetStringUTFLength(str) + 1;
  jint stringLength = jni->GetStringLength(str);

  char *cstring = (char *) malloc(stringByteLength);
  jni->GetStringUTFRegion(str, 0, stringLength, cstring);
  std::string retval(cstring);
  free(cstring);

  return retval;
}

std::tuple<std::string, std::string> getMethodNameAndSignature(jvmtiEnv *jvmti, jmethodID method) {
  uint8_t *methodName;
  uint8_t *methodSignature;
  jvmtiError err;

  err = jvmti->GetMethodName(method, (char **) &methodName, (char **) &methodSignature, NULL);
  check_jvmti_error(jvmti, err, "Get method name");

  auto nameAndSignature = std::make_tuple(std::string((char *) methodName), std::string((char *) methodSignature));

  err = jvmti->Deallocate(methodName);
  check_jvmti_error(jvmti, err, "Deallocate methodName");
  err = jvmti->Deallocate(methodSignature);
  check_jvmti_error(jvmti, err, "Deallocate methodSignature");

  return nameAndSignature;
};

std::string getClassName(JNIEnv *jni, jclass klass) {
  jmethodID getClassID = jni->GetMethodID(klass, "getClass", "()Ljava/lang/Class;");

  jclass cls = (jclass) jni->CallObjectMethod(klass, getClassID);

  //Unfortunately calling this directly on class returns null
  jmethodID getNameID = jni->GetMethodID(cls, "getName", "()Ljava/lang/String;");

  jstring className = (jstring) jni->CallObjectMethod(klass, getNameID);

  return jstringToString(jni, className);
}

std::string getExceptionMessage(JNIEnv *jni, jobject exception) {
  jclass exceptionClass = jni->GetObjectClass(exception);

  jmethodID getMessageID = jni->GetMethodID(exceptionClass, "getMessage", "()Ljava/lang/String;");

  if (checkJniException(jni, "Getting getMessage method from exception")) {
    return std::string();
  }

  jstring detailMessage = (jstring) jni->CallObjectMethod(exception, getMessageID);

  return jstringToString(jni, detailMessage);
}

std::tuple<jint, std::vector<uint8_t>> getConstPool(jvmtiEnv *jvmti, jclass klass) {
  jint cpCount;
  jint cpByteSize;
  uint8_t *constPoolBytes;
  jvmtiError err;

  err = jvmti->GetConstantPool(klass, &cpCount, &cpByteSize, &constPoolBytes);
  check_jvmti_error(jvmti, err, "Get constant pool");
  std::vector<uint8_t> constPool(constPoolBytes, constPoolBytes + cpByteSize);
  err = jvmti->Deallocate(constPoolBytes);
  check_jvmti_error(jvmti, err, "Deallocate constPool");

  return std::make_tuple(cpCount, constPool);
}

std::vector<uint8_t> getMethodBytecode(jvmtiEnv *jvmti, jmethodID method) {
  jvmtiError err;

  jint bytecodeLength;
  uint8_t *bytecodes;

  err = jvmti->GetBytecodes(method, &bytecodeLength, &bytecodes);
  check_jvmti_error(jvmti, err, "Get method bytecodes");

  std::vector<uint8_t> methodBytecode(bytecodes, bytecodes + bytecodeLength);

  err = jvmti->Deallocate(bytecodes);
  check_jvmti_error(jvmti, err, "Deallocate bytecodes");

  return methodBytecode;
}

std::string formatString(const char *format, ...) {
  std::va_list args, temp;
  va_start(args, format);
  va_copy(temp, args);
  size_t length = (size_t) (vsnprintf(nullptr, 0, format, temp));
  va_end(temp);

  std::string formattedString(length, '\0');
  if (std::vsnprintf(&formattedString[0], formattedString.size() + 1, format, args) < 0) {
    throw std::runtime_error("Failed to encode string: " + std::string(format));
  }
  return formattedString;
}

/**
 *
 * @param signature
 * @return Java type declaration
 */
std::string parseMethodSignature(const std::string &signature) {
  return "";
}
