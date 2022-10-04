cmake_minimum_required(VERSION 3.13)
project(purple LANGUAGES C VERSION "0.1.0")

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE Debug)

find_package(Clang)
if(${Clang_FOUND})
    message("Found clang")
else (${Clang_FOUND})
    message(FATAL_ERROR "Could not locate some clang files, consider installing libclang-xx-dev")
endif(${Clang_FOUND})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_CMAKE_DIR}")

string(CONCAT PROJECT_NAME_AND_VERS ${CMAKE_PROJECT_NAME} " " ${CMAKE_PROJECT_VERSION})
set(DEFAULT_CLANG_EXECUTABLE_PATH ${Clang_DIR})
message(${DEFAULT_CLANG_EXECUTABLE_PATH})

file (GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.c")

add_compile_options(-D_FILE_OFFSET_BITS=64 -pedantic-errors)

configure_file(include/info.h.in info.h @ONLY)
include_directories(build include)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE m)