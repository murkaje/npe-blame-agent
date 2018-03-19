#include "Field.h"

#include "util.h"

Field::Field(std::string className, std::string fieldName, std::string typeName) :
    className(std::move(className)), fieldName(std::move(fieldName)), typeName(std::move(typeName)) {}

Field Field::readFromFieldInsn(const CodeAttribute &code, const ConstPool &constPool, size_t bci) {
  uint8_t opCode = code.getOpcode(bci);
  if (opCode != OpCodes::GETFIELD && opCode != OpCodes::PUTFIELD && opCode != OpCodes::GETSTATIC && opCode != OpCodes::PUTSTATIC) {
    throw std::invalid_argument("Opcode is not a field access");
  }

  uint16_t fieldRef = ByteVectorUtil::readuint16(code.getCode(), bci + 1);
  return readFromMemberRef(constPool, fieldRef);
}

Field Field::readFromMemberRef(const ConstPool &constPool, size_t refId) {
  const auto &memberRef = dynamic_cast<const MemberRefInfo &> (constPool.get(refId));
  const auto &nameAndTypeRef = dynamic_cast<const NameAndTypeInfo &>(constPool.get(memberRef.getNameAndTypeIndex()));

  std::string className = toJavaClassName(constPool.entryToString(memberRef.getClassIndex(), false));
  std::string fieldName = constPool.entryToString(nameAndTypeRef.getNameIndex(), false);
  std::string typeName = toJavaClassName(constPool.entryToString(nameAndTypeRef.getDescriptorIndex(), false));

  return Field(className, fieldName, typeName);
}