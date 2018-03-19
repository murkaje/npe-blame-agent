#include "analyzer.h"

#include "util.h"
#include "Method.h"
#include "Field.h"

std::string traceDetailedCause(const ConstPool &constPool, const CodeAttribute &code, const LocalVariableTable &vars, size_t location, size_t stackExcess) {
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
      if (stackExcess == 0) {
        size_t slot;
        if (opCode <= OpCodes::ALOAD) {
          slot = ByteVectorUtil::readuint16(code.getCode(), off + 1);
        }
        else {
          slot = opcodeSlot(opCode);
        }

        if (auto varInfo = vars.getEntry(slot)) {
          auto[name, signature] = *varInfo;
          return "local variable " + name + ":" + toJavaTypeName(signature);
        }
        else {
          return formatString("local variable in slot %d", slot);
        }
      }
      else {
        stackExcess--;
      }
    }
    else if (opCode >= OpCodes::ISTORE && opCode <= OpCodes::ASTORE_3) {
      stackExcess++;
    }
    //Indy? Invokespecial?
    else if (opCode >= OpCodes::INVOKEVIRTUAL && opCode <= OpCodes::INVOKEINTERFACE) {
      //Calc stack diff from method invoke
      auto invokedMethod = Method::readFromCodeInvoke(code, constPool, off);
      int invokeStackDelta = invokedMethod.getParameterCount();
      if (invokedMethod.getReturnType() != "V") { invokeStackDelta--; }
      if (opCode != OpCodes::INVOKESTATIC) { invokeStackDelta++; }

      // The stack depth is fitting and the method invocation before it added to top of stack
      if (stackExcess == 0 && invokedMethod.getReturnType() != "V") {
        return "object returned from " + invokedMethod.getClassName() + "#" + invokedMethod.getMethodName();
      }

      stackExcess += invokeStackDelta;
    }
  }

  return "UNKNOWN";
}

std::string describeNPEInstruction(const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location) {
  std::string errorSource;
  size_t stackExcess;

  uint8_t op = code.getOpcode(location);
  if (op >= OpCodes::INVOKEVIRTUAL && op <= OpCodes::INVOKEDYNAMIC) {
    Method method = Method::readFromCodeInvoke(code, cp, location);

    errorSource = "Invoking " + method.getClassName() + "#" + method.getMethodName() + " on null ";
    stackExcess = method.getParameterCount();
  }
  else if (op >= OpCodes::GETFIELD && op <= OpCodes::PUTFIELD) {
    Field field = Field::readFromFieldInsn(code, cp, location);
    std::string putOrGet;

    if(op == OpCodes::GETFIELD) {
      putOrGet = "Getting";
      stackExcess = 0;
    }
    else {
      putOrGet = "Setting";
      stackExcess = 1;
    }

    errorSource = putOrGet + " field " + field.getClassName() + "." + field.getFieldName() + " of null ";
  }
  else if (op >= OpCodes::IASTORE && op <= OpCodes::SASTORE) {
    errorSource = "Storing array value to null ";
    stackExcess = 1;
  }
  else if (op >= OpCodes::IALOAD && op <= OpCodes::SALOAD) {
    errorSource = "Loading array value from null ";
    stackExcess = 0;
  }
  else if (op == OpCodes::ARRAYLENGTH) {
    errorSource = "Getting array length of null ";
    stackExcess = 0;
  }
  else if (op == OpCodes::ATHROW) {
    errorSource = "[athrow] ";
    stackExcess = 0;
  }
  else {
    return "[Unknown NPE cause] ";
  }

  return errorSource + traceDetailedCause(cp, code, vars, location, stackExcess);
}