#include <cstring>
#include <iostream>
#include <fstream>
#include <jvmti.h>
#include "jni.h"

#include "jvmti.h"

#include "CodeAttribute.h"
#include "util.h"
#include "exceptionCallback.h"
#include "logging.h"

static Logger log("OnLoad");

/**
 * Run all tests at start of onLoad, refactor to actual cmake tests later(catch? gtest?)
 */
void runTests() {
  // Example class

  // Getting elements from const pool

  // Printing method bytecode

  // Printing method local variables

  // Printing exception table

  // -----------

  // Bytecode instrumentation tests
}

void JNICALL callback_VMInit(jvmtiEnv *jvmti, JNIEnv *env, jthread thread) {
}

void JNICALL
ClassFileLoadHook(jvmtiEnv *jvmti_env,
                  JNIEnv *jni_env,
                  jclass class_being_redefined,
                  jobject loader,
                  const char *name,
                  jobject protection_domain,
                  jint class_data_len,
                  const unsigned char *class_data,
                  jint *new_class_data_len,
                  unsigned char **new_class_data) {
//  std::cerr << "Loading class '" << name << "'\n";
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {

  runTests();

  jint rc;
  jvmtiError err;
  jvmtiEnv *jvmti = nullptr;

  rc = vm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_1);
  if (rc != JNI_OK) {
    log.error("Failed to acquire JVMTI 1.1 environment");
  }

  jvmtiEventCallbacks callbacks = {nullptr};
  jvmtiCapabilities caps = {0};

  caps.can_get_bytecodes = 1;
  caps.can_generate_exception_events = 1;
  caps.can_get_line_numbers = 1;
  caps.can_get_constant_pool = 1;
  caps.can_access_local_variables = 1;

  caps.can_generate_all_class_hook_events = 1;
  caps.can_retransform_classes = 1;
  caps.can_retransform_any_class = 1;

  err = jvmti->AddCapabilities(&caps);
  check_jvmti_error(jvmti, err, "Set required JVMTI Capabilities");

  jvmtiJlocationFormat locationFormat;
  err = jvmti->GetJLocationFormat(&locationFormat);
  check_jvmti_error(jvmti, err, "Get jlocation format");

  if (locationFormat == JVMTI_JLOCATION_JVMBCI) {
    log.debug("jlocation format is JVMBCI");
  }
  else if (locationFormat == JVMTI_JLOCATION_MACHINEPC) {
    log.debug("jlocation format is MACHINEPC");
  }
  else {
    log.debug("jlocation format is OTHER");
  }

  callbacks.VMInit = &callback_VMInit;
  callbacks.Exception = &callback_Exception;
  callbacks.ClassFileLoadHook = &ClassFileLoadHook;
  err = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
  check_jvmti_error(jvmti, err, "Set Event callbacks");

  err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
  check_jvmti_error(jvmti, err, "Enable VMInit events");

  err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, nullptr);
  check_jvmti_error(jvmti, err, "Enable Exception events");

  err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
  check_jvmti_error(jvmti, err, "Enable class load events");

  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
}

