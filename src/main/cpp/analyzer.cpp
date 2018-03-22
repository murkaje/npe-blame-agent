#include "analyzer.h"

#include "util.h"
#include "Field.h"

std::string traceDetailedCause(const Method &currentFrameMethod,
                               const ConstPool &constPool,
                               const CodeAttribute &code,
                               const LocalVariableTable &vars,
                               size_t location,
                               size_t stackExcess) {
  const std::vector<size_t> &instructions = code.getInstructions();
  size_t ins = 0;
  for (size_t off : instructions) {
    if (off == location) { break; }
    ins++;
  }

  while (stackExcess != -1 && ins != -1) {
    size_t off = instructions[--ins];
    uint8_t opCode = code.getOpcode(off);

    if (opCode >= OpCodes::ILOAD && opCode <= OpCodes::ALOAD_3) {
      //TODO: Extract repetitive if check to getStackDelta(opcode) and if(stackExcess==0 && stackDelta <= 0)?
      if (stackExcess != 0) {
        stackExcess--;
        continue;
      }

      uint8_t slot;
      if (opCode <= OpCodes::ALOAD) {
        slot = ByteVectorUtil::readuint8(code.getCode(), off + 1);
      } else {
        slot = opcodeSlot(opCode);
      }

      size_t methodParamsCount = currentFrameMethod.getParameterCount();
      bool isMethodParam = slot < methodParamsCount + (currentFrameMethod.isStatic() ? 0 : 1);

      auto optVarInfo = vars.getEntry(slot);
      if (optVarInfo.has_value()) {
        auto[name, signature] = *optVarInfo;
        return (isMethodParam ? "method parameter " : "local variable ") + name + ":" + toJavaTypeName(signature);
      } else {
        if (isMethodParam) {
          return formatString("method parameter at index %d", (slot + (currentFrameMethod.isStatic() ? 1 : 0)));
        } else {
          return formatString("local variable in slot %d", slot);
        }
      }
    } else if (opCode >= OpCodes::ISTORE && opCode <= OpCodes::ASTORE_3) {
      stackExcess++;
    } else if (opCode >= OpCodes::LDC && opCode <= OpCodes::LDC2_W ) {
      if (stackExcess != 0) {
        stackExcess--;
      }
    } else if(opCode == OpCodes::ACONST_NULL) {
      if (stackExcess != 0) {
        stackExcess--;
        continue;
      }

      return "constant";
    } else if (opCode == OpCodes::GETFIELD) {
      if (stackExcess != 0) {
        continue;
      }

      Field field = Field::readFromFieldInsn(code, constPool, off);
      return "instance field " + field.getClassName() + "." + field.getFieldName();
    } else if (opCode == OpCodes::GETSTATIC) {
      if (stackExcess != 0) {
        stackExcess--;
        continue;
      }

      Field field = Field::readFromFieldInsn(code, constPool, off);
      return "static field " + field.getClassName() + "." + field.getFieldName();
    }
      //TODO: Manually generated bytecode for indy? does it throw npe? javac prepends implicit null check with getClass/Objects.requireNonNull
      //Parse BootStrapmethod and get MethodType passed to LambdaMetaFactory to determine which method ref was taken
      //Diff between method ref and lambda?
    else if (opCode >= OpCodes::INVOKEVIRTUAL && opCode <= OpCodes::INVOKEINTERFACE) {
      //Calc stack diff from method invoke
      auto invokedMethod = Method::readFromCodeInvoke(code, constPool, off);
      int invokeStackDelta = -invokedMethod.getParameterCount();
      if (invokedMethod.getReturnType() != "void") { invokeStackDelta++; }
      if (opCode == OpCodes::INVOKEVIRTUAL || opCode == OpCodes::INVOKEINTERFACE) { invokeStackDelta--; }

      // The stack depth is fitting and the method invocation before it added to top of stack
      if (stackExcess == 0 && invokedMethod.getReturnType() != "void") {
        return "object returned from " + invokedMethod.getClassName() + "#" + invokedMethod.getMethodName();
      }

      stackExcess += invokeStackDelta;
    }
  }

  return "UNKNOWN";
}

std::string describeNPEInstruction(const Method &currentFrameMethod, const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location) {
  std::string errorSource;
  size_t stackExcess;

  uint8_t op = code.getOpcode(location);
  if (op >= OpCodes::INVOKEVIRTUAL && op <= OpCodes::INVOKEDYNAMIC) {
    Method method = Method::readFromCodeInvoke(code, cp, location);

    if (method.getClassName() == "java.util.Objects" && method.getMethodName() == "requireNonNull") {
      errorSource = "Assertion Objects#requireNonNull failed for null ";
      stackExcess = 0;
    } else {
      errorSource = "Invoking " + method.getClassName() + "#" + method.getMethodName() + " on null ";
      stackExcess = method.getParameterCount();
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
    stackExcess = 1;
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