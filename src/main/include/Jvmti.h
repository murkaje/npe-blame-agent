#pragma once

#include <tuple>
#include <jvmti.h>

#include "util.h"
#include "exceptionCallback.h"
#include "Method.h"

//TODO:Add mutex
//TODO:Specialized error messages for each method?
//TODO: split regular wrapped calls from ones returning compound types and additional transformation?
class Jvmti {

  inline static jvmtiEnv *env = nullptr;
  inline static JNIEnv *jni = nullptr;
  inline static std::shared_ptr<spdlog::logger> logger = getLogger("JVMTI");

  static void checkError(jvmtiError err) {
    if (err != JVMTI_ERROR_NONE) {
      char *errorChars = nullptr;
      env->GetErrorName(err, &errorChars);
      std::string errorString{errorChars};
      env->Deallocate((unsigned char *) errorChars);
      throw std::runtime_error("JVMTI invocation failed: " + errorString);
    }
  }

  static void JNICALL vmInit(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread) {
    jni = jni_env;
  }

public:

  static void init(JavaVM *vm);

  static bool isMethodNative(jmethodID method);

  //TODO: return frame object (Method(class,name,sig,modifier),location)
  static std::pair<jmethodID, jlocation> getFrameLocation(jthread thread, unsigned int depth);

  static jclass getMethodDeclaringClass(jmethodID method);

  static std::vector<uint8_t> getBytecodes(jmethodID method);

  static ConstPool getConstPool(jclass klass);

  static uint32_t getMethodModifiers(jmethodID methodId);

  //replaces GetMethodName, otherwise we create temporary string copies
  static Method getMethod(jmethodID methodId);

  static uint8_t getMethodArgumentsSize(jmethodID methodId);

  static LocalVariableTable getLocalVariableTable(jmethodID methodId);

  static uint32_t getFrameCount(jthread thread);
};