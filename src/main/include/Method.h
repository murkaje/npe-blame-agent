#pragma once

#include <string>
#include <vector>

#include "CodeAttribute.h"
#include "ConstPool.h"

class Method {
  std::string className;
  std::string returnType;
  std::string methodName;
  std::vector<std::string> parameterTypes;

public:
  Method(std::string className, std::string methodName, const std::string &signature);

  static Method readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci);

  static Method readFromMemberRef(const ConstPool &constPool, size_t refId);

  const std::string &getClassName() const {
    return className;
  }

  const std::string &getReturnType() const {
    return returnType;
  }

  const std::string &getMethodName() const {
    return methodName;
  }

  const std::vector<std::string> &getParameterTypes() const {
    return parameterTypes;
  }

  const size_t getParameterCount() const {
    return parameterTypes.size();
  }
};