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

//thread_local bool recursiveJniException = false;

void checkJniException(JNIEnv *jni) {
  jthrowable exception = jni->ExceptionOccurred();
  if (exception == nullptr) { return; }
/*
  if (recursiveJniException) {
    jni->ExceptionDescribe();
    jni->ExceptionClear();
    recursiveJniException = false;
    throw JniError("Error while getting class and message for wrapped JNI error throw");
  }

  recursiveJniException = true;*/
  logger->warn("JNI Exception");
//  logger->warn("Exception toString: {}", objectToString(jni, exception));
  jni->ExceptionDescribe();
  jni->ExceptionClear();
  throw JniError("");
/*  std::string exceptionClassName = getObjectClassName(jni, exception);
  std::string exceptionMessage = getExceptionMessage(jni, exception);
  jni->ExceptionClear();
  recursiveJniException = false;

  throw JniError(exceptionClassName + ": " + exceptionMessage);*/
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
  cstring[stringByteLength-1] = 0;
  std::string retval(cstring);
  delete[] cstring;

  return retval;
}

std::string objectToString(JNIEnv *jni, jobject obj) {
  return Jni::invokeVirtual(obj, "toString", jnisig("()Ljava/lang/String;"));
//  jclass cls = jni->GetObjectClass(obj);
//  checkJniException(jni);
//
//  jmethodID toString = jni->GetMethodID(cls, "toString", "()Ljava/lang/String;");
//  checkJniException(jni);
//
//  jstring str = (jstring) jni->CallObjectMethod(obj, toString);
//  checkJniException(jni);
//
//  return jstringToString(jni, str);
}

std::string getObjectClassName(JNIEnv *jni, jobject obj) {
  jclass cls = jni->GetObjectClass(obj);
  checkJniException(jni);
  return getClassName(jni, cls);
}

std::string getClassName(JNIEnv *jni, jclass cls) {
  jclass javaLangClass = jni->FindClass("java/lang/Class");

  // doing GetMethodID on cls directly throws NoSuchMethodError, the fuck?
  jmethodID getNameID = jni->GetMethodID(javaLangClass, "getName", "()Ljava/lang/String;");
  checkJniException(jni);

  jstring className;

  className = (jstring) jni->CallObjectMethod(cls, getNameID);
  try {
    if (className == nullptr) {
//      logger->warn("Failed to call java.lang.Class#getName on '{}'", objectToString(jni, cls));
      logger->warn("Failed to call java.lang.Class#getName");
    }
    checkJniException(jni);
  } catch (const std::exception &e) {
    logger->warn("This is fucked up: {}", e.what());
    jni->ExceptionClear();
  }

  return jstringToString(jni, className);
}

std::string getExceptionMessage(JNIEnv *jni, jobject exception) {
  jclass throwableClass = jni->FindClass("java/lang/Throwable");
  checkJniException(jni);

  jmethodID getMessageID = jni->GetMethodID(throwableClass, "getMessage", "()Ljava/lang/String;");
  checkJniException(jni);

  jstring detailMessage;
  detailMessage = (jstring) jni->CallObjectMethod(exception, getMessageID);

  if(jni->ExceptionCheck()) {
    jni->ExceptionDescribe();
    jni->ExceptionClear();
    logger->warn("This is fucked up");
    return "";
  }

  return jstringToString(jni, detailMessage);
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
