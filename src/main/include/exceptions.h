#pragma once

#include "backward.hpp"
#include <exception>
#include <string_view>
#include <stdexcept>

class ExceptionBase : public std::runtime_error {
  backward::StackTrace stackTrace;
  std::string printedTrace;
public:

  explicit ExceptionBase(std::string_view message) : runtime_error(std::string(message)) {
    stackTrace.load_here();
    stackTrace.skip_n_firsts(4);

    std::ostringstream oss;

    oss << runtime_error::what() << "\n";

    backward::Printer printer;
    printer.color_mode = backward::ColorMode::always;
    printer.address = true;
    printer.print(stackTrace, oss);

    printedTrace = oss.str();
  }

private:
  const char *what() const noexcept override {
    return printedTrace.c_str();
  }

};

class JvmtiError : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};

class JniError : public ExceptionBase {
  using ExceptionBase::ExceptionBase;
};