cmake_minimum_required(VERSION 3.12)

project(minilisp)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(third_party/libedit)

include_directories(${edit_includes})
add_executable(minilisp minilisp.c)

add_library(libml minilisp.c)
target_compile_definitions(libml PRIVATE -DLIBRARY)

enable_testing()
add_test(minilisp-test "${PROJECT_SOURCE_DIR}/test.sh" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
