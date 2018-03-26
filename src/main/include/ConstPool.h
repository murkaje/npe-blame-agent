#pragma once

#include <utility>
#include <vector>
#include <functional>
#include <iostream>
#include <memory>

class ConstPool;

class ConstInfo {
private:
  size_t index;

public:
  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  explicit ConstInfo(size_t _index) : index(_index) {}

  virtual ~ConstInfo() = default;

  virtual ConstInfo *clone() const = 0;

  virtual void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const = 0;

  virtual uint8_t getTag() const = 0;
};

class ConstInfoPadding : public ConstInfo {
public:
  static const uint8_t tag = 0;

  using ConstInfo::ConstInfo;

  ConstInfoPadding *clone() const override { return new ConstInfoPadding(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }
};

//std::ostream &operator<<(std::ostream &out, const ConstInfoPadding &constInfoPadding) {
//  return out << "padding\n";
//}

class UTF8Info : public ConstInfo {
private:
  std::string string;

public:
  static const uint8_t tag = 1;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  UTF8Info(std::string string, size_t index) : ConstInfo(index), string(std::move(string)) {}

  UTF8Info *clone() const override { return new UTF8Info(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  const std::string &getString() const { return string; }
};

//std::ostream &operator<<(std::ostream &out, const UTF8Info &utf8Info) {
//  return out << "UTF8 \"" << utf8Info.getString() << "\"\n";
//}

class IntegerInfo : public ConstInfo {
private:
  int32_t value;

public:
  static const uint8_t tag = 3;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  IntegerInfo(int32_t value, size_t index) : ConstInfo(index), value(value) {}

  IntegerInfo *clone() const override { return new IntegerInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  int32_t getValue() const { return value; }
};

//std::ostream &operator<<(std::ostream &out, const IntegerInfo &integerInfo) {
//  return out << "Integer " << integerInfo.getValue() << "\n";
//}

class FloatInfo : public ConstInfo {
private:
  float value;
public:
  static const uint8_t tag = 4;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  FloatInfo(float value, size_t index) : ConstInfo(index), value(value) {}

  FloatInfo *clone() const override { return new FloatInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  float getValue() const { return value; }
};

//std::ostream &operator<<(std::ostream &out, const FloatInfo &floatInfo) {
//  return out << "Float " << floatInfo.getValue() << "\n";
//}

class LongInfo : public ConstInfo {
private:
  int64_t value;
public:
  static const uint8_t tag = 5;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  LongInfo(int64_t value, size_t index) : ConstInfo(index), value(value) {}

  LongInfo *clone() const override { return new LongInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  int64_t getValue() const { return value; }
};

//std::ostream &operator<<(std::ostream &out, const LongInfo &longInfo) {
//  return out << "Long " << longInfo.getValue() << "\n";
//}

class DoubleInfo : public ConstInfo {
private:
  double value;
public:
  static const uint8_t tag = 6;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  DoubleInfo(double value, size_t index) : ConstInfo(index), value(value) {}

  DoubleInfo *clone() const override { return new DoubleInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  double getValue() const { return value; }
};

//std::ostream &operator<<(std::ostream &out, const DoubleInfo &doubleInfo) {
//  return out << "Double " << doubleInfo.getValue() << "\n";
//}

class ClassInfo : public ConstInfo {
private:
  uint16_t nameIndex;
public:
  static const uint8_t tag = 7;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  ClassInfo(uint16_t nameIndex, size_t index) : ConstInfo(index), nameIndex(nameIndex) {}

  ClassInfo *clone() const override { return new ClassInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  uint16_t getNameIndex() const { return nameIndex; }
};

//std::ostream &operator<<(std::ostream &out, const ClassInfo &classInfo) {
//  return out << "Class #" << classInfo.getNameIndex() << "\n";
//}

class StringInfo : public ConstInfo {
private:
  uint16_t stringIndex;
public:
  static const uint8_t tag = 8;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  StringInfo(uint16_t stringIndex, size_t index) : ConstInfo(index), stringIndex(stringIndex) {}

  StringInfo *clone() const override { return new StringInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  uint16_t getStringIndex() const { return stringIndex; }
};

//std::ostream &operator<<(std::ostream &out, const StringInfo &stringInfo) {
//  return out << "String #" << stringInfo.getStringIndex() << "\n";
//}

class MemberRefInfo : public ConstInfo {
private:
  uint16_t classIndex;
  uint16_t nameAndTypeIndex;

public:
  MemberRefInfo(uint16_t classIndex, uint16_t nameAndTypeIndex, size_t index) : ConstInfo(index), classIndex(classIndex), nameAndTypeIndex(nameAndTypeIndex) {}

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  virtual std::string getTagName() const = 0;

  uint16_t getClassIndex() const { return classIndex; }

  uint16_t getNameAndTypeIndex() const { return nameAndTypeIndex; }
};

//std::ostream &operator<<(std::ostream &out, const MemberRefInfo &memberRefInfo) {
//  return out << memberRefInfo.getTagName() << " #" << memberRefInfo.getClassIndex() << ".#" << memberRefInfo.getNameAndTypeIndex() << "\n";
//}

class FieldRefInfo : public MemberRefInfo {
public:
  static const uint8_t tag = 9;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  using MemberRefInfo::MemberRefInfo;

  FieldRefInfo *clone() const override { return new FieldRefInfo(*this); }

  uint8_t getTag() const override { return tag; }

  std::string getTagName() const override { return "Field"; }
};

class MethodRefInfo : public MemberRefInfo {
public:
  static const uint8_t tag = 10;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  using MemberRefInfo::MemberRefInfo;

  MethodRefInfo *clone() const override { return new MethodRefInfo(*this); }

  uint8_t getTag() const override { return tag; }

  std::string getTagName() const override { return "Method"; }
};

class InterfaceMethodRefInfo : public MemberRefInfo {
public:
  static const uint8_t tag = 11;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  using MemberRefInfo::MemberRefInfo;

  InterfaceMethodRefInfo *clone() const override { return new InterfaceMethodRefInfo(*this); }

  uint8_t getTag() const override { return tag; }

  std::string getTagName() const override { return "InterfaceMethod"; }
};

class NameAndTypeInfo : public ConstInfo {
private:
  uint16_t nameIndex;
  uint16_t descriptorIndex;
public:
  static const uint8_t tag = 12;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  NameAndTypeInfo(uint16_t nameIndex, uint16_t descriptorIndex, size_t index) : ConstInfo(index), nameIndex(nameIndex), descriptorIndex(descriptorIndex) {}

  NameAndTypeInfo *clone() const override { return new NameAndTypeInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  uint16_t getNameIndex() const { return nameIndex; }

  uint16_t getDescriptorIndex() const { return descriptorIndex; }
};

//std::ostream &operator<<(std::ostream &out, const NameAndTypeInfo &nameAndTypeInfo) {
//  return out << "NameAndType #" << nameAndTypeInfo.getNameIndex() << ":#" << nameAndTypeInfo.getDescriptorIndex() << "\n";
//}

class MethodHandleInfo : public ConstInfo {
private:
  uint8_t referenceKind;
  uint16_t referenceIndex;
public:
  static const uint8_t tag = 15;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  MethodHandleInfo(uint8_t _referenceKind, uint16_t _referenceIndex, uint16_t _index) : ConstInfo(_index), referenceKind(_referenceKind), referenceIndex(_referenceIndex) {}

  MethodHandleInfo *clone() const override { return new MethodHandleInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  uint8_t getReferenceKind() const { return referenceKind; }

  uint16_t getReferenceIndex() const { return referenceIndex; }
};

//std::ostream &operator<<(std::ostream &out, const MethodHandleInfo &methodHandleInfo) {
//  return out << "MethodHandle " << methodHandleInfo.getReferenceKind() << " #" << methodHandleInfo.getReferenceIndex() << "\n";
//}

class MethodTypeInfo : public ConstInfo {
private:
  uint16_t descriptorIndex;
public:
  static const uint8_t tag = 16;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  MethodTypeInfo(uint16_t _descriptorIndex, uint16_t _index) : ConstInfo(_index), descriptorIndex(_descriptorIndex) {}

  MethodTypeInfo *clone() const override { return new MethodTypeInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  uint16_t getDescriptorIndex() const { return descriptorIndex; }
};

//std::ostream &operator<<(std::ostream &out, const MethodTypeInfo &methodTypeInfo) {
//  return out << "MethodType #" << methodTypeInfo.getDescriptorIndex() << "\n";
//}

class InvokeDynamicInfo : public ConstInfo {
private:
  uint16_t bootstrapMethodAttrIndex;
  uint16_t nameAndTypeIndex;
public:
  static const uint8_t tag = 18;

  static std::unique_ptr<ConstInfo> read(const std::vector<uint8_t> &constPoolBytes, size_t &offset, size_t index);

  InvokeDynamicInfo(uint16_t _bootstrapMethodAttrIndex, uint16_t _nameAndTypeIndex, uint16_t _index) :
      ConstInfo(_index), bootstrapMethodAttrIndex(_bootstrapMethodAttrIndex), nameAndTypeIndex(_nameAndTypeIndex) {}

  InvokeDynamicInfo *clone() const override { return new InvokeDynamicInfo(*this); }

  void visit(const ConstPool &constPool, std::function<void(const std::string &)> visitor, bool firstLevel = true) const override;

  uint8_t getTag() const override { return tag; }

  uint16_t getBootstrapMethodAttrIndex() const { return bootstrapMethodAttrIndex; }

  uint16_t getNameAndTypeIndex() const { return nameAndTypeIndex; }
};

//std::ostream &operator<<(std::ostream &out, const InvokeDynamicInfo &invokeDynamicInfo) {
//  return out << "InvokeDynamic #" << invokeDynamicInfo.getBootstrapMethodAttrIndex() << ":#" << invokeDynamicInfo.getNameAndTypeIndex() << "\n";
//}

class ConstPool {
private:
  size_t indexPos = 1;
  std::vector<std::unique_ptr<ConstInfo>> entries;

  void read(const std::vector<uint8_t> &constPoolBytes);

  void read(std::ifstream, size_t count);

public:

  ConstPool() {
    entries.push_back(std::make_unique<ConstInfoPadding>(0));
  }

  ConstPool(const std::vector<uint8_t> &constPoolBytes) {
    entries.push_back(std::make_unique<ConstInfoPadding>(0));
    read(constPoolBytes);
  }

  const ConstInfo &get(size_t index) const {
    return *entries.at(index);
  }

  std::string entryToString(size_t index, bool identifier = true) const;

  void print() const;
};