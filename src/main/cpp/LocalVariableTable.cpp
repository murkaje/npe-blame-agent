#include "LocalVariableTable.h"

#include <string>

#include "util.h"

LocalVariableTable::LocalVariableTable(jvmtiEnv *jvmti, jmethodID method) {
  jint localVariableEntryCount = 0;
  jvmtiLocalVariableEntry *localVariableTable = nullptr;
  jvmtiError err = jvmti->GetLocalVariableTable(method, &localVariableEntryCount, &localVariableTable);

  if (err != JVMTI_ERROR_ABSENT_INFORMATION) {
    check_jvmti_error(jvmti, err, "Get LocalVariableTable");

    for (int i = 0; i < localVariableEntryCount; i++) {
      int pos = localVariableTable[i].slot;
      table.emplace(pos, std::make_tuple(std::string(localVariableTable[i].name), std::string(localVariableTable[i].signature)));

      err = jvmti->Deallocate((unsigned char *) localVariableTable[i].name);
      check_jvmti_error(jvmti, err, "Deallocate localVariableTable[i].name");

      err = jvmti->Deallocate((unsigned char *) localVariableTable[i].signature);
      check_jvmti_error(jvmti, err, "Deallocate localVariableTable[i].signature");

      err = jvmti->Deallocate((unsigned char *) localVariableTable[i].generic_signature);
      check_jvmti_error(jvmti, err, "Deallocate localVariableTable[i].generic_signature");
    }
    err = jvmti->Deallocate((unsigned char *) localVariableTable);
    check_jvmti_error(jvmti, err, "Deallocate localVariableTable");
  }
}

std::optional<const std::tuple<const std::string, const std::string>> LocalVariableTable::getEntry(uint16_t slot) const {
  if (auto entry = table.find(slot); entry != table.end()) {
    return entry->second;
  }
  return std::nullopt;
}
