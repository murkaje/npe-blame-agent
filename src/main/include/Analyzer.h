#pragma once

#include <stack>
#include "CodeAttribute.h"
#include "util.h"

/*
 * aload_1
 * invi a(Object)Object
 * invi b(Object)Object
 * aconst_null
 * stack: slot1 -> a(Object)Object -> b(Object)Object, null
 *
 * Show only immediate source?
 * null,astore_x,aload_x,invoke ? (loaded from variable slot x, returned from aconst_null)?
 */

std::string traceInvokeInstance(const ConstPool &constPool, const CodeAttribute &code, size_t invokeBci) {
  const std::vector<size_t> &instructions = code.getInstructions();
  size_t ins = 0;
  for(size_t off : instructions) {
    if(off == invokeBci) break;
    ins++;
  }

  // Walk back to where 'this' was obtained
  size_t pos = instructions[ins];
  auto method = Method::readFromCodeInvoke(code, constPool, pos);

  // go back this many elements in stack
  size_t stackExcess = method.getParameterCount();

  ins--;

  size_t off = instructions[ins];
  uint8_t opCode = code.getOpcode(off);
  if(opCode >= OpCodes::ILOAD && opCode <= OpCodes::ALOAD_3) {
    if(stackExcess == 0) {
      size_t slot;
      if(opCode <= ALOAD) {
        slot = ByteVectorUtil::readuint16(code.getCode(), off + 1);
      } else {
        slot = opcodeSlot(opCode);
      }

      return formatString("local variable in slot %d", slot);
    }
    else {
      stackExcess--;
    }
  }
  else if(opCode >= OpCodes::ISTORE && opCode <= OpCodes::ASTORE_3) {
    stackExcess++;
  }

  return "[unimplemented]";
}