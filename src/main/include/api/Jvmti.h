#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <jvmti.h>

#include "exceptions.h"
#include "util.h"
#include "bytecode/Method.h"
#include "api/Jni.h"

//TODO:Specialized error messages for each method?
//TODO: split regular wrapped calls from ones returning compound types and additional transformation?
class Jvmti {

  inline static thread_local jvmtiEnv *env = nullptr;
  inline static std::shared_ptr<spdlog::logger> logger = getLogger("JVMTI");

  static void checkError(jvmtiError err) {
    if (err != JVMTI_ERROR_NONE) {
      char *errorChars = nullptr;
      env->GetErrorName(err, &errorChars);
      std::string errorString{errorChars};
      env->Deallocate((unsigned char *) errorChars);
      //TODO separate jvmti error objects so callsite can catch by type and not verything?
      throw JvmtiError("JVMTI invocation failed: " + errorString);
    }
  }

  static void JNICALL vmInit(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread) {
    logger->debug("VMInit\n");
  }

public:

  static void ensureInit(jvmtiEnv *tenv) {
    env = tenv;
  }

  static void init(JavaVM *vm);

  //region JVMTI API wrappers

  static bool isMethodNative(jmethodID method);

  static std::pair<jmethodID, uint16_t> getFrameLocation(jthread thread, unsigned int depth);

  static jclass getMethodDeclaringClass(jmethodID method);

  static std::vector<uint8_t> getBytecodes(jmethodID method);

  static ConstPool getConstPool(jclass klass);

  static uint32_t getMethodModifiers(jmethodID methodId);

  static std::pair<std::string, std::string> getMethodNameAndSignature(jmethodID methodId);

  static uint8_t getMethodArgumentsSize(jmethodID methodId);

  static LocalVariableTable getLocalVariableTable(jmethodID methodId);

  static int32_t getLocalInt(jthread thread, uint16_t depth, uint8_t slot) {
    int32_t value;
    jvmtiError err = env->GetLocalInt(thread, depth, slot, &value);
    checkError(err);
    return value;
  }

  static int64_t getLocalLong(jthread thread, uint16_t depth, uint8_t slot) {
    int64_t value;
    jvmtiError err = env->GetLocalLong(thread, depth, slot, &value);
    checkError(err);
    return value;
  }

  static float getLocalFloat(jthread thread, uint16_t depth, uint8_t slot) {
    float value;
    jvmtiError err = env->GetLocalFloat(thread, depth, slot, &value);
    checkError(err);
    return value;
  }

  static float getLocalDouble(jthread thread, uint16_t depth, uint8_t slot) {
    double value;
    jvmtiError err = env->GetLocalDouble(thread, depth, slot, &value);
    checkError(err);
    return value;
  }

  static jobject getLocalObject(jthread thread, uint16_t depth, uint8_t slot) {
    jobject value;
    jvmtiError err = env->GetLocalObject(thread, depth, slot, &value);
    checkError(err);
    return value;
  }

  static jobject getLocalInstance(jthread thread, uint16_t depth) {
    jobject value;
    jvmtiError err = env->GetLocalInstance(thread, depth, &value);
    checkError(err);
    return value;
  }

  static uint32_t getFrameCount(jthread thread);

  //endregion

  //region Converters to bytecode types

  static Method toMethod(jmethodID methodId);

  //endregion

  //region Convenience functions

  static std::string localVariableToString(jthread thread, uint16_t depth, uint8_t slot, std::string_view signature) {
    TypeAndDim varType = typeOf(signature);

    if (varType.dim != 0) {
      jobject array = getLocalObject(thread, depth, slot);
      jclass arraysClass = Jni::getClass("java/util/Arrays");
      std::string result = Jni::invokeStatic(arraysClass, "deepToString", jnisig("([Ljava/lang/Object;)Ljava/lang/String;"), array);
      Jni::deleteLocalRef(array);
      Jni::deleteLocalRef(arraysClass);
      return result;
    } else if (varType.type == Type::IntType || varType.type == Type::ShortType || varType.type == Type::CharType ||
               varType.type == Type::ByteType || varType.type == Type::BoolType) {
      int32_t val = getLocalInt(thread, depth, slot);
      return varType.type == Type::BoolType ? (val != 0 ? "true" : "false") : std::to_string(val);
    } else if (varType.type == Type::LongType) {
      return std::to_string(getLocalLong(thread, depth, slot));
    } else if (varType.type == Type::FloatType) {
      return std::to_string(getLocalFloat(thread, depth, slot));
    } else if (varType.type == Type::DoubleType) {
      return std::to_string(getLocalDouble(thread, depth, slot));
    } else if (varType.type == Type::ObjectType || varType.type == Type::StringType) {
      jobject obj = getLocalObject(thread, depth, slot);
      return obj == nullptr ? "null" : Jni::invokeVirtual(obj, "toString", jnisig("()Ljava/lang/String;"));
    } else {
      throw JniError("Unknown variable type");
    }
  }

  //endregion
};