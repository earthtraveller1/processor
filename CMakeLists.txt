cmake_minimum_required(VERSION 3.13)

project(processor)

set(CMAKE_CXX_STANDARD 20)

option(PROCESSOR_BUILD_TESTS "Whether or not to build tests" OFF)

add_executable(processor)

if (PROCESSOR_BUILD_TESTS)
    target_compile_definitions(processor PRIVATE PROCESSOR_BUILD_TESTS)
endif()

add_subdirectory(src)
