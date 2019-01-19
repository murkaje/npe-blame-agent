#include <cstring>
#include <iostream>
#include <fstream>
#include <jvmti.h>
#include <spdlog.h>

#include "util.h"
#include "api/Jvmti.h"

static auto logger = getLogger("Boot");

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
  Agent_OnLoad(vm, options, reserved);

  return JNI_OK;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
  std::string opts = options == nullptr ? "" : std::string{options};
  if (opts == "debug") {
    spdlog::set_level(spdlog::level::debug);
  } else if (opts == "trace") {
    spdlog::set_level(spdlog::level::trace);
  }

  spdlog::set_pattern("%Y-%m-%d %T.%e %L [%n] %v");

  Jvmti::init(vm);

  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
}

