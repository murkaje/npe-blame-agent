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
  std::string signature;
  std::vector<std::string> parameterTypes;
  uint8_t parameterLength;

public:
  Method(std::string_view className, std::string_view methodName, std::string_view signature, bool isStatic);

  static Method readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci);

  static Method readFromMemberRef(const ConstPool &constPool, size_t refId, bool isStatic);

  std::string_view getClassName() const {
    return className;
  }

  std::string_view getReturnType() const {
    return returnType;
  }

  std::string_view getMethodName() const {
    return methodName;
  }

  std::string_view getMethodSignature() const {
    return signature;
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

struct Frame {
  Method method;
  jlocation location;
};