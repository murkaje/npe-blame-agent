#pragma once

#include <string>
#include <vector>

#include "CodeAttribute.h"
#include "ConstPool.h"

class Method {
  //TODO: other method modifiers?
  bool methodStatic;
  std::string className;
  std::string returnType;
  std::string methodName;
  std::vector<std::string> parameterTypes;
  uint8_t parameterLength;

public:
  Method(std::string className, std::string methodName, const std::string &signature, bool isStatic);

  static Method readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci);

  static Method readFromMemberRef(const ConstPool &constPool, size_t refId, bool isStatic);

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

  uint8_t getParameterCount() const {
    // JVM §4.3.3: Max length 255 parameters
    return static_cast<uint8_t>(parameterTypes.size());
  }

  /**
   * Return the length as a sum of contributions from each method parameter
   * A parameter of type long or double contributes two units to the length and a parameter of any other type contributes one unit
   */
  uint8_t getParameterLength() const {
    return parameterLength;
  }

  bool isStatic() const {
    return methodStatic;
  }
};