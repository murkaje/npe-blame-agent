#pragma once

#include <string>
#include <utility>
#include <spdlog.h>
#include <fmt/fmt.h>
#include "jni.h"
#include "exceptions.h"
#include "typestring.hh"
#include "assert.h"
#include "util.h"

using irqus::typestring;
using std::string_view;

template<typename T, size_t MaxSize>
class CompileTimeList {
  constexpr static size_t capacity = MaxSize;
  size_t curSize = 0;
  T data[MaxSize] = {};

public:
  constexpr CompileTimeList() = default;

  constexpr T &operator[](size_t index) { return data[index]; }

  constexpr const T &operator[](size_t index) const { return data[index]; }

  constexpr const T *begin() const { return data; }

  constexpr const T *end() const { return data + curSize; }

  constexpr T &back() { return data[curSize - 1]; }

  constexpr size_t size() const { return size; }

  constexpr void add(const T &val) {
    curSize++;
    back() = val;
  }
};

#define THROW(x) do { static_cast<void>(sizeof(x)); assert(false); } while(false);

enum class Type {
  InvalidType, IntType, LongType, FloatType, DoubleType, CharType, ByteType, ShortType, BoolType, VoidType, StringType, ObjectType
};

constexpr auto typeOf(char typeChar) {
  switch (typeChar) {
    case 'I':
      return Type::IntType;
    case 'J':
      return Type::LongType;
    case 'F':
      return Type::FloatType;
    case 'D':
      return Type::DoubleType;
    case 'C':
      return Type::CharType;
    case 'B':
      return Type::ByteType;
    case 'S':
      return Type::ShortType;
    case 'Z':
      return Type::BoolType;
    case 'V':
      return Type::VoidType;
    case '\0':
      THROW("typeChar is NUL")
    case ')':
      THROW("typeChar is ')'")
    case 'L':
      THROW("typeChar is 'L'")
    default:
      THROW("typeChar is something")
  }
}

constexpr auto typeOf(string_view typeString) {
  if (typeString.length() == 1) {
    return typeOf(typeString[0]);
  }
  if (typeString[0] != 'L') {
    THROW("Class signature doesn't start with L")
  }
  if (typeString.back() != ';') {
    THROW("Class signature doesn't end with ;")
  }
  //From c++20 use starts_with
  if (typeString == "Ljava/lang/String;") {
    return Type::StringType;
  } else {
    return Type::ObjectType;
  }
}

struct UnexpectedType {
  void error(string_view message) {
  }

  constexpr UnexpectedType(string_view message, size_t idx) {
    error("");
  }
};

template<typename DeclaredType, typename SignatureChecker>
struct TypeChecker {

  constexpr static void check(SignatureChecker &context) {
    size_t idx = context.nextIndex();
    Type t = context.getArgType(idx);

    if (t == Type::IntType) {
      constexpr bool matches = std::is_same_v<int32_t, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int32 at parameter index=", idx);
    } else if (t == Type::LongType) {
      constexpr bool matches = std::is_same_v<int64_t, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int64 at parameter index=", idx);
    } else if (t == Type::ShortType) {
      constexpr bool matches = std::is_same_v<int16_t, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int16 at parameter index=", idx);
    } else if (t == Type::CharType) {
      constexpr bool matches = std::is_same_v<int16_t, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int16 at parameter index=", idx);
    } else if (t == Type::ByteType) {
      constexpr bool matches = std::is_same_v<int8_t, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int8 at parameter index=", idx);
    } else if (t == Type::FloatType) {
      constexpr bool matches = std::is_same_v<float, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected float at parameter index=", idx);
    } else if (t == Type::DoubleType) {
      constexpr bool matches = std::is_same_v<double, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected double at parameter index=", idx);
    } else if (t == Type::BoolType) {
      constexpr bool matches = std::is_same_v<bool, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected bool at parameter index=", idx);
    } else if (t == Type::ObjectType) {
      constexpr bool matches = std::is_same_v<jobject, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected jobject at parameter index=", idx);
    } else if (t == Type::StringType) {
      constexpr bool matches = std::is_same_v<std::string_view, DeclaredType> || std::is_same_v<std::string, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected std::string or std::string_view at parameter index=", idx);
    } else {
      THROW("Invalid type");
    }
  }
};

using fmt::literals::operator ""_format;

template<typename ArgType>
struct FieldSignatureChecker {

  const Type fieldType;

  explicit constexpr FieldSignatureChecker(string_view signature) : fieldType(typeOf(signature)) {
    TypeChecker<ArgType, FieldSignatureChecker>::check(*this);
  }

  constexpr size_t nextIndex() {
    return 0;
  }

  constexpr Type getArgType(size_t idx) const {
    return fieldType;
  }

  constexpr Type getFieldType() const {
    return fieldType;
  }
};

template<typename... ArgType>
struct SignatureChecker {

  using ChekerFun = void (*)(SignatureChecker &checker);

  constexpr static size_t argc = sizeof...(ArgType);
  constexpr static ChekerFun checkers[argc]{&TypeChecker<ArgType, SignatureChecker>::check...};

  CompileTimeList<Type, argc + 1> types;
  size_t curIndex = 0;

  explicit constexpr SignatureChecker(string_view signature) {
    auto it = signature.begin();
    auto start = it;
    bool parsingObject = false;
    size_t index = 0;

    assert(it != signature.end() && *it == '(');
    it++;

    while (it != signature.end()) {
      char typeChar = *it++;

      if (typeChar == ')') {
        if (parsingObject) {
          THROW("Reached end of parameters while reading object type from signature. Missing ';' somewhere?");
        }
        if (index != argc) {
          THROW("Signature specifies more arguments than passed to function");
        }
        continue;
      }

      if (index == argc + 1) {
        THROW("Signature specifies more types than passed to function");
      }

      if (parsingObject) {
        if (typeChar == ';') {
          Type type = typeOf(string_view(start, it - start));
          if (type == Type::InvalidType) {
            THROW("Unexpected character in signature");
          }
          types[index++] = type;
          parsingObject = false;
        }
      } else {
        if (typeChar == 'L') {
          start = it - 1;
          parsingObject = true;
        } else {
          types[index++] = typeOf(typeChar);
        }
      }
    }

    if (parsingObject) {
      THROW("Reached end of signature while reading object type. Missing ';' somewhere?");
    }

    for (auto checkFun : checkers) {
      checkFun(*this);
    }
  }

  constexpr size_t nextIndex() {
    if (curIndex == argc) {
      THROW("Signature specifies more types than passed to function");
    }
    return curIndex++;
  }

  constexpr Type getArgType(size_t idx) const {
    return types[idx];
  }

  constexpr Type getRetType() const {
    return types[argc];
  }
};

#define jnisig(x) typestring_is(x){}

class Jni {
  inline static thread_local JNIEnv *jni = nullptr;
  inline static std::shared_ptr<spdlog::logger> logger = getLogger("JVMTI");

  struct JString {
    jstring elem;
    JString() = default;
    JString(std::string_view str) {
      elem = jni->NewStringUTF(str.data());
      checkJniException(jni);
    }
    //Not trivial...
    ~JString() noexcept(false) {
      jni->DeleteLocalRef(elem);
      checkJniException(jni);
    }
    operator jstring() const {
      return elem;
    }
  };

  struct JniScopedExceptionChecker {
    ~JniScopedExceptionChecker() noexcept(false) {
      checkJniException(jni);
    }
  };

  template<class PassedType>
  static auto toJniType(PassedType arg) {
    return arg;
  }

  static auto toJniType(std::string_view arg) {
    //TODO: Figure out how to fix leak, can't use any destructors as those become non-trivial types
    //TODO: Perhaps add it to some list for later cleanup?
    return jni->NewStringUTF(arg.data());
  }

public:

  static void ensureInit(JNIEnv* tjni) {
    jni = tjni;
  }

  //TODO: arrays, e.g. invokeVirtual(obj, method, ([B)V, (std::vector<char>? char[]?))
  //TODO: new, invokespecial

  template<char... Signature>
  static auto getStatic(jclass klass, string_view fieldName, typestring<Signature...> signature) {
    jfieldID fieldId = jni->GetFieldID(klass, fieldName.data(), signature.data());
    checkJniException(jni);

    constexpr Type fieldType = typeOf(signature.data());
    JniScopedExceptionChecker beforeReturn;
    if constexpr (fieldType == Type::IntType) {
      return jni->GetStaticIntField(klass, fieldId);
    } else if constexpr (fieldType == Type::LongType) {
      return jni->GetStaticLongField(klass, fieldId);
    } else if constexpr (fieldType == Type::ShortType) {
      return jni->GetStaticShortField(klass, fieldId);
    } else if constexpr (fieldType == Type::CharType) {
      return jni->GetStaticCharField(klass, fieldId);
    } else if constexpr (fieldType == Type::ByteType) {
      return jni->GetStaticByteField(klass, fieldId);
    } else if constexpr (fieldType == Type::FloatType) {
      return jni->GetStaticFloatField(klass, fieldId);
    } else if constexpr (fieldType == Type::DoubleType) {
      return jni->GetStaticDoubleField(klass, fieldId);
    } else if constexpr (fieldType == Type::StringType) {
      return jstringToString(jni, (jstring) jni->GetStaticObjectField(klass, fieldId));
    } else if constexpr (fieldType == Type::ObjectType) {
      return jni->GetStaticObjectField(klass, fieldId);
    } else if constexpr (fieldType == Type::BoolType) {
      return jni->GetStaticBooleanField(klass, fieldId);
    } else {
      throw JniError("Unknown field type");
    }
  }

  template<char... Signature, typename ArgType>
  static void putStatic(jclass klass, string_view fieldName, typestring<Signature...> signature, ArgType value) {
    constexpr FieldSignatureChecker<ArgType> checker(signature.data());

    jfieldID fieldId = jni->GetFieldID(klass, fieldName.data(), signature.data());
    checkJniException(jni);

    constexpr Type fieldType = checker.getFieldType();
    if constexpr (fieldType == Type::IntType) {
      jni->SetStaticIntField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::LongType) {
      jni->SetStaticLongField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::ShortType) {
      jni->SetStaticShortField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::CharType) {
      jni->SetStaticCharField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::ByteType) {
      jni->SetStaticByteField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::FloatType) {
      jni->SetStaticFloatField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::DoubleType) {
      jni->SetStaticDoubleField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::StringType) {
      jni->SetStaticObjectField(klass, fieldId, JString(value));
    } else if constexpr (fieldType == Type::ObjectType) {
      jni->SetStaticObjectField(klass, fieldId, value);
    } else if constexpr (fieldType == Type::BoolType) {
      jni->SetStaticBooleanField(klass, fieldId, value);
    } else {
      throw JniError("Unknown field type");
    }
    checkJniException(jni);
  }

  template<char... Signature>
  static auto getField(jobject object, string_view fieldName, typestring<Signature...> signature) {
    jclass klass = jni->GetObjectClass(object);
    checkJniException(jni);
    jfieldID fieldId = jni->GetFieldID(klass, fieldName.data(), signature.data());
    checkJniException(jni);

    constexpr Type fieldType = typeOf(signature.data());
    JniScopedExceptionChecker beforeReturn;
    if constexpr (fieldType == Type::IntType) {
      return jni->GetIntField(object, fieldId);
    } else if constexpr (fieldType == Type::LongType) {
      return jni->GetLongField(object, fieldId);
    } else if constexpr (fieldType == Type::ShortType) {
      return jni->GetShortField(object, fieldId);
    } else if constexpr (fieldType == Type::CharType) {
      return jni->GetCharField(object, fieldId);
    } else if constexpr (fieldType == Type::ByteType) {
      return jni->GetByteField(object, fieldId);
    } else if constexpr (fieldType == Type::FloatType) {
      return jni->GetFloatField(object, fieldId);
    } else if constexpr (fieldType == Type::DoubleType) {
      return jni->GetDoubleField(object, fieldId);
    } else if constexpr (fieldType == Type::StringType) {
      return jstringToString(jni, (jstring) jni->GetObjectField(object, fieldId));
    } else if constexpr (fieldType == Type::ObjectType) {
      return jni->GetObjectField(object, fieldId);
    } else if constexpr (fieldType == Type::BoolType) {
      return jni->GetBooleanField(object, fieldId);
    } else {
      throw JniError("Unknown field type");
    }
  }

  template<char... Signature, typename ArgType>
  static void putField(jobject object, string_view fieldName, typestring<Signature...> signature, ArgType value) {
    constexpr FieldSignatureChecker<ArgType> checker(signature.data());

    jclass klass = jni->GetObjectClass(object);
    checkJniException(jni);
    jfieldID fieldId = jni->GetFieldID(klass, fieldName.data(), signature.data());
    checkJniException(jni);

    constexpr Type fieldType = checker.getFieldType();
    if constexpr (fieldType == Type::IntType) {
      jni->SetIntField(object, fieldId, value);
    } else if constexpr (fieldType == Type::LongType) {
      jni->SetLongField(object, fieldId, value);
    } else if constexpr (fieldType == Type::ShortType) {
      jni->SetShortField(object, fieldId, value);
    } else if constexpr (fieldType == Type::CharType) {
      jni->SetCharField(object, fieldId, value);
    } else if constexpr (fieldType == Type::ByteType) {
      jni->SetByteField(object, fieldId, value);
    } else if constexpr (fieldType == Type::FloatType) {
      jni->SetFloatField(object, fieldId, value);
    } else if constexpr (fieldType == Type::DoubleType) {
      jni->SetDoubleField(object, fieldId, value);
    } else if constexpr (fieldType == Type::StringType) {
      jni->SetObjectField(object, fieldId, JString(value));
    } else if constexpr (fieldType == Type::ObjectType) {
      jni->SetObjectField(object, fieldId, value);
    } else if constexpr (fieldType == Type::BoolType) {
      jni->SetBooleanField(object, fieldId, value);
    } else {
      throw JniError("Unknown field type");
    }
    checkJniException(jni);
  }

  template<char... Signature, typename... ArgType>
  static auto invokeStatic(jclass klass, string_view methodName, typestring<Signature...> signature, ArgType... args) {
    constexpr SignatureChecker<ArgType...> checker(signature.data());

    jmethodID methodId = jni->GetMethodID(klass, methodName.data(), signature.data());
    checkJniException(jni);

    constexpr auto retType = checker.getRetType();
    JniScopedExceptionChecker beforeReturn;
    if constexpr (retType == Type::StringType) {
      return jstringToString(jni, (jstring) jni->CallStaticObjectMethod(klass, methodId, toJniType(args)...));
    } else if constexpr (retType == Type::IntType) {
      return jni->CallStaticIntMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::LongType) {
      return jni->CallStaticLongMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::ShortType) {
      return jni->CallStaticShortMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::CharType) {
      return jni->CallStaticCharMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::ByteType) {
      return jni->CallStaticByteMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::FloatType) {
      return jni->CallStaticFloatMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::DoubleType) {
      return jni->CallStaticDoubleMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::ObjectType) {
      return jni->CallStaticObjectMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::BoolType) {
      return jni->CallStaticBooleanMethod(klass, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::VoidType) {
      jni->CallStaticVoidMethod(klass, methodId, toJniType(args)...);
    } else {
      throw JniError("Unknown return type");
    }
  }

  template<char... Signature, typename... ArgType>
  static auto invokeVirtual(jobject object, string_view methodName, typestring<Signature...> signature, ArgType... args) {
    constexpr SignatureChecker<ArgType...> checker(signature.data());

    jclass klass = jni->GetObjectClass(object);
    checkJniException(jni);
    jmethodID methodId = jni->GetMethodID(klass, methodName.data(), signature.data());
    checkJniException(jni);

    constexpr auto retType = checker.getRetType();
    JniScopedExceptionChecker beforeReturn;
    if constexpr (retType == Type::StringType) {
      return jstringToString(jni, (jstring) jni->CallObjectMethod(object, methodId, toJniType(args)...));
    } else if constexpr (retType == Type::IntType) {
      return jni->CallIntMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::LongType) {
      return jni->CallLongMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::ShortType) {
      return jni->CallShortMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::CharType) {
      return jni->CallCharMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::ByteType) {
      return jni->CallByteMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::FloatType) {
      return jni->CallFloatMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::DoubleType) {
      return jni->CallDoubleMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::ObjectType) {
      return jni->CallObjectMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::BoolType) {
      return jni->CallBooleanMethod(object, methodId, toJniType(args)...);
    } else if constexpr (retType == Type::VoidType) {
      jni->CallVoidMethod(object, methodId, toJniType(args)...);
    } else {
      throw JniError("Unknown return type");
    }
  }

  static jclass getClass(jobject obj) {
    jclass cls = jni->GetObjectClass(obj);
    checkJniException(jni);
    return cls;
  }

  static void test() {
    using std::string_view_literals::operator ""sv;

    std::string a = invokeVirtual(nullptr, "", jnisig("(IIJLjava/lang/String;I)Ljava/lang/String;"), 1, 1, 3l, ""sv, 1);
//    int i = Jni::invokeVirtual(nullptr, "", jnisig("()V"));
//    int f = Jni::invokeVirtual(nullptr, "", jnisig("()I"), 13);
//    std::string b = invokeVirtual(nullptr, "", jnisig("(Ljava/lang/ObjectIIJ)Ljava/lang/String"), nullptr, 1, 2, 3l);
    [[maybe_unused]]
    bool c = invokeStatic(nullptr, "", jnisig("(IIJLjava/lang/String;I)Z"), 1, 1, 3l, ""sv, 1);
  }
};
