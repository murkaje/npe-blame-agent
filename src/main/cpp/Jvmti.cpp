#include "Jvmti.h"

#include <util.h>

static void Jvmti::init(JavaVM *vm) {
  jvmtiEnv *initEnv = nullptr;

  jint errorCode = vm->GetEnv((void **) &initEnv, JVMTI_VERSION_1_1);
  if (errorCode != JNI_OK) {
    logger->error("Failed to acquire JVMTI 1.1 environment");
    throw std::runtime_error("Failed to acquire JVMTI 1.1 environment");
  }

  jvmtiEventCallbacks callbacks = {nullptr};
  jvmtiCapabilities caps = {0};

  caps.can_get_bytecodes = 1;
  caps.can_generate_exception_events = 1;
  caps.can_get_line_numbers = 1;
  caps.can_get_constant_pool = 1;
  caps.can_access_local_variables = 1;

  jvmtiError err = initEnv->AddCapabilities(&caps);
  checkError(err);

  callbacks.Exception = &exceptionCallback;
  callbacks.VMInit = &vmInit;

  err = initEnv->SetEventCallbacks(&callbacks, sizeof(callbacks));
  checkError(err);

  err = initEnv->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
  checkError(err);

  err = initEnv->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, nullptr);
  checkError(err);

  env = initEnv;
}

static bool Jvmti::isMethodNative(jmethodID method) {
  jboolean isNative;
  jvmtiError err = env->IsMethodNative(method, &isNative);
  checkError(err);

  return isNative;
}

static std::pair<jmethodID, jlocation> Jvmti::getFrameLocation(jthread thread, unsigned int depth) {
  jmethodID methodId;
  jlocation location;

  jvmtiError err = env->GetFrameLocation(thread, depth, &methodId, &location);
  checkError(err);

  return std::make_pair(methodId, location);
}

static jclass Jvmti::getMethodDeclaringClass(jmethodID method) {
  jclass methodClass;
  jvmtiError err = env->GetMethodDeclaringClass(method, &methodClass);
  checkError(err);

  return methodClass;
}

static std::vector<uint8_t> Jvmti::getBytecodes(jmethodID method) {
  jvmtiError err;

  jint length;
  uint8_t *bytes;

  err = env->GetBytecodes(method, &length, &bytes);
  checkError(err);
  std::vector<uint8_t> methodBytecode(bytes, bytes + length);
  err = env->Deallocate(bytes);
  checkError(err);

  return methodBytecode;
}

static ConstPool Jvmti::getConstPool(jclass klass) {
  jint cpCount;
  jint cpByteSize;
  uint8_t *constPoolBytes;

  jvmtiError err = env->GetConstantPool(klass, &cpCount, &cpByteSize, &constPoolBytes);
  checkError(err);
  ConstPool constPool(std::vector(constPoolBytes, constPoolBytes + cpByteSize));
  err = env->Deallocate(constPoolBytes);
  checkError(err);

  assert(cpCount == constPool.size());

  return constPool;
}

static int Jvmti::getMethodModifiers(jmethodID methodId) {
  int modifiers;
  jvmtiError err = env->GetMethodModifiers(methodId, &modifiers);
  checkError(err);
  return modifiers;
}

static Method Jvmti::getMethod(jmethodID methodId) {
  char *methodName;
  char *methodSignature;

  jvmtiError err = env->GetMethodName(methodId, &methodName, &methodSignature, nullptr);
  checkError(err);

  jclass methodClass = getMethodDeclaringClass(methodId);
  std::string methodClassName = getClassName(jni, methodClass);

  jint modifiers = getMethodModifiers(methodId);

  Method method{methodClassName, methodName, methodSignature, (((modifiers & Modifier::STATIC) != 0))};

  err = env->Deallocate((uint8_t *) methodName);
  checkError(err);
  err = env->Deallocate((uint8_t *) methodSignature);
  checkError(err);

  return method;
}