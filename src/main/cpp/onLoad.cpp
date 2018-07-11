#include <cstring>
#include <iostream>
#include <fstream>
#include <jvmti.h>
#include <spdlog.h>
#include <backward.hpp>

#include "util.h"
#include "Jvmti.h"

static auto logger = getLogger("Boot");

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
  Agent_OnLoad(vm, options, reserved);

  return JNI_OK;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
  Jvmti::init(vm);

  std::string opts = options == nullptr ? "" : std::string{options};
  if (opts == "debug") {
    spdlog::set_level(spdlog::level::debug);
  } else if (opts == "trace") {
    spdlog::set_level(spdlog::level::trace);
  }

  spdlog::set_pattern("%Y-%m-%d %T.%e %L [%n] %v");
  logger->info("npe-blame-agent init");

  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
}

