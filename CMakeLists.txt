cmake_minimum_required(VERSION 3.8)
project(exception_info)

set(CMAKE_CXX_COMPILER /usr/bin/clang++-5.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -m64 -fPIC -D_LP64=1 -Wall")

include_directories($ENV{JAVA_HOME}/include)
include_directories($ENV{JAVA_HOME}/include/linux)
include_directories(src/main/include)

file(GLOB SOURCES src/main/c/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "exceptioninfo")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "64")