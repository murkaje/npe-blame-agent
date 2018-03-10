#pragma once

#include <string>
#include <vector>
#include <cstdarg>

#include "ConstPool.h"
#include "CodeAttribute.h"

#include "jni.h"
#include "jvmti.h"

bool checkJniException(JNIEnv *jni, const std::string &actionDescription);

void check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum, const std::string &actionDescription);

std::string jstringToString(JNIEnv *jni, jstring str);

std::tuple<std::string,std::string> getMethodNameAndSignature(jvmtiEnv *jvmti, jmethodID method);

std::string getClassName(JNIEnv *jni, jclass klass);

std::string getExceptionMessage(JNIEnv *jni, jobject exception);

std::tuple<jint, std::vector<uint8_t>> getConstPool(jvmtiEnv *jvmti, jclass klass);

std::vector<uint8_t> getMethodBytecode(jvmtiEnv *jvmti, jmethodID method);

std::string formatString(const char *format, ...);

std::string toJavaClassName(const std::string &jvmClassName);

std::string toJavaTypeName(const std::string &jvmTypeName, size_t startPos, size_t *outEndPos);

std::string parseMethodSignature(const std::string &signature, const std::string &methodName);

uint8_t opcodeSlot(uint8_t opCode);

class Method {
  std::string className;
  std::string returnType;
  std::string methodName;
  std::vector<std::string> parameterTypes;

public:
  Method(std::string className, std::string methodName, const std::string &signature);

  static Method readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci);

  static Method readFromMemberRef(const ConstPool &constPool, size_t refId);

  const std::string &getClassName() const {
    return className;
  }

  const std::string &getReturnType() const {
    return returnType;
  }

  const std::string &getMethodName() const {
    return methodName;
  }

  const std::vector<std::string> &getParameterTypes() const {
    return parameterTypes;
  }

  const size_t getParameterCount() const {
    return parameterTypes.size();
  }
};

//TODO: Extract to cpp
class ByteVectorUtil {
public:

  static uint8_t readuint8(const std::vector<uint8_t> &vec, size_t pos) {
    return vec[pos];
  }

  static int8_t readint8(const std::vector<uint8_t> &vec, size_t pos) {
    return reinterpret_cast<const int8_t &>(vec[pos]);
  }

  static uint16_t readuint16(const std::vector<uint8_t> &vec, size_t pos) {
    uint16_t data = ((uint16_t) vec[pos] << 8) |
                    vec[pos + 1];
    return data;
  }

  static int16_t readint16(const std::vector<uint8_t> &vec, size_t pos) {
    uint16_t data = ((uint16_t) vec[pos] << 8) | vec[pos + 1];
    return reinterpret_cast<const int16_t &>(data);
  }

  static uint32_t readuint32(const std::vector<uint8_t> &vec, size_t pos) {
    uint32_t data = ((uint32_t) vec[pos] << 24) |
                    ((uint32_t) vec[pos + 1] << 16) |
                    ((uint32_t) vec[pos + 2] << 8) |
                    vec[pos + 3];
    return data;
  }

  static int32_t readint32(const std::vector<uint8_t> &vec, size_t pos) {
    uint32_t data = ((uint32_t) vec[pos] << 24) |
                    ((uint32_t) vec[pos + 1] << 16) |
                    ((uint32_t) vec[pos + 2] << 8) |
                    vec[pos + 3];
    return reinterpret_cast<const int32_t &>(data);
  }

  static uint64_t readuint64(const std::vector<uint8_t> &vec, size_t pos) {
    uint64_t data = ((uint64_t) vec[pos] << 56) |
                    ((uint64_t) vec[pos + 1] << 48) |
                    ((uint64_t) vec[pos + 2] << 40) |
                    ((uint64_t) vec[pos + 3] << 32) |
                    ((uint64_t) vec[pos + 4] << 24) |
                    ((uint64_t) vec[pos + 5] << 16) |
                    ((uint64_t) vec[pos + 6] << 8) |
                    vec[pos + 7];
    return data;
  }

  static int64_t readint64(const std::vector<uint8_t> &vec, size_t pos) {
    uint64_t data = ((uint64_t) vec[pos] << 56) |
                    ((uint64_t) vec[pos + 1] << 48) |
                    ((uint64_t) vec[pos + 2] << 40) |
                    ((uint64_t) vec[pos + 3] << 32) |
                    ((uint64_t) vec[pos + 4] << 24) |
                    ((uint64_t) vec[pos + 5] << 16) |
                    ((uint64_t) vec[pos + 6] << 8) |
                    vec[pos + 7];
    return reinterpret_cast<const int64_t &>(data);
  }

  static float readfloat(const std::vector<uint8_t> &vec, size_t pos) {
    uint32_t data = ((uint32_t) vec[pos] << 24) |
                    ((uint32_t) vec[pos + 1] << 16) |
                    ((uint32_t) vec[pos + 2] << 8) |
                    vec[pos + 3];
    return reinterpret_cast<const float &>(data);
  }

  static double readdouble(const std::vector<uint8_t> &vec, size_t pos) {
    uint64_t data = ((uint64_t) vec[pos] << 56) |
                    ((uint64_t) vec[pos + 1] << 48) |
                    ((uint64_t) vec[pos + 2] << 40) |
                    ((uint64_t) vec[pos + 3] << 32) |
                    ((uint64_t) vec[pos + 4] << 24) |
                    ((uint64_t) vec[pos + 5] << 16) |
                    ((uint64_t) vec[pos + 6] << 8) |
                    vec[pos + 7];
    return reinterpret_cast<const double &>(data);
  }
};