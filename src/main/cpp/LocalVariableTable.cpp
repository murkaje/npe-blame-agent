#include "LocalVariableTable.h"

#include "util.h"

std::optional<const std::tuple<std::string_view, std::string_view>> LocalVariableTable::getEntry(uint8_t slot) const {
  if (auto entry = table.find(slot); entry != table.end()) {
    return entry->second;
  }
  return {};
}

void LocalVariableTable::addEntry(uint8_t slot, std::string_view name, std::string_view signature) {
  table.emplace(slot, std::make_pair(name, signature));
}
