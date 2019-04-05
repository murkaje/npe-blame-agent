#pragma once

#include <exception>
#include <string_view>
#include <stdexcept>

#ifndef _MSC_VER
#include <backward.hpp>
#endif

class ExceptionBase : public std::runtime_error {

  static std::string getTraceString(std::string_view message) {
#ifndef _MSC_VER
    backward::StackTrace stackTrace;
    stackTrace.load_here();
    stackTrace.skip_n_firsts(5);

    std::ostringstream oss;

    oss << message << "\n";

    backward::Printer printer;
    printer.color_mode = backward::ColorMode::always;
    printer.address = true;
    printer.print(stackTrace, oss);

    return oss.str();
#else
    return std::string(message);
#endif
  }

public:

  explicit ExceptionBase(std::string_view message) : runtime_error(getTraceString(message)) {}

};

class JvmtiError : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};

class JniError : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};

class InvalidArgument : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};