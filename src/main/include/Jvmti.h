#pragma once

#include <tuple>
#include <jvmti.h>

#include "exceptions.h"
#include "util.h"
#include "exceptionCallback.h"
#include "bytecode/Method.h"
#include "Jni.h"

using std::string;
using std::vector;
using std::pair;
using std::shared_ptr;

//TODO:Add mutex
//TODO:Specialized error messages for each method?
//TODO: split regular wrapped calls from ones returning compound types and additional transformation?
class Jvmti {

  inline static jvmtiEnv *env = nullptr;
  inline static JNIEnv *jni = nullptr;
  inline static shared_ptr<spdlog::logger> logger = getLogger("JVMTI");

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
    jni = jni_env;
    Jni::init(jni_env);
  }

public:

  static void init(JavaVM *vm);

  //region JVMTI API wrappers

  static bool isMethodNative(jmethodID method);

  static pair<jmethodID, uint16_t> getFrameLocation(jthread thread, unsigned int depth);

  static jclass getMethodDeclaringClass(jmethodID method);

  static vector<uint8_t> getBytecodes(jmethodID method);

  static ConstPool getConstPool(jclass klass);

  static uint32_t getMethodModifiers(jmethodID methodId);

  static pair<string, string> getMethodNameAndSignature(jmethodID methodId);

  static uint8_t getMethodArgumentsSize(jmethodID methodId);

  static LocalVariableTable getLocalVariableTable(jmethodID methodId);

  static uint32_t getFrameCount(jthread thread);

  //endregion

  //region Converters to bytecode types

  static Method toMethod(jmethodID methodId);

  //endregion

};