#include "util.h"

#include <sstream>
#include <spdlog.h>
#include <fmt/fmt.h>

using fmt::literals::operator""_format;

std::shared_ptr<spdlog::logger> getLogger(std::string_view loggerName) {
  auto log = spdlog::get(std::string(loggerName));
  if (!log) {
    log = spdlog::stdout_color_st(std::string(loggerName));
  }
  return log;
}

static auto logger = getLogger("Util");

void check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum, const std::string &actionDescription) {
  if (errnum != JVMTI_ERROR_NONE) {
    char *errnum_str = nullptr;
    jvmti->GetErrorName(errnum, &errnum_str);

    logger->error("{}({]): {}", errnum, errnum_str, actionDescription);

    jvmti->Deallocate((unsigned char *) errnum_str);
  }
}

bool checkJniException(JNIEnv *jni, const std::string &actionDescription) {
  if (jni->ExceptionCheck()) {
    logger->error(actionDescription);
    jni->ExceptionDescribe();
    jni->ExceptionClear();
    return true;
  }

  return false;
}

std::string jstringToString(JNIEnv *jni, jstring str) {
  if (str == nullptr) {
    return "";
  }

  //Returns length without null terminator space
  size_t stringByteLength = jni->GetStringUTFLength(str) + 1;
  size_t stringLength = jni->GetStringLength(str);

  char *cstring = new char[stringByteLength];
  jni->GetStringUTFRegion(str, 0, (jsize) stringLength, cstring);
  std::string retval(cstring);
  delete[] cstring;

  return retval;
}

std::tuple<std::string, std::string> getMethodNameAndSignature(jvmtiEnv *jvmti, jmethodID method) {
  char *methodName;
  char *methodSignature;
  jvmtiError err;

  err = jvmti->GetMethodName(method, &methodName, &methodSignature, nullptr);
  check_jvmti_error(jvmti, err, "Get method name");

  auto nameAndSignature = std::make_tuple(std::string(methodName), std::string(methodSignature));

  err = jvmti->Deallocate((uint8_t *) methodName);
  check_jvmti_error(jvmti, err, "Deallocate methodName");
  err = jvmti->Deallocate((uint8_t *) methodSignature);
  check_jvmti_error(jvmti, err, "Deallocate methodSignature");

  return nameAndSignature;
};

std::string getClassName(JNIEnv *jni, jclass klass) {
  jmethodID getClassID = jni->GetMethodID(klass, "getClass", "()Ljava/lang/Class;");

  jclass cls = (jclass) jni->CallObjectMethod(klass, getClassID);

  jmethodID getNameID = jni->GetMethodID(cls, "getName", "()Ljava/lang/String;");

  jstring className = (jstring) jni->CallObjectMethod(klass, getNameID);

  return jstringToString(jni, className);
}

std::string getExceptionMessage(JNIEnv *jni, jobject exception) {
  jclass exceptionClass = jni->GetObjectClass(exception);

  jmethodID getMessageID = jni->GetMethodID(exceptionClass, "getMessage", "()Ljava/lang/String;");

  if (checkJniException(jni, "Getting getMessage method from exception")) {
    logger->warn("Exception while calling Exception#getMessage");
    return "";
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
      } else {
        ss << ", ";
      }
      ss << type;
    }

  }

  return type + " " + methodName + "(" + ss.str() + ")";

}

const uint8_t loadStoreSlotLookupTable[] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, //0-9
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, //10-19
    9, 9, 9, 9, 9, 9, 0, 1, 2, 3, //20-29
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, //30-39
    2, 3, 0, 1, 2, 3, 9, 9, 9, 9, //40-49
    9, 9, 9, 9, 9, 9, 9, 9, 9, 0, //50-59
    1, 2, 3, 0, 1, 2, 3, 0, 1, 2, //60-69
    3, 0, 1, 2, 3, 0, 1, 2, 3, 9, //70-79
};

/**
 * Get local variable slot for 1 byte load/store opcodes e.g. ALOAD_1 -> 1
 */
uint8_t opcodeSlot(uint8_t opCode) {
  if (opCode >= OpCodes::ILOAD_0 && opCode <= OpCodes::ASTORE_3) {
    uint8_t val = loadStoreSlotLookupTable[opCode];
    if (val != 9) { return val; }
  }

  throw std::invalid_argument("Opcode is not a valid 1 byte load/store: {}"_format(Constants::OpcodeMnemonic[opCode]));
}
