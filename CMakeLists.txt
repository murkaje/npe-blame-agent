cmake_minimum_required(VERSION 3.8)
project(npe-blame-agent)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

# Set c++ standard
set(CMAKE_CXX_STANDARD 17)
#if (MSVC)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
#endif (MSVC)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project configuration
file(GLOB_RECURSE SOURCES src/main/cpp/*.cpp)
include_directories(src/main/include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "npeblame")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "")

# Compiler flags
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_compile_options(${PROJECT_NAME} PRIVATE -pthread -m64 -D_LP64=1 -Wall -Wno-logical-op-parentheses)
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -m64 -fPIC -D_LP64=1 -Wall -Wno-logical-op-parentheses")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  add_compile_options(${PROJECT_NAME} PRIVATE -pthread -m64 -D_LP64=1 -Wall -Wno-parentheses)
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -m64 -fPIC -D_LP64=1 -Wall -Wno-parentheses")
endif ()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JDK library includes
if (NOT EXISTS $ENV{JAVA_HOME}/include/jvmti.h)
  message(FATAL_ERROR "$JAVA_HOME/include/jvmti.h not found, ensure the enironment variable is pointing to a valid JDK install")
else ()
  message("Using JAVA_HOME=$ENV{JAVA_HOME}")
endif ()

if (UNIX AND NOT APPLE)
  include_directories($ENV{JAVA_HOME}/include/linux)
elseif (APPLE)
  include_directories($ENV{JAVA_HOME}/include/darwin)
elseif (WIN32)
  include_directories($ENV{JAVA_HOME}/include/win32)
endif ()
include_directories($ENV{JAVA_HOME}/include)

# 3rd party libs and git submodules
set(SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include/spdlog)
set(BACKWARD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/backward-cpp)
if ((NOT EXISTS ${SPDLOG_INCLUDE_DIR}) OR (NOT EXISTS ${BACKWARD_INCLUDE_DIR}))
  message("Init git submodule dependencies")
  execute_process(COMMAND git submodule update --init)
endif ()
include_directories(${SPDLOG_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/external)

if (NOT MSVC)
  include_directories(${BACKWARD_INCLUDE_DIR})
endif ()

# Optional dependency on libdw for backward-cpp - shows stack traces with source snippets
if (CMAKE_BUILD_TYPE MATCHES Debug)
  find_library(LIB_DW NAMES dw)
  if (NOT LIB_DW)
    message("Optional dependency libdw (package libelf) not found")
  else ()
    message("Found libdw as optional library for backward-cpp")
    target_compile_definitions(${PROJECT_NAME} PRIVATE BACKWARD_HAS_DW=1)
    target_link_libraries(${PROJECT_NAME} -ldw)
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBACKWARD_HAS_DW=1")
  endif ()
endif()

target_link_libraries(${PROJECT_NAME})
