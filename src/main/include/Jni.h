#pragma once

#include <string>
#include <utility>
#include <spdlog.h>
#include <fmt/fmt.h>
#include "jni.h"
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
//
//template<Type... Args>
//struct ParamsPack {
//  static constexpr Type args[sizeof...(Args)] = {Args...};
//  static constexpr size_t argc = sizeof...(Args);
//};
//
//template<Type RetType, Type... Args>
//struct ComputedTypesPack : ParamsPack<Args...> {
//  static constexpr Type retType = RetType;
//};
//
//Map terminal types to enum for later processing
//template<char... ObjType>
//constexpr auto typeOf(typestring<ObjType...>) {
//  if(std::is_same_v<typestring_is("Ljava/lang/String;"), typestring<ObjType...>>) {
//    return Type::StringType;
//  } else {
//    return Type::ObjectType;
//  }
//}
//

constexpr auto typeOf(string_view typeString) {
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
//    THROW("ObjectType")
    return Type::ObjectType;
  }
}

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
      assert(false);
    case ')':
      THROW("typeChar is ')'")
      assert(false);
    case 'L':
      THROW("typeChar is 'L'")
      assert(false);
    default:
      THROW("typeChar is something")
      assert(false);
  }
}
//
//// Terminal patterns, read return type and finish
//template<Type... Args, char... RetString>
//constexpr auto parseSignature(ParamsPack<Args...>, typestring<')'>, typestring<RetString>...) {
//  return ComputedTypesPack<typeOf(typestring<RetString...>()), Args...>();
//}
//
//template<Type... Args, char RetChar>
//constexpr auto parseSignature(ParamsPack<Args...>, typestring<')'>, typestring<RetChar>) {
//  return ComputedTypesPack<typeOf(RetChar), Args...>();
//}
//
//// State functions for constructing object type signature "Lfoo/Bar;" to a single typestring that we can match
//template<Type... Args, char... Head, char... Tail>
//constexpr auto parseSignatureObjState(ParamsPack<Args...>, typestring<Head...>, typestring<';'>, typestring<Tail>...) {
//  return parseSignature(ParamsPack<Args..., typeOf(typestring<Head..., ';'>())>(), typestring<Tail>()...);
//}
//
//template<Type... Args, char... Head, char Current, char... Tail>
//constexpr auto parseSignatureObjState(ParamsPack<Args...>, typestring<Head...>, typestring<Current>, typestring<Tail>...) {
//  return parseSignatureObjState(ParamsPack<Args...>(), typestring<Head..., Current>(), typestring<Tail>()...);
//}
//
//template<Type... Args, char... Tail>
//constexpr auto parseSignature(ParamsPack<Args...>, typestring<'L'>, typestring<Tail>...) {
//  return parseSignatureObjState(ParamsPack<Args...>(), typestring<'L'>(), typestring<Tail>()...);
//}
//
//// Single char param type parsing
//template<Type... Args, char TypeChar, std::enable_if_t<TypeChar != 'L', int> = 0, char... Tail>
//constexpr auto parseSignature(ParamsPack<Args...>, typestring<TypeChar>, typestring<Tail>...) {
//  return parseSignature(ParamsPack<Args..., typeOf(TypeChar)>(), typestring<Tail>()...);
//}
//
//// Split single typestring to its char elements and init empty param pack
//template<char... Tail>
//constexpr auto parseSignature(typestring<'(', Tail...>) {
//  return parseSignature(ParamsPack<>(), typestring<Tail>()...);
//}

// Version3
// We have the passed types, we could iterate over those and check against signature
// instead of heavy template matching, use constexpr functions, figure out how
// string_view is compile-time usable?

template<typename T>
struct TypeHolder {
  using type = T;
};

template<Type t>
constexpr auto fromEnum() {
  if constexpr(t == Type::IntType) {
    return int{};
  } else if constexpr (t == Type::StringType) {
    return std::string{};
  } else {
    assert(false);
  }
}

template<bool matches = false>
struct UnexpectedType {
  void error(string_view message) {
  }

  constexpr UnexpectedType(string_view message, size_t idx) {
//    if(idx == 0) {
//      THROW("Error at parameter index 0")
//    } else if(idx == 1) {
//      THROW("Error at parameter index 1")
//    } else if(idx == 2) {
//      THROW("Error at parameter index 2")
//    } else if(idx == 3) {
//      THROW("Error at parameter index 3")
//    }
//    assert(matches);
    error("");
  }
};

template<typename DeclaredType, typename SignatureChecker>
struct TypeChecker {

  constexpr static void check(SignatureChecker &context) {
    size_t idx = context.nextIndex();
    Type t = context.types[idx];

    if (t == Type::IntType) {
      constexpr bool matches = std::is_same_v<int, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int at parameter index=", idx);
    } else if (t == Type::StringType) {
      constexpr bool matches = std::is_same_v<std::string, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected std::string at parameter index=", idx);
    } else if (t == Type::LongType) {
      constexpr bool matches = std::is_same_v<int64_t, DeclaredType>;
      if constexpr (!matches)
        UnexpectedType("Expected int64 at parameter index=", idx);
    } else {
      context.onError("Type not implemented yet");
    }

//    using ExpectedType = decltype(fromEnum(t));
//    static_assert(std::is_same_v<ExpectedType, DeclaredType>);
  }

//  template<Type T, typename >
//  static void checkInternal(SignatureChecker &context) {
//    static_assert(std::is_same_v<ExpectedType, DeclaredType>);
//  }
};

using fmt::literals::operator ""_format;

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
          onError("Signature specifies more arguments than passed to function");
        }
        continue;
      }

      if (index == argc + 1) {
        onError("Signature specifies more types than passed to function");
      }

      if (parsingObject) {
        if (typeChar == ';') {
          Type type = typeOf(string_view(start, it - start));
          if (type == Type::InvalidType) {
            onError("Unexpected character in signature");
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
      onError("Reached end of signature while reading object type from signature. Missing ';' somewhere?");
    }

    for (auto checkFun : checkers) {
      checkFun(*this);
    }
  }

  template<auto... T>
  void onError(const char *message) {
    do {
      static_cast<void>(sizeof(message));
      assert(false);
    } while (false);
  }

  constexpr size_t nextIndex() {
    if (curIndex == argc) {
      onError("Signature specifies more types than passed to function");
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

template<typename... ArgType>
constexpr bool checkSignature(string_view signature) {
  SignatureChecker<ArgType...>{signature};
  return true;
}

template<typename... ArgType>
constexpr bool checkSignature(SignatureChecker<ArgType...> checker) {
  return true;
}

using std::string_view_literals::operator ""sv;
using std::string_literals::operator ""s;
#define jnisig(x) typestring_is(x){}

class Jni {
  inline static JNIEnv *jni = nullptr;
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

  template<>
  static auto toNativeType<Type::StringType>(jstring arg) {
    return jstringToString(jni, arg);
  }

public:

  static void init(JNIEnv *jniEnv) {
    jni = jniEnv;
  }

  template<char... Signature, typename... ArgType>
  static auto invokeVirtual(jobject object, string_view methodName, typestring<Signature...> signature, ArgType... args) {
    constexpr SignatureChecker<ArgType...> checker{signature.data()};

    jclass klass = jni->GetObjectClass(object);
    checkJniException(jni);
    jmethodID methodId = jni->GetMethodID(klass, methodName.data(), signature.data());
    checkJniException(jni);

    constexpr auto R = checker.getRetType();
    if constexpr (R == Type::StringType) {
      auto retVal = (jstring) jni->CallObjectMethod(object, methodId, toJniType(args)...);
      return jstringToString(jni, retVal);
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

