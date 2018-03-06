#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <jvmti.h>

class LocalVariableTable {

  std::map<uint16_t, std::tuple<std::string, std::string>> table;

public:

  static std::tuple<std::string, std::string> missing;

  LocalVariableTable() = default;

  LocalVariableTable(jvmtiEnv *jvmti, jmethodID method);

  const std::tuple<std::string, std::string>& get(uint16_t slot) const;
};