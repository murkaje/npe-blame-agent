#include "CodeAttribute.h"

#include "util.h"
#include "Constants.h"
#include "logging.h"

static Logger log("CodeAttribute");

std::string CodeAttribute::toString(const ConstPool &constPool) const {
  return "";
}

std::string CodeAttribute::printInstruction(const ConstPool &constPool, size_t offset) const {
  uint8_t opcode = code[offset];

  switch (opcode) {
    case BIPUSH:
      return formatString("%-15s %d", Constants::OpcodeMnemonic[opcode], code[offset + 1]);
    case SIPUSH:
      return formatString("%-15s %d", Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1));
    case LDC:
      return formatString("%-15s ", Constants::OpcodeMnemonic[opcode]) + constPool.entryToString(code[offset + 1], false);
    case LDC_W:
    case LDC2_W: {
      const uint16_t constantIndex = ByteVectorUtil::readuint16(code, offset + 1);
      return formatString("%-15s ", Constants::OpcodeMnemonic[opcode]) + constPool.entryToString(constantIndex, false);
    }
    case ILOAD:
    case LLOAD:
    case FLOAD:
    case DLOAD:
    case ALOAD:
    case ISTORE:
    case LSTORE:
    case FSTORE:
    case DSTORE:
    case ASTORE:
      return formatString("%-15s ", Constants::OpcodeMnemonic[opcode]) + printLocalVariable(code[offset + 1]);
    case ILOAD_0:
    case ILOAD_1:
    case ILOAD_2:
    case ILOAD_3:
    case LLOAD_0:
    case LLOAD_1:
    case LLOAD_2:
    case LLOAD_3:
    case FLOAD_0:
    case FLOAD_1:
    case FLOAD_2:
    case FLOAD_3:
    case DLOAD_0:
    case DLOAD_1:
    case DLOAD_2:
    case DLOAD_3:
    case ALOAD_0:
    case ALOAD_1:
    case ALOAD_2:
    case ALOAD_3:
    case ISTORE_0:
    case ISTORE_1:
    case ISTORE_2:
    case ISTORE_3:
    case LSTORE_0:
    case LSTORE_1:
    case LSTORE_2:
    case LSTORE_3:
    case FSTORE_0:
    case FSTORE_1:
    case FSTORE_2:
    case FSTORE_3:
    case DSTORE_0:
    case DSTORE_1:
    case DSTORE_2:
    case DSTORE_3:
    case ASTORE_0:
    case ASTORE_1:
    case ASTORE_2:
    case ASTORE_3: {
      uint8_t slot = opcodeSlot(opcode);
      return formatString("%-15s ", Constants::OpcodeMnemonic[opcode]) + printLocalVariable(slot);
    }
    case IFEQ:
    case IFGE:
    case IFGT:
    case IFLE:
    case IFLT:
    case IFNE:
    case IFNONNULL:
    case IFNULL:
    case IF_ACMPEQ:
    case IF_ACMPNE:
    case IF_ICMPEQ:
    case IF_ICMPGE:
    case IF_ICMPGT:
    case IF_ICMPLE:
    case IF_ICMPLT:
    case IF_ICMPNE:
      return formatString("%-15s %d", Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1) + offset);
    case IINC:
      return formatString("%-15s %d, %d", Constants::OpcodeMnemonic[opcode], code[offset + 1], ByteVectorUtil::readint8(code, offset + 2));
    case GOTO:
    case JSR:
      return formatString("%-15s %d", Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1) + offset);
    case RET:
      return formatString("%-15s %d", Constants::OpcodeMnemonic[opcode], code[offset + 1]);
    case TABLESWITCH:
      return "";  //TODO
    case LOOKUPSWITCH:
      return "";  //TODO
    case GETSTATIC:       //Fieldref
    case PUTSTATIC:
    case GETFIELD:
    case PUTFIELD:
    case INVOKEVIRTUAL:   //Methodref
    case INVOKESPECIAL:
    case INVOKESTATIC:
    case INVOKEINTERFACE: //InterfaceMethodref
    case INVOKEDYNAMIC:   //MethodHandle
    case NEW:             //Class
    case CHECKCAST:
    case ANEWARRAY: {
      const uint16_t refIndex = ByteVectorUtil::readuint16(code, offset + 1);
      return formatString("%-15s ", Constants::OpcodeMnemonic[opcode]) + constPool.entryToString(refIndex, false);
    }
    case MULTIANEWARRAY: {
      const uint16_t refIndex = ByteVectorUtil::readuint16(code, offset + 1);
      return formatString("%-15s ", Constants::OpcodeMnemonic[opcode]) + constPool.entryToString(refIndex, false) + formatString(" %d", code[offset + 3]);
    }
    case NEWARRAY: {
      uint8_t type = code[offset + 1];
      return formatString("%-15s %s", Constants::OpcodeMnemonic[opcode], Constants::ArrayType[type]);
    }
    case GOTO_W:
    case JSR_W:
      return formatString("%-15s %d", Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1) + offset);
    case WIDE:
      //TODO
    default:
      return formatString("%-15s", Constants::OpcodeMnemonic[opcode]);
  }
}

std::string CodeAttribute::printLocalVariable(uint8_t slot) const {
  auto [name, desc] = localVariables.get(slot);
  return formatString("%d: name=%s type=%s", slot, name.c_str(), desc.c_str());
}

const uint8_t loadStoreSlotLookupTable[] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, //0-9
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, //10-19
    9, 9, 9, 9, 9, 9, 0, 1, 2, 3, //20-29
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, //30-39
    2, 3, 0, 1, 2, 3, 9, 9, 9, 9, //40-49
    9, 9, 9, 9, 9, 9, 9, 9, 9, 0, //50-59
    1, 2, 3, 0, 1, 2, 3, 0, 1, 2, //60-69
    3, 0, 1, 2, 3, 0, 1, 2, 3, 9, //70-79
};

/**
 * Get local variable slot for 1 byte load/store opcodes e.g. ALOAD_1 -> 1
 */
uint8_t CodeAttribute::opcodeSlot(uint8_t opCode) const {
  if (opCode >= ILOAD_0 && opCode <= ASTORE_3) {
    uint8_t val = loadStoreSlotLookupTable[opCode];
    if (val != 9) { return val; }
  }

  throw std::invalid_argument(formatString("Opcode is not a valid 1 byte load/store: %s", Constants::OpcodeMnemonic[opCode]));
}

void InstructionPrintIterator::operator++(int) {
  if (offset > code.getSize()) {
    throw std::out_of_range(formatString("Current offset: %d, bytecode length: %d", offset, code.getSize()));
  }

//  log.info(formatString("Offset before: %d", offset));
  offset += Constants::InstructionLength[code.getOpcode(offset)];
//  log.info(formatString("Offset after: %d", offset));
}

std::string InstructionPrintIterator::operator*() const {
  return code.printInstruction(constPool, offset);
}


std::tuple<uint16_t, uint8_t> InstructionIterator::operator*() const {
  return std::make_tuple(offset, code.getOpcode(offset));
}

void InstructionIterator::operator++(int) {
  if (offset > code.getSize()) {
    throw std::out_of_range(formatString("Current offset: %d, bytecode length: %d", offset, code.getSize()));
  }

  offset += Constants::InstructionLength[code.getOpcode(offset)];
}
