#include "util.h"

#include <sstream>
#include <spdlog.h>
#include <fmt/fmt.h>
#include <exceptions.h>
#include <Jni.h>

using fmt::literals::operator ""_format;

std::shared_ptr<spdlog::logger> getLogger(std::string_view loggerName) {
  auto log = spdlog::get(std::string(loggerName));
  if (!log) {
    log = spdlog::stdout_color_st(std::string(loggerName));
  }
  return log;
}

static auto logger = getLogger("Util");

void checkJniException(JNIEnv *jni, bool tryPrint) {
  jthrowable exception = jni->ExceptionOccurred();
  if (exception == nullptr) { return; }

  if (!tryPrint) {
    jni->ExceptionDescribe();
    jni->ExceptionClear();
    throw JniError("");
  }

  jni->ExceptionClear();

  jclass cls = jni->GetObjectClass(exception);
  checkJniException(jni, false);
  jclass jlClass = jni->FindClass("java/lang/Class");
  checkJniException(jni, false);
  jmethodID getNameId = jni->GetMethodID(jlClass, "getName", "()Ljava/lang/String;");
  checkJniException(jni, false);
  jstring className = (jstring) jni->CallObjectMethod(cls, getNameId);
  checkJniException(jni, false);
  jclass jlThrowable = jni->FindClass("java/lang/Throwable");
  checkJniException(jni, false);
  jmethodID getMessageId = jni->GetMethodID(jlThrowable, "getMessage", "()Ljava/lang/String;");
  checkJniException(jni, false);
  jstring exceptionMessage = (jstring) jni->CallObjectMethod(exception, getMessageId);
  checkJniException(jni, false);

  throw JniError(jstringToString(jni, className) + ": " + jstringToString(jni, exceptionMessage));
}

std::string jstringToString(JNIEnv *jni, jstring str) {
  if (str == nullptr) {
    return "";
  }

  const char *cstring = jni->GetStringUTFChars(str, nullptr);
  checkJniException(jni);

  std::string retval(cstring);

  jni->ReleaseStringUTFChars(str, cstring);
  checkJniException(jni);

  return retval;
}

std::string toJavaClassName(std::string_view jvmClassName) {
  std::string javaClassName{jvmClassName};
  std::replace(javaClassName.begin(), javaClassName.end(), '/', '.');
  return javaClassName;
}

std::string toJavaTypeName(std::string_view jvmTypeName, size_t startPos, size_t *outEndPos) {
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
      throw std::invalid_argument("Unknown type letter '{}'"_format(jvmTypeName.substr(pos, pos)));
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
std::string parseMethodSignature(std::string_view signature, std::string_view methodName) {
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

  return "{} {}({})"_format(type, methodName, ss.str());

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
