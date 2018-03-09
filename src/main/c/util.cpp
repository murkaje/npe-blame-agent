#include <cstdarg>
#include <algorithm>
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

std::string toJavaClassName(const std::string &jvmClassName) {
  std::string javaClassName = jvmClassName;
  std::replace(javaClassName.begin(), javaClassName.end(), '/', '.');
  return javaClassName;
}

std::string toJavaTypeName(const std::string &jvmTypeName, size_t startPos, size_t *outEndPos) {
  if (jvmTypeName.empty()) { throw std::invalid_argument("Empty string is not a signature"); }

  size_t pos = startPos;
  uint8_t arrayDim = 0;

  //pos+1 is checking that after [ at least 1 letter will follow for next switch block
  while (jvmTypeName[pos] == '[' && pos + 1 < jvmTypeName.size()) {
    arrayDim++;
    pos++;
  }

  std::string basicType;
  switch (jvmTypeName[pos]) {
    case 'L': {
      size_t end = jvmTypeName.find(';', pos + 1);
      if (end == std::string::npos) { throw std::invalid_argument("Malformed class signature, did not find ';'"); }
      basicType = jvmTypeName.substr(pos + 1, end - pos - 1);
      std::replace(basicType.begin(), basicType.end(), '/', '.');

      pos += basicType.size() + 2;

      break;
    }
    case 'V':
      basicType = "void";
      pos++;
      break;
    case 'B':
      basicType = "byte";
      pos++;
      break;
    case 'I':
      basicType = "int";
      pos++;
      break;
    case 'J':
      basicType = "long";
      pos++;
      break;
    case 'Z':
      basicType = "bool";
      pos++;
      break;
    case 'C':
      basicType = "char";
      pos++;
      break;
    case 'D':
      basicType = "double";
      pos++;
      break;
    case 'F':
      basicType = "float";
      pos++;
      break;
    case 'S':
      basicType = "short";
      pos++;
      break;
    default:
      throw std::invalid_argument("Unknown type letter " + jvmTypeName.substr(pos, pos));
  }

  if (outEndPos != nullptr) {
    *outEndPos = pos;
  }

  if (arrayDim == 0) {
    return basicType;
  }

  std::stringstream ss;
  ss << basicType;
  for (size_t count = 0; count < arrayDim; count++) {
    ss << "[]";
  }

  return ss.str();
}

// (Ljava/lang/Class;IIZ)V -> void (Class, int, int, bool)
std::string parseMethodSignature(const std::string &signature, const std::string &methodName) {
  size_t pos = 0;

  if (signature.empty() || signature[pos] != '(') {
    throw std::invalid_argument("Signature must begin with '('");
  }
  pos++;

  std::stringstream ss;

  bool first = true;
  std::string type;
  while (pos < signature.size()) {
    if (signature[pos] == ')') {
      pos++;
      continue;
    }
    type = toJavaTypeName(signature, pos, &pos);

    if (pos != signature.size()) {
      if (first) {
        first = false;
      }
      else {
        ss << ", ";
      }
      ss << type;
    }

  }

  return type + " " + methodName + "(" + ss.str() + ")";

}

Method::Method(const std::string &methodName, const std::string &signature) : methodName(methodName) {
  size_t pos = 0;

  if (signature.empty() || signature[pos] != '(') {
    throw std::invalid_argument("Signature must begin with '('");
  }
  pos++;

  std::string type;
  while (pos < signature.size()) {
    if (signature[pos] == ')') {
      pos++;
      continue;
    }
    type = toJavaTypeName(signature, pos, &pos);

    if (pos != signature.size()) {
      parameterTypes.push_back(type);
    }
    else {
      returnType = type;
    }

  }
}
