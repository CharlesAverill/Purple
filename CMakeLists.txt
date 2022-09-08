cmake_minimum_required(VERSION 3.13)
project(purple LANGUAGES C VERSION "0.1.0")

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE Debug)

string(CONCAT PROJECT_NAME_AND_VERS ${CMAKE_PROJECT_NAME} " " ${CMAKE_PROJECT_VERSION})

file (GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.c")

add_compile_options(-D_FILE_OFFSET_BITS=64)

configure_file(include/info.h.in info.h @ONLY)
include_directories(${CMAKE_SOURCE_DIR}/build)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SRC_FILES})
