#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cstdint>

#include "ConstPool.h"
#include "LocalVariableTable.h"
#include "Constants.h"

class CodeAttribute {
private:
  std::vector<uint8_t> code;
  std::vector<size_t> instructions; // Offset to code array indicating instruction
  //std::set<Attribute> - LineNumberTable? LocalVariableTable LocalVariableTypeTable

  LocalVariableTable localVariables;

  uint8_t opcodeSlot(uint8_t opCode) const;

public:
  CodeAttribute(std::vector<uint8_t> code, LocalVariableTable localVariables) : code(std::move(code)), localVariables(std::move(localVariables)) {}

  explicit CodeAttribute(std::vector<uint8_t> code) : code(std::move(code)) {}

  std::string toString(const ConstPool &constPool) const;

  std::string printInstruction(const ConstPool &constPool, size_t offset) const;

  std::string printLocalVariable(uint8_t slot) const;

  size_t getSize() const { return code.size(); }

  uint8_t getOpcode(size_t offset) const {
    //TODO:Check that byte at offset is opcode
    return code[offset];
  }
};

class InstructionIterator {
  const CodeAttribute &code;
  uint16_t offset = 0;

public:
  InstructionIterator(const CodeAttribute &code) : code(code) {}

  std::tuple<uint16_t, uint8_t> operator*() const;

  void operator++(int);

  size_t getOffset() const { return offset; }
};

class InstructionPrintIterator {
private:
  const CodeAttribute &code;
  const ConstPool &constPool;
  size_t offset = 0;

public:
  InstructionPrintIterator(const CodeAttribute &code, const ConstPool &constPool) : code(code), constPool(constPool) {}

  std::string operator*() const;

  void operator++(int);

  size_t getOffset() const { return offset; }
};