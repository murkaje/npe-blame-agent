#include "Method.h"

#include "util.h"

Method::Method(std::string className, std::string methodName, const std::string &signature) : className(std::move(className)), methodName(std::move(methodName)) {
  size_t pos = 0;

  if (signature.empty() || signature[pos] != '(') {
    throw std::invalid_argument("Signature must begin with '('");
  }
  pos++;

  std::string type;
  while (pos < signature.size()) {
    if (signature[pos] == ')') {
      pos++;
      continue;
    }
    type = toJavaTypeName(signature, pos, &pos);

    if (pos != signature.size()) {
      parameterTypes.push_back(type);
    }
    else {
      returnType = type;
    }

  }
}

Method Method::readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci) {
  uint8_t opCode = code.getOpcode(bci);
  if (opCode < OpCodes::INVOKEVIRTUAL || opCode > OpCodes::INVOKEINTERFACE) {
    throw std::invalid_argument(formatString("Opcode %d is not a known invoke", opCode));
  }

  const uint16_t refIndex = ByteVectorUtil::readuint16(code.getCode(), bci + 1);
  return readFromMemberRef(constPool, refIndex);
}

//TODO: checks
Method Method::readFromMemberRef(const ConstPool &constPool, size_t refId) {
  const auto &memberRef = dynamic_cast<const MemberRefInfo &> (constPool.get(refId));
  const auto &nameAndTypeRef = dynamic_cast<const NameAndTypeInfo &>(constPool.get(memberRef.getNameAndTypeIndex()));

  std::string className = toJavaClassName(constPool.entryToString(memberRef.getClassIndex(), false));
  std::string methodName = constPool.entryToString(nameAndTypeRef.getNameIndex(), false);
  std::string methodSignature = constPool.entryToString(nameAndTypeRef.getDescriptorIndex(), false);

  return Method(className, methodName, methodSignature);
}
