#pragma once

#include <optional>
#include <string>
#include <map>
#include <cstdint>
#include <jvmti.h>

class LocalVariableTable {

  std::map<uint16_t, const std::tuple<const std::string, const std::string>> table;

public:

  LocalVariableTable() = default;

  LocalVariableTable(jvmtiEnv *jvmti, jmethodID method);

  std::optional<const std::tuple<const std::string, const std::string>> getEntry(uint16_t slot) const;
};