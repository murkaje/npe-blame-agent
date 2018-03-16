#pragma once

//#include <iostream>
//#include <iomanip>
//#include <string>
//
//#define LOGLEVEL_FATAL 0
//#define LOGLEVEL_ERROR 1
//#define LOGLEVEL_WARN  2
//#define LOGLEVEL_INFO  3
//#define LOGLEVEL_DEBUG 4
//
//class Logger {
//private:
//
//  std::string id;
//
//  std::ostream& print(const std::string &level);
//
//  void print(const std::string &level, const std::string &message);
//
//public:
//  Logger(const std::string &loggerId);
//
//  std::ostream& error();
//  std::ostream& info();
//  std::ostream& debug();
//
//  void error(const std::string &message);
//  void info(const std::string &message);
//  void debug(const std::string &message);
//};