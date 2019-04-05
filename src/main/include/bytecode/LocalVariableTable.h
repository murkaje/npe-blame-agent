#pragma once

#include <optional>
#include <string>
#include <map>
#include <cstdint>
#include <jvmti.h>

class LocalVariableTable {

  std::map<uint8_t, const std::tuple<const std::string, const std::string>> table;

public:

  void addEntry(uint8_t slot, std::string_view name, std::string_view signature);

  //Bytecode manipulation tools may add local variables without altering LocalVariableTable
  std::optional<const std::tuple<std::string_view, std::string_view>> getEntry(uint16_t slot) const;
};