#include "analyzer.h"

#include <string_view>

#include "util.h"
#include "Field.h"

using fmt::literals::operator""_format;

int getStackDelta(const CodeAttribute &code, const ConstPool &constPool, size_t off, int stackExcess) {
  uint8_t opCode = code.getOpcode(off);
  int delta = Constants::OpCodeStackDelta[opCode];

  if (delta != -127 && delta != 127) { return delta; }

  if (opCode >= OpCodes::INVOKEVIRTUAL && opCode <= OpCodes::INVOKEINTERFACE) {
    auto invokedMethod = Method::readFromCodeInvoke(code, constPool, off);
    int invokeStackDelta = -invokedMethod.getParameterLength();
    if (invokedMethod.getReturnType() != "void") { invokeStackDelta++; }
    if (opCode == OpCodes::INVOKEVIRTUAL || opCode == OpCodes::INVOKEINTERFACE || opCode == OpCodes::INVOKESPECIAL) { invokeStackDelta--; }
    return invokeStackDelta;
  }

  if (opCode == OpCodes::MULTIANEWARRAY) {
    uint8_t dimensions = ByteVectorUtil::readuint8(code.getCode(), off + 3);
    return 1 - dimensions;
  }

  if (opCode >= OpCodes::GETSTATIC && opCode <= OpCodes::PUTFIELD) {
    std::string fieldType = Field::readFromFieldInsn(code, constPool, off).getTypeName();
    int fieldTypeSize = (fieldType == "long" || fieldType == "double") ? 2 : 1;
    switch (opCode) {
      case OpCodes::GETSTATIC:
        return fieldTypeSize;
      case OpCodes::PUTSTATIC:
        return -fieldTypeSize;
      case OpCodes::GETFIELD:
        return -1 + fieldTypeSize;
      case OpCodes::PUTFIELD:
        return -1 - fieldTypeSize;
    }
  }

  if (opCode == OpCodes::SWAP) {
    if (stackExcess == 1) {
      return 1;
    } else if (stackExcess == 0) {
      return -1;
    } else {
      return 0;
    }
  }

  // For distant elements in stack, top moved 1 unit up
  // The element copied by dup can be found 2 elements closer
  if (opCode == OpCodes::DUP_X1) {
    if (stackExcess == 2) {
      return 2;
    } else if (stackExcess < 2) {
      return 0;
    } else {
      return 1;
    }
  }

  // For distant elements in stack, top moved 1 unit up
  // The element copied by dup can be found 3 elements closer
  if (opCode == OpCodes::DUP_X2) {
    if (stackExcess == 3) {
      return 3;
    } else if (stackExcess < 3) {
      return 0;
    } else {
      return 1;
    }
  }

  // For distant elements in stack, top moved 2 units up
  // The elements copied by dup can be found 2 elements closer
  if (opCode == OpCodes::DUP2) {
    return (stackExcess >= 2) ? 2 : 0;
  }

  // For distant elements in stack, top moved 2 units up
  // The elements copied by dup can be found 3 elements closer
  if (opCode == OpCodes::DUP2_X1) {
    if (stackExcess == 3 || stackExcess == 4) {
      return 3;
    } else if (stackExcess < 3) {
      return 0;
    } else {
      return 2;
    }
  }

  // For distant elements in stack, top moved 2 units up
  // The elements copied by dup can be found 4 elements closer
  if (opCode == OpCodes::DUP2_X2) {
    if (stackExcess == 4 || stackExcess == 5) {
      return 4;
    } else if (stackExcess < 4) {
      return 0;
    } else {
      return 2;
    }
  }

  throw std::invalid_argument("Unsupported opcode for calculating stack delta: {}"_format(Constants::OpcodeMnemonic[opCode]));
}

std::string traceDetailedCause(const Method &currentFrameMethod,
                               const ConstPool &constPool,
                               const CodeAttribute &code,
                               const LocalVariableTable &vars,
                               size_t location,
                               int stackExcess) {
  const std::vector<size_t> &instructions = code.getInstructions();
  size_t ins = 0;
  for (size_t off : instructions) {
    if (off == location) { break; }
    ins++;
  }

  while (stackExcess >= 0 && ins != 0) {
    size_t off = instructions[--ins];

    int stackDelta = getStackDelta(code, constPool, off, stackExcess);
    getLogger("Analyzer")->trace("Op: {}, delta: {}, excess: {}", Constants::OpcodeMnemonic[code.getOpcode(off)], stackDelta, stackExcess);
    stackExcess -= stackDelta;
    if (stackExcess > 0 || stackExcess == 0 && stackDelta != 0) {
      continue;
    }

    uint8_t opCode = code.getOpcode(off);
    if (opCode >= OpCodes::ILOAD && opCode <= OpCodes::ALOAD_3) {
      uint8_t slot;
      if (opCode <= OpCodes::ALOAD) {
        slot = ByteVectorUtil::readuint8(code.getCode(), off + 1);
      } else {
        slot = opcodeSlot(opCode);
      }

      size_t methodParamsLength = currentFrameMethod.getParameterLength();
      bool isMethodParam = slot < methodParamsLength + (currentFrameMethod.isStatic() ? 0 : 1);

      auto optVarInfo = vars.getEntry(slot);
      if (optVarInfo.has_value()) {
        auto[name, signature] = *optVarInfo;
        return (isMethodParam ? "method parameter " : "local variable ") + name + ":" + toJavaTypeName(signature);
      } else {
        if (isMethodParam) {
          int index = currentFrameMethod.isStatic() ? 1 : 0;
          int paramSlot = 0;
          for(std::string_view param : currentFrameMethod.getParameterTypes()) {
            if(param == "long" || param == "double") {
              paramSlot += 2;
            } else {
              paramSlot++;
            }
            index++;
            if(paramSlot == slot) break;
          }
          return "method parameter at index {}"_format(index);
        } else {
          return "local variable in slot {}"_format(slot);
        }
      }
    } else if (opCode == OpCodes::ACONST_NULL) {
      return "constant";
    } else if (opCode == OpCodes::GETFIELD) {
      Field field = Field::readFromFieldInsn(code, constPool, off);
      return "instance field " + field.getClassName() + "." + field.getFieldName();
    } else if (opCode == OpCodes::GETSTATIC) {
      Field field = Field::readFromFieldInsn(code, constPool, off);
      return "static field " + field.getClassName() + "." + field.getFieldName();
    }
      //TODO: Manually generated bytecode for indy? does it throw npe? javac prepends implicit null check with getClass/Objects.requireNonNull
      //Parse BootStrapmethod and get MethodType passed to LambdaMetaFactory to determine which method ref was taken
      //Diff between method ref and lambda?
    else if (opCode >= OpCodes::INVOKEVIRTUAL && opCode <= OpCodes::INVOKEINTERFACE) {
      auto invokedMethod = Method::readFromCodeInvoke(code, constPool, off);

      if (invokedMethod.getReturnType() != "void") {
        return "object returned from " + invokedMethod.getClassName() + "#" + invokedMethod.getMethodName();
      }
    }
  }

  return "UNKNOWN";
}

std::string describeNPEInstruction(const Method &currentFrameMethod, const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location) {
  std::string errorSource;
  int stackExcess;

  uint8_t op = code.getOpcode(location);
  if (op >= OpCodes::INVOKEVIRTUAL && op <= OpCodes::INVOKEDYNAMIC) {
    Method method = Method::readFromCodeInvoke(code, cp, location);

    if (method.getClassName() == "java.util.Objects" && method.getMethodName() == "requireNonNull") {
      errorSource = "Assertion Objects#requireNonNull failed for null ";
      stackExcess = 0;
    } else {
      errorSource = "Invoking " + method.getClassName() + "#" + method.getMethodName() + " on null ";
      stackExcess = method.getParameterLength();
    }
  } else if (op >= OpCodes::GETFIELD && op <= OpCodes::PUTFIELD) {
    Field field = Field::readFromFieldInsn(code, cp, location);
    std::string putOrGet;

    if (op == OpCodes::GETFIELD) {
      putOrGet = "Getting";
      stackExcess = 0;
    } else {
      putOrGet = "Setting";
      stackExcess = 1;
    }

    errorSource = putOrGet + " field " + field.getClassName() + "." + field.getFieldName() + " of null ";
  } else if (op >= OpCodes::IASTORE && op <= OpCodes::SASTORE) {
    errorSource = "Storing array value to null ";
    stackExcess = 2;
  } else if (op >= OpCodes::IALOAD && op <= OpCodes::SALOAD) {
    errorSource = "Loading array value from null ";
    stackExcess = 0;
  } else if (op == OpCodes::ARRAYLENGTH) {
    errorSource = "Getting array length of null ";
    stackExcess = 0;
  } else if (op == OpCodes::ATHROW) {
    errorSource = "Throwing null ";
    stackExcess = 0;
  } else {
    return "[Unknown NPE cause] ";
  }

  return errorSource + traceDetailedCause(currentFrameMethod, cp, code, vars, location, stackExcess);
}