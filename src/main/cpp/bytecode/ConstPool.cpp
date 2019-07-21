#include "bytecode/ConstPool.h"

#include <fstream>
#include <spdlog.h>

#include "bytecode/Constants.h"
#include "util.h"

using fmt::literals::operator""_format;

static auto logger = getLogger("Bytecode");

// ****************************************
// ******      Print Visitors       *******
// ****************************************

void ConstInfoPadding::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  visitor("padding");
}

void UTF8Info::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(string);
}

void IntegerInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(std::to_string(value));
}

void FloatInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(std::to_string(value));
}

void LongInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(std::to_string(value));
}

void DoubleInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(std::to_string(value));
}

void ClassInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  constPool.get(nameIndex).visit(constPool, visitor, false);
}

void StringInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  constPool.get(stringIndex).visit(constPool, visitor, false);
}

void MemberRefInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[getTag()]) + " ");
  }
  constPool.get(classIndex).visit(constPool, visitor, false);
  visitor(".");
  constPool.get(nameAndTypeIndex).visit(constPool, visitor, false);
}

void NameAndTypeInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  constPool.get(nameIndex).visit(constPool, visitor, false);
  visitor(":");
  constPool.get(descriptorIndex).visit(constPool, visitor, false);
}

void MethodHandleInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(std::string(Constants::ReferenceKindMnemonic[referenceKind]) + " ");
  constPool.get(referenceIndex).visit(constPool, visitor, false);
}

void MethodTypeInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  constPool.get(descriptorIndex).visit(constPool, visitor, false);
}

void InvokeDynamicInfo::visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel) const {
  if (firstLevel) {
    visitor(std::string(Constants::CpInfoMnemonic[tag]) + " ");
  }
  visitor(std::to_string(bootstrapMethodAttrIndex) + " ");
  constPool.get(nameAndTypeIndex).visit(constPool, visitor, false);
}

// ****************************************
// ******       Deserializers       *******
// ****************************************

std::unique_ptr<ConstInfo> ConstInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint8_t tag = constPoolBytes[offset];
  offset++;
  switch (tag) {
    case CpInfo::Utf8:
      return UTF8Info::read(constPoolBytes, offset, index);
    case CpInfo::Integer:
      return IntegerInfo::read(constPoolBytes, offset, index);
    case CpInfo::Float:
      return FloatInfo::read(constPoolBytes, offset, index);
    case CpInfo::Long:
      return LongInfo::read(constPoolBytes, offset, index);
    case CpInfo::Double:
      return DoubleInfo::read(constPoolBytes, offset, index);
    case CpInfo::Class:
      return ClassInfo::read(constPoolBytes, offset, index);
    case CpInfo::String:
      return StringInfo::read(constPoolBytes, offset, index);
    case CpInfo::Fieldref:
      return FieldRefInfo::read(constPoolBytes, offset, index);
    case CpInfo::Methodref:
      return MethodRefInfo::read(constPoolBytes, offset, index);
    case CpInfo::InterfaceMethodref:
      return InterfaceMethodRefInfo::read(constPoolBytes, offset, index);
    case CpInfo::NameAndType:
      return NameAndTypeInfo::read(constPoolBytes, offset, index);
    case CpInfo::MethodHandle:
      return MethodHandleInfo::read(constPoolBytes, offset, index);
    case CpInfo::MethodType:
      return MethodTypeInfo::read(constPoolBytes, offset, index);
    case CpInfo::InvokeDynamic:
      return InvokeDynamicInfo::read(constPoolBytes, offset, index);
    default:
      throw std::runtime_error("Unexpected tag {} at offset {}"_format(tag, offset));
  }
}

std::unique_ptr<ConstInfo> UTF8Info::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t utflen = ByteVectorUtil::readuint16(constPoolBytes, offset);
  offset += 2;

  std::string utf8String((const char *) &constPoolBytes[offset], utflen);
  offset += utflen;

  return std::make_unique<UTF8Info>(utf8String, index);
}

std::unique_ptr<ConstInfo> IntegerInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  int32_t value = ByteVectorUtil::readint32(constPoolBytes, offset);
  offset += 4;

  return std::make_unique<IntegerInfo>(value, index);
}

std::unique_ptr<ConstInfo> FloatInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  float value = ByteVectorUtil::readfloat(constPoolBytes, offset);
  offset += 4;

  return std::make_unique<FloatInfo>(value, index);
}

std::unique_ptr<ConstInfo> LongInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  long value = ByteVectorUtil::readint64(constPoolBytes, offset);
  offset += 8;

  return std::make_unique<LongInfo>(value, index);
}

std::unique_ptr<ConstInfo> DoubleInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  double value = ByteVectorUtil::readdouble(constPoolBytes, offset);
  offset += 8;

  return std::make_unique<DoubleInfo>(value, index);
}

std::unique_ptr<ConstInfo> ClassInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t nameIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  offset += 2;

  return std::make_unique<ClassInfo>(nameIndex, index);
}

std::unique_ptr<ConstInfo> StringInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t stringIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  offset += 2;

  return std::make_unique<StringInfo>(stringIndex, index);
}

std::unique_ptr<ConstInfo> FieldRefInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t classIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  uint16_t nameAndTypeIndex = ByteVectorUtil::readuint16(constPoolBytes, offset + 2);
  offset += 4;

  return std::make_unique<FieldRefInfo>(classIndex, nameAndTypeIndex, index);
}

std::unique_ptr<ConstInfo> MethodRefInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t classIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  uint16_t nameAndTypeIndex = ByteVectorUtil::readuint16(constPoolBytes, offset + 2);
  offset += 4;

  return std::make_unique<MethodRefInfo>(classIndex, nameAndTypeIndex, index);
}

std::unique_ptr<ConstInfo> InterfaceMethodRefInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t classIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  uint16_t nameAndTypeIndex = ByteVectorUtil::readuint16(constPoolBytes, offset + 2);
  offset += 4;

  return std::make_unique<InterfaceMethodRefInfo>(classIndex, nameAndTypeIndex, index);
}

std::unique_ptr<ConstInfo> NameAndTypeInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t nameIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  uint16_t descriptorIndex = ByteVectorUtil::readuint16(constPoolBytes, offset + 2);
  offset += 4;

  return std::make_unique<NameAndTypeInfo>(nameIndex, descriptorIndex, index);
}

std::unique_ptr<ConstInfo> MethodHandleInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint8_t referenceKind = constPoolBytes[offset];
  uint16_t referenceIndex = ByteVectorUtil::readuint16(constPoolBytes, offset + 1);
  offset += 3;

  return std::make_unique<MethodHandleInfo>(referenceKind, referenceIndex, index);
}

std::unique_ptr<ConstInfo> MethodTypeInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t descriptorIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  offset += 2;

  return std::make_unique<MethodTypeInfo>(descriptorIndex, index);
}

std::unique_ptr<ConstInfo> InvokeDynamicInfo::read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index) {
  uint16_t bootstrapMethodAttrIndex = ByteVectorUtil::readuint16(constPoolBytes, offset);
  uint16_t nameAndTypeIndex = ByteVectorUtil::readuint16(constPoolBytes, offset + 2);
  offset += 4;

  return std::make_unique<InvokeDynamicInfo>(bootstrapMethodAttrIndex, nameAndTypeIndex, index);
}

void ConstPool::read(const std::vector<uint8_t> &constPoolBytes) {
  entries.reserve(constPoolBytes.size() / 4);
  size_t readPos = 0;
  while (readPos < constPoolBytes.size()) {
    std::unique_ptr<ConstInfo> constPoolEntry = ConstInfo::read(constPoolBytes, readPos, indexPos);
    uint8_t tag = constPoolEntry->getTag();
//    log.debug(formatString("#%d\t%d=%s", indexPos, constPoolEntry->getTag(), Constants::CpInfoMnemonic[constPoolEntry->getTag()]));
//    log.debug(formatString("Readpos: %d / %d", readPos, constPoolBytes.size()));
    entries.push_back(std::move(constPoolEntry));
    indexPos++;
    if (tag == CpInfo::Long || tag == CpInfo::Double) {
      entries.push_back(std::make_unique<ConstInfoPadding>(indexPos));
      indexPos++;
    }
  }
}

void ConstPool::read(std::ifstream classFileStream, size_t count) {
  entries.reserve(count);

  while (indexPos < count - 1) {
    //std::unique_ptr<ConstInfo> constPoolEntry = ConstInfo::read(classFileStream, indexPos);
  }
}

void ConstPool::print() const {
  for (size_t index = 0; index < entries.size(); index++) {
    logger->info("#{:<5}\t{}", index, entryToString(index, false));
  }
}

std::string ConstPool::entryToString(size_t index, bool identifier) const {
  if (index >= entries.size()) {
    return "#{}"_format(index);
  }

  std::string entryString;
  entries[index]->visit(*this, [&](const std::string &str) -> void { entryString += str; }, identifier);
  return entryString;
}
