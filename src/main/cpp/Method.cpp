#include "Method.h"

#include "util.h"

using fmt::literals::operator ""_format;

Method::Method(std::string_view className, std::string_view methodName, std::string_view signature, uint32_t modifiers) :
    modifiers(modifiers), className(className), methodName(methodName), signature(signature), parameterLength(0) {
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
      if (type == "long" || type == "double") {
        parameterLength += 2;
      } else {
        parameterLength++;
      }
    } else {
      returnType = type;
    }
  }
}

Method Method::readFromCodeInvoke(const CodeAttribute &code, const ConstPool &constPool, size_t bci) {
  uint8_t opCode = code.getOpcode(bci);
  if (opCode < OpCodes::INVOKEVIRTUAL || opCode > OpCodes::INVOKEINTERFACE) {
    throw std::invalid_argument("Opcode {} is not a known invoke"_format(opCode));
  }

  uint16_t refIndex = ByteVectorUtil::readuint16(code.getCode(), bci + 1);
  //TODO: Creating Method with limited info - keep universal class or separate for performant | jvmti->GetModifiers variant?
  bool isStatic = opCode == OpCodes::INVOKESTATIC;
  return readFromMemberRef(constPool, refIndex, (isStatic ? Modifier::STATIC : 0));
}

//TODO: checks
Method Method::readFromMemberRef(const ConstPool &constPool, size_t refId, uint32_t modifiers) {
  const auto &memberRef = dynamic_cast<const MemberRefInfo &> (constPool.get(refId));
  const auto &nameAndTypeRef = dynamic_cast<const NameAndTypeInfo &>(constPool.get(memberRef.getNameAndTypeIndex()));

  std::string className = toJavaClassName(constPool.entryToString(memberRef.getClassIndex(), false));
  std::string methodName = constPool.entryToString(nameAndTypeRef.getNameIndex(), false);
  std::string methodSignature = constPool.entryToString(nameAndTypeRef.getDescriptorIndex(), false);

  return Method{className, methodName, methodSignature, modifiers};
}
