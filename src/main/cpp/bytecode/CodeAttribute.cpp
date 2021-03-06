#include "bytecode/CodeAttribute.h"

#include <spdlog.h>
#include <fmt/fmt.h>

#include "exceptions.h"
#include "util.h"

using fmt::literals::operator""_format;

static auto logger = getLogger("Bytecode");

CodeAttribute::CodeAttribute(std::vector<uint8_t> code, LocalVariableTable localVariables) : code(std::move(code)), localVariables(std::move(localVariables)) {
  init();
}

CodeAttribute::CodeAttribute(std::vector<uint8_t> code) : code(std::move(code)) {
  init();
}

void CodeAttribute::init() {
  for (size_t pos = 0; pos < code.size();) {
    instructions.push_back(pos);
    pos += getInstructionLength(pos);
  }
}

std::string CodeAttribute::toString(const ConstPool &constPool) const {
  return "";
}

std::string CodeAttribute::printInstruction(const ConstPool &constPool, size_t offset) const {
  uint8_t opcode = code[offset];

  switch (opcode) {
    case OpCodes::BIPUSH:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], code[offset + 1]);
    case OpCodes::SIPUSH:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1));
    case OpCodes::LDC:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], constPool.entryToString(code[offset + 1], false));
    case OpCodes::LDC_W:
    case OpCodes::LDC2_W: {
      const uint16_t constantIndex = ByteVectorUtil::readuint16(code, offset + 1);
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], constPool.entryToString(constantIndex, false));
    }
    case OpCodes::ILOAD:
    case OpCodes::LLOAD:
    case OpCodes::FLOAD:
    case OpCodes::DLOAD:
    case OpCodes::ALOAD:
    case OpCodes::ISTORE:
    case OpCodes::LSTORE:
    case OpCodes::FSTORE:
    case OpCodes::DSTORE:
    case OpCodes::ASTORE:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], printLocalVariable(code[offset + 1]));
    case OpCodes::ILOAD_0:
    case OpCodes::ILOAD_1:
    case OpCodes::ILOAD_2:
    case OpCodes::ILOAD_3:
    case OpCodes::LLOAD_0:
    case OpCodes::LLOAD_1:
    case OpCodes::LLOAD_2:
    case OpCodes::LLOAD_3:
    case OpCodes::FLOAD_0:
    case OpCodes::FLOAD_1:
    case OpCodes::FLOAD_2:
    case OpCodes::FLOAD_3:
    case OpCodes::DLOAD_0:
    case OpCodes::DLOAD_1:
    case OpCodes::DLOAD_2:
    case OpCodes::DLOAD_3:
    case OpCodes::ALOAD_0:
    case OpCodes::ALOAD_1:
    case OpCodes::ALOAD_2:
    case OpCodes::ALOAD_3:
    case OpCodes::ISTORE_0:
    case OpCodes::ISTORE_1:
    case OpCodes::ISTORE_2:
    case OpCodes::ISTORE_3:
    case OpCodes::LSTORE_0:
    case OpCodes::LSTORE_1:
    case OpCodes::LSTORE_2:
    case OpCodes::LSTORE_3:
    case OpCodes::FSTORE_0:
    case OpCodes::FSTORE_1:
    case OpCodes::FSTORE_2:
    case OpCodes::FSTORE_3:
    case OpCodes::DSTORE_0:
    case OpCodes::DSTORE_1:
    case OpCodes::DSTORE_2:
    case OpCodes::DSTORE_3:
    case OpCodes::ASTORE_0:
    case OpCodes::ASTORE_1:
    case OpCodes::ASTORE_2:
    case OpCodes::ASTORE_3: {
      uint8_t slot = opcodeSlot(opcode);
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], printLocalVariable(slot));
    }
    case OpCodes::IFEQ:
    case OpCodes::IFGE:
    case OpCodes::IFGT:
    case OpCodes::IFLE:
    case OpCodes::IFLT:
    case OpCodes::IFNE:
    case OpCodes::IFNONNULL:
    case OpCodes::IFNULL:
    case OpCodes::IF_ACMPEQ:
    case OpCodes::IF_ACMPNE:
    case OpCodes::IF_ICMPEQ:
    case OpCodes::IF_ICMPGE:
    case OpCodes::IF_ICMPGT:
    case OpCodes::IF_ICMPLE:
    case OpCodes::IF_ICMPLT:
    case OpCodes::IF_ICMPNE:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1) + offset);
    case OpCodes::IINC:
      return "{:<15} {}, {}"_format(Constants::OpcodeMnemonic[opcode], code[offset + 1], ByteVectorUtil::readint8(code, offset + 2));
    case OpCodes::GOTO:
    case OpCodes::JSR:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1) + offset);
    case OpCodes::RET:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], code[offset + 1]);
//    case OpCodes::TABLESWITCH:
//      return "";  //TODO
//    case OpCodes::LOOKUPSWITCH:
//      return "";  //TODO
    case OpCodes::GETSTATIC:       //Fieldref
    case OpCodes::PUTSTATIC:
    case OpCodes::GETFIELD:
    case OpCodes::PUTFIELD:
    case OpCodes::INVOKEVIRTUAL:   //Methodref
    case OpCodes::INVOKESPECIAL:
    case OpCodes::INVOKESTATIC:
    case OpCodes::INVOKEINTERFACE: //InterfaceMethodref
    case OpCodes::INVOKEDYNAMIC:   //MethodHandle
    case OpCodes::NEW:             //Class
    case OpCodes::CHECKCAST:
    case OpCodes::ANEWARRAY: {
      const uint16_t refIndex = ByteVectorUtil::readuint16(code, offset + 1);
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], constPool.entryToString(refIndex, false));
    }
    case OpCodes::MULTIANEWARRAY: {
      const uint16_t refIndex = ByteVectorUtil::readuint16(code, offset + 1);
      return "{:<15} {} {}"_format(Constants::OpcodeMnemonic[opcode], constPool.entryToString(refIndex, false), code[offset + 3]);
    }
    case OpCodes::NEWARRAY: {
      uint8_t type = code[offset + 1];
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], Constants::ArrayType[type]);
    }
    case OpCodes::GOTO_W:
    case OpCodes::JSR_W:
      return "{:<15} {}"_format(Constants::OpcodeMnemonic[opcode], ByteVectorUtil::readint16(code, offset + 1) + offset);
    case OpCodes::WIDE: {
      uint8_t wideOp = code[offset+1];
      if (wideOp == OpCodes::IINC) {
        return "{:<15} {}, {}"_format(
            Constants::OpcodeMnemonic[wideOp],
            ByteVectorUtil::readint16(code, offset + 2),
            ByteVectorUtil::readint16(code, offset + 4));
      } else {
        return "{:<15} {}"_format(Constants::OpcodeMnemonic[wideOp], ByteVectorUtil::readint16(code, offset + 2));
      }
    }
    default:
      return "{:<15}"_format(Constants::OpcodeMnemonic[opcode]);
  }
}

std::string CodeAttribute::printLocalVariable(uint8_t slot) const {
  auto[name, desc] = localVariables.getEntry(slot).value_or(std::make_tuple("?", "?"));
  return "{}: name={} type={}"_format(slot, name, desc);
}

uint8_t CodeAttribute::getInstructionLength(size_t offset) const {
  uint8_t opCode = code[offset];
  uint8_t len = Constants::InstructionLength[opCode];

  if (len != 0) return len;

  if (opCode == OpCodes::WIDE) {
    uint8_t wideOp = ByteVectorUtil::readuint8(code, offset + 1);
    if (wideOp == OpCodes::IINC) {
      return 6;
    } else {
      return 4;
    }
  } else if (opCode == OpCodes::TABLESWITCH) {
    size_t padding = 4 - (offset + 1) % 4;
    int32_t lowValue = ByteVectorUtil::readint32(code, offset + 1 + padding + 4);
    int32_t highValue = ByteVectorUtil::readint32(code, offset + 1 + padding + 8);
    // opcode, 0-3 padding, u4 default, u4 low, u4 high, (high - low + 1) * 4 (u4 offset)
    return 1 + padding + 4 + 4 + 4 + (highValue - lowValue + 1) * 4;
  } else if (opCode == OpCodes::LOOKUPSWITCH) {
    size_t padding = 4 - (offset + 1) % 4;
    int32_t npairs = ByteVectorUtil::readint32(code, offset + 1 + padding + 4);
    // opcode, 0-3 padding, u4 default, u4 npairs, npairs * 8(u4 key, u4 targetOffset)
    return 1 + padding + 4 + 4 + 8 * npairs;
  }

  throw InvalidArgument("TODO: Not implemented opcode {}"_format(opCode));
}

void InstructionPrintIterator::operator++(int) {
  if (offset > code.getSize()) {
    throw std::out_of_range("Current offset: {}, bytecode length: {}"_format(offset, code.getSize()));
  }

  offset += code.getInstructionLength(offset);
}

std::string InstructionPrintIterator::operator*() const {
  return code.printInstruction(constPool, offset);
}


std::tuple<uint16_t, uint8_t> InstructionIterator::operator*() const {
  return std::make_tuple(offset, code.getOpcode(offset));
}

void InstructionIterator::operator++(int) {
  if (offset > code.getSize()) {
    throw std::out_of_range("Current offset: {}, bytecode length: {}"_format(offset, code.getSize()));
  }

  offset += code.getInstructionLength(offset);
}
