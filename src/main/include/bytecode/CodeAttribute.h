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

  void init();

public:
  CodeAttribute(std::vector<uint8_t> code, LocalVariableTable localVariables);

  explicit CodeAttribute(std::vector<uint8_t> code);

  std::string toString(const ConstPool &constPool) const;

  std::string printInstruction(const ConstPool &constPool, size_t offset) const;

  std::string printLocalVariable(uint8_t slot) const;

  size_t getSize() const { return code.size(); }

  uint8_t getOpcode(size_t offset) const {
    //TODO:Check that byte at offset is opcode
    //Do we allow handlers of wide to see opcode?
    return code[offset];
  }

  uint8_t getInstructionLength(size_t offset) const;

  const std::vector<uint8_t> &getCode() const { return code; }

  const std::vector<size_t> &getInstructions() const { return instructions; }

  //TODO: Methods for accessing specific refs, e.g. method signature
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