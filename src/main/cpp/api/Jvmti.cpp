#include "api/Jvmti.h"

#include <util.h>
#include <jvmti.h>
#include <tuple>

#include "exceptionCallback.h"
#include "api/Jni.h"


void Jvmti::init(JavaVM *vm) {
  logger->info("OnLoad npe-blame-agent");

  jvmtiEnv *initEnv = nullptr;

  jint errorCode = vm->GetEnv((void **) &initEnv, JVMTI_VERSION_1_1);
  if (errorCode != JNI_OK) {
    logger->error("Failed to acquire JVMTI 1.1 environment");
    throw JvmtiError("Failed to acquire JVMTI 1.1 environment");
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

bool Jvmti::isMethodNative(jmethodID method) {
  jboolean isNative;
  jvmtiError err = env->IsMethodNative(method, &isNative);
  checkError(err);

  return isNative;
}

std::pair<jmethodID, uint16_t> Jvmti::getFrameLocation(jthread thread, unsigned int depth) {
  jmethodID methodId;
  jlocation location;

  jvmtiError err = env->GetFrameLocation(thread, depth, &methodId, &location);
  checkError(err);

  return {methodId, static_cast<uint16_t>(location)};
}

jclass Jvmti::getMethodDeclaringClass(jmethodID method) {
  jclass methodClass;
  jvmtiError err = env->GetMethodDeclaringClass(method, &methodClass);
  checkError(err);

  return methodClass;
}

std::vector<uint8_t> Jvmti::getBytecodes(jmethodID method) {
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

ConstPool Jvmti::getConstPool(jclass klass) {
  jint cpCount;
  jint cpByteSize;
  uint8_t *constPoolBytes;

  jvmtiError err = env->GetConstantPool(klass, &cpCount, &cpByteSize, &constPoolBytes);
  checkError(err);
  ConstPool constPool(std::vector<uint8_t>(constPoolBytes, constPoolBytes + cpByteSize));
  err = env->Deallocate(constPoolBytes);
  checkError(err);

  assert(cpCount == constPool.size());

  return constPool;
}

uint32_t Jvmti::getMethodModifiers(jmethodID methodId) {
  jint modifiers;
  jvmtiError err = env->GetMethodModifiers(methodId, &modifiers);
  checkError(err);
  return static_cast<uint32_t>(modifiers);
}

std::pair<std::string, std::string> Jvmti::getMethodNameAndSignature(jmethodID methodId) {
  char *methodName;
  char *methodSignature;

  jvmtiError err = env->GetMethodName(methodId, &methodName, &methodSignature, nullptr);
  checkError(err);

  std::pair<std::string, std::string> nameAndSignature = {std::string{methodName}, std::string{methodSignature}};

  err = env->Deallocate((uint8_t *) methodName);
  checkError(err);
  err = env->Deallocate((uint8_t *) methodSignature);
  checkError(err);

  return nameAndSignature;
}

uint8_t Jvmti::getMethodArgumentsSize(jmethodID methodId) {
  jint size;
  jvmtiError err = env->GetArgumentsSize(methodId, &size);
  checkError(err);
  return static_cast<uint8_t>(size);
}

LocalVariableTable Jvmti::getLocalVariableTable(jmethodID methodId) {
  jint localVariableEntryCount = 0;
  jvmtiLocalVariableEntry *localVariableTable = nullptr;

  jvmtiError err = env->GetLocalVariableTable(methodId, &localVariableEntryCount, &localVariableTable);
  if (err == JVMTI_ERROR_ABSENT_INFORMATION) return {};
  checkError(err);

  LocalVariableTable table;
  for (int i = 0; i < localVariableEntryCount; i++) {
    table.addEntry(static_cast<uint8_t>(localVariableTable[i].slot), localVariableTable[i].name, localVariableTable[i].signature);

    err = env->Deallocate((unsigned char *) localVariableTable[i].name);
    checkError(err);
    err = env->Deallocate((unsigned char *) localVariableTable[i].signature);
    checkError(err);
    err = env->Deallocate((unsigned char *) localVariableTable[i].generic_signature);
    checkError(err);
  }
  err = env->Deallocate((unsigned char *) localVariableTable);
  checkError(err);

  return table;
}

uint32_t Jvmti::getFrameCount(jthread thread) {
  jint count;
  jvmtiError err = env->GetFrameCount(thread, &count);
  checkError(err);
  return static_cast<uint32_t>(count);
}

Method Jvmti::toMethod(jmethodID methodId) {
  jclass declaringClass = getMethodDeclaringClass(methodId);
  auto[name, signature] = getMethodNameAndSignature(methodId);
  uint32_t modifiers = getMethodModifiers(methodId);

  std::string className = Jni::invokeVirtual(declaringClass, "getName", jnisig("()Ljava/lang/String;"));
  return Method(className, name, signature, modifiers);
}
