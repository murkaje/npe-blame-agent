#pragma once

#include <string>
#include <vector>

#include "CodeAttribute.h"
#include "ConstPool.h"

class Method {
  uint32_t modifiers;
  std::string className;
  std::string returnType;
  std::string methodName;
  std::string signature;
  std::vector<std::string> parameterTypes;
  uint8_t parameterLength;

public:
  Method(std::string_view className, std::string_view methodName, std::string_view signature, uint32_t modifiers);

  static Method readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci);

  static Method readFromMemberRef(const ConstPool &constPool, size_t refId, uint32_t modifiers);

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

  //region Method modifiers

  bool isPublic() const {
    return (modifiers & Modifier::PUBLIC) != 0;
  }

  bool isPrivate() const {
    return (modifiers & Modifier::PRIVATE) != 0;
  }

  bool isProtected() const {
    return (modifiers & Modifier::PROTECTED) != 0;
  }

  bool isStatic() const {
    return (modifiers & Modifier::STATIC) != 0;
  }

  bool isFinal() const {
    return (modifiers & Modifier::FINAL) != 0;
  }

  bool isSynchronized() const {
    return (modifiers & Modifier::SYNCHRONIZED) != 0;
  }

  bool isBridge() const {
    return (modifiers & Modifier::BRIDGE) != 0;
  }

  bool isVarargs() const {
    return (modifiers & Modifier::VARARGS) != 0;
  }

  bool isNative() const {
    return (modifiers & Modifier::NATIVE) != 0;
  }

  bool isAbstract() const {
    return (modifiers & Modifier::ABSTRACT) != 0;
  }

  bool isStrictfp() const {
    return (modifiers & Modifier::STRICT) != 0;
  }

  bool isSynthetic() const {
    return (modifiers & Modifier::SYNTHETIC) != 0;
  }
  //endregion

};

struct Frame {
  Method method;
  jlocation location;
};