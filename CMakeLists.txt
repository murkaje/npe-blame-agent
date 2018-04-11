cmake_minimum_required(VERSION 3.8)
project(npe-blame-agent)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -m64 -fPIC -D_LP64=1 -Wall -Wno-logical-op-parentheses")

if (UNIX AND NOT APPLE)
  include_directories($ENV{JAVA_HOME}/include/linux)
elseif (APPLE)
  include_directories($ENV{JAVA_HOME}/include/darwin)
elseif (WIN32)
  include_directories($ENV{JAVA_HOME}/include/win32)
endif ()

include_directories($ENV{JAVA_HOME}/include)
include_directories(src/main/include)

set(SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include/spdlog CACHE PATH "spdlog include directory")
find_path(SPDLOG_INCLUDE_DIR ...)
if ((NOT SPDLOG_INCLUDE_DIR) OR (NOT EXISTS ${SPDLOG_INCLUDE_DIR}))
  message("Init spdlog submodule")
  execute_process(COMMAND git submodule update --init)
endif ()
include_directories(${SPDLOG_INCLUDE_DIR})

file(GLOB SOURCES src/main/cpp/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} "-lstdc++")

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "npeblame")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")