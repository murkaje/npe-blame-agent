#include <cstring>
#include <iostream>
#include <fstream>
#include <jvmti.h>
#include <spdlog.h>

#include "CodeAttribute.h"
#include "util.h"
#include "exceptionCallback.h"

static auto logger = getLogger("Boot");

// Extract jni/jvmti calls to a simple library that exposes proper c++ interface and masks the c-isms

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
  //Same as onload?
  return JNI_OK;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
  std::string opts = options == nullptr ? "" : std::string{options};
  if (opts == "debug") {
    spdlog::set_level(spdlog::level::debug);
  }

  spdlog::set_pattern("%Y-%m-%d %T.%e %L [%n] %v");
  logger->info("Agent_OnLoad");

  jint rc;
  jvmtiError err;
  jvmtiEnv *jvmti = nullptr;

  rc = vm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_1);
  if (rc != JNI_OK) {
    logger->error("Failed to acquire JVMTI 1.1 environment");
    return JNI_OK;
  }

  jvmtiEventCallbacks callbacks = {nullptr};
  jvmtiCapabilities caps = {0};

  caps.can_get_bytecodes = 1;
  caps.can_generate_exception_events = 1;
  caps.can_get_line_numbers = 1;
  caps.can_get_constant_pool = 1;
  caps.can_access_local_variables = 1;

  err = jvmti->AddCapabilities(&caps);
  check_jvmti_error(jvmti, err, "Set required JVMTI Capabilities");

  jvmtiJlocationFormat locationFormat;
  err = jvmti->GetJLocationFormat(&locationFormat);
  check_jvmti_error(jvmti, err, "Get jlocation format");

  if (locationFormat == JVMTI_JLOCATION_JVMBCI) {
    logger->debug("jlocation format is JVMBCI");
  } else if (locationFormat == JVMTI_JLOCATION_MACHINEPC) {
    logger->debug("jlocation format is MACHINEPC");
  } else {
    logger->debug("jlocation format is OTHER");
  }

  callbacks.Exception = &callback_Exception;
  err = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
  check_jvmti_error(jvmti, err, "Set Event callbacks");

  err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, nullptr);
  check_jvmti_error(jvmti, err, "Enable Exception events");

  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
}

