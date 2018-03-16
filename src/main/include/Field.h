#pragma once

#include <string>

#include "CodeAttribute.h"
#include "ConstPool.h"

class Field {
  std::string className;
  std::string fieldName;
  std::string typeName;

public:
  Field(std::string className, std::string fieldName, std::string typeName);

  static Field readFromFieldInsn(const CodeAttribute &code, const ConstPool &constPool, size_t bci);

  static Field readFromMemberRef(const ConstPool &constPool, size_t refId);

  const std::string &getClassName() const {
    return className;
  }

  const std::string &getFieldName() const {
    return fieldName;
  }

  const std::string &getTypeName() const {
    return typeName;
  }
};