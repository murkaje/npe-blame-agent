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
      constexpr bool matches = std::is_same_v<std::string, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected std::string at parameter index=", idx);
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

//  void onError(const char *message) {
//    do {
//      static_cast<void>(sizeof(message));
//      assert(false);
//    } while (false);
//  }

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
      THROW("Reached end of signature while reading object type from signature. Missing ';' somewhere?");
    }

    for (auto checkFun : checkers) {
      checkFun(*this);
    }
  }

//  void onError(const char *message) {
//    do {
//      static_cast<void>(sizeof(message));
//      assert(false);
//    } while (false);
//  }

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

using std::string_view_literals::operator ""sv;
using std::string_literals::operator ""s;
#define jnisig(x) typestring_is(x){}

class Jni {
  inline static thread_local JNIEnv *jni = nullptr;
  inline static std::shared_ptr<spdlog::logger> logger = getLogger("JVMTI");

  template<class PassedType>
  static auto toJniType(PassedType arg) {
    return arg;
  }

  // TODO: Return RAII container with implicit conversion to string_view?
  template<>
  static auto toJniType<std::string>(std::string arg) {
    return jni->NewStringUTF(arg.data());
  }

  template<Type ExpectedType, class T>
  static auto toNativeType(T arg) {
    return arg;
  }

//  template<>
//  static auto toNativeType<Type::StringType>(jstring arg) {
//    return jstringToString(jni, arg);
//  }

public:

  static void ensureInit(JNIEnv* tjni) {
    jni = tjni;
  }

  static JNIEnv* getJni() {
    return jni;
  }

  //TODO: putStatic, getStatic, invokeStatic, ARRAYS!!!(as args and return types)

  template<char... Signature>
  static auto getField(jobject object, string_view fieldName, typestring<Signature...> signature) {
    jclass klass = jni->GetObjectClass(object);
    checkJniException(jni);
    jfieldID fieldId = jni->GetFieldID(klass, fieldName.data(), signature.data());
    checkJniException(jni);

    constexpr Type t = typeOf(signature.data());
    if constexpr (t == Type::IntType) {
      return jni->GetIntField(object, fieldId);
    } else if constexpr (t == Type::LongType) {
      return jni->GetLongField(object, fieldId);
    } else if constexpr (t == Type::ShortType) {
      return jni->GetShortField(object, fieldId);
    } else if constexpr (t == Type::CharType) {
      return jni->GetCharField(object, fieldId);
    } else if constexpr (t == Type::ByteType) {
      return jni->GetByteField(object, fieldId);
    } else if constexpr (t == Type::FloatType) {
      return jni->GetFloatField(object, fieldId);
    } else if constexpr (t == Type::DoubleType) {
      return jni->GetDoubleField(object, fieldId);
    } else if constexpr (t == Type::StringType) {
      return jstringToString(jni, (jstring) jni->GetObjectField(object, fieldId));
    } else if constexpr (t == Type::ObjectType) {
      return jni->GetObjectField(object, fieldId);
    } else if constexpr (t == Type::BoolType) {
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

    constexpr Type t = checker.getFieldType();
    if constexpr (t == Type::IntType) {
      jni->SetIntField(object, fieldId, value);
    } else if constexpr (t == Type::LongType) {
      jni->SetLongField(object, fieldId, value);
    } else if constexpr (t == Type::ShortType) {
      jni->SetShortField(object, fieldId, value);
    } else if constexpr (t == Type::CharType) {
      jni->SetCharField(object, fieldId, value);
    } else if constexpr (t == Type::ByteType) {
      jni->SetByteField(object, fieldId, value);
    } else if constexpr (t == Type::FloatType) {
      jni->SetFloatField(object, fieldId, value);
    } else if constexpr (t == Type::DoubleType) {
      jni->SetDoubleField(object, fieldId, value);
    } else if constexpr (t == Type::StringType) {
      jstring str = jni->NewStringUTF(value.data());
      jni->SetObjectField(object, fieldId, str);
      jni->DeleteLocalRef(str);
    } else if constexpr (t == Type::ObjectType) {
      jni->SetObjectField(object, fieldId, value);
    } else if constexpr (t == Type::BoolType) {
      jni->SetBooleanField(object, fieldId, value);
    } else {
      throw JniError("Unknown field type");
    }
  }

  template<char... Signature, typename... ArgType>
  static auto invokeVirtual(jobject object, string_view methodName, typestring<Signature...> signature, ArgType... args) {
    constexpr SignatureChecker<ArgType...> checker(signature.data());

    jclass klass = jni->GetObjectClass(object);
    checkJniException(jni);
    jmethodID methodId = jni->GetMethodID(klass, methodName.data(), signature.data());
    checkJniException(jni);

    constexpr auto R = checker.getRetType();
//    if(logger->should_log(spdlog::level::debug)) {
//      std::string className = invokeVirtual(klass, "getName", jnisig("()Ljava/lang/String;"));
//      logger->debug("Jni::invokeVirtual {}", signature.data());
//    }
    if constexpr (R == Type::StringType) {
      return jstringToString(jni, (jstring) jni->CallObjectMethod(object, methodId, toJniType(args)...));
    } else if constexpr (R == Type::IntType) {
      return jni->CallIntMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::LongType) {
      return jni->CallLongMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::ShortType) {
      return jni->CallShortMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::CharType) {
      return jni->CallCharMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::ByteType) {
      return jni->CallByteMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::FloatType) {
      return jni->CallFloatMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::DoubleType) {
      return jni->CallDoubleMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::ObjectType) {
      return jni->CallObjectMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::BoolType) {
      return jni->CallBooleanMethod(object, methodId, toJniType(args)...);
    } else if constexpr (R == Type::VoidType) {
      jni->CallVoidMethod(object, methodId, toJniType(args)...);
      return false;
    } else {
      throw JniError("Unknown return type");
    }
  }

  static void test() {
//  int i = Jni::invokeVirtual("method", [] { return "()I"sv; });
//  int i = Jni::invokeVirtual("method", typestring_is("()I"){});
    std::string a = invokeVirtual(nullptr, "method", jnisig("(IIJLjava/lang/String;I)Ljava/lang/String;"), 1, 1, 3l, ""s, 1);
//  int f = Jni::invokeVirtual("method", [] { return "()I"sv; }, 13);
  }
};

//static void test() {
//  using sig = decltype(parseSignature(typestring_is("()I")()));
//  static_assert(sig::retType == Type::IntType);
//  static_assert(sig::argc == 0);
//
//  using sig2 = decltype(parseSignature(typestring_is("()Ljava/lang/String;")()));
//  static_assert(sig2::retType == Type::StringType);
//  static_assert(sig2::argc == 0);
//
//  using sig3 = decltype(parseSignature(typestring_is("(II)J")()));
//  static_assert(sig3::retType == Type::LongType);
//  static_assert(sig3::argc == 2);
//
//  using sig4 = decltype(parseSignature(typestring_is("(ILjava/lang/String;ILcom/Bar;Z)J")()));
//  static_assert(sig4::retType == Type::LongType);
//  static_assert(sig4::argc == 5);
//  static_assert(sig4::args[3] == Type::ObjectType);
//
//  int i = Jni::invokeVirtual<typestring_is("()I")>();
//  std::string a = Jni::invokeVirtual<typestring_is("()Ljava/lang/String;")>();
//}

