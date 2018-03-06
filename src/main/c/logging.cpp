#include "logging.h"

Logger::Logger(const std::string &loggerId) : id{loggerId} {}

std::ostream& Logger::print(const std::string &level) {
  return std::cerr << id << " [" << level << "] ";
}

void Logger::print(const std::string &level, const std::string &message) {
  std::cerr << id << " [" << level << "] " << message << std::endl;
}

std::ostream& Logger::error() {
  return print("ERROR");
}

std::ostream& Logger::info() {
  return print("INFO");
}

std::ostream& Logger::debug() {
  return print("DEBUG");
}

void Logger::error(const std::string &message) {
  print("ERROR", message);
}

void Logger::info(const std::string &message) {
  print("INFO", message);
}

void Logger::debug(const std::string &message) {
  print("DEBUG", message);
}