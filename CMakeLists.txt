cmake_minimum_required(VERSION 3.15)
project(nostd VERSION 0.0.1 DESCRIPTION "nostd c++ library")

set(CXX clang++)
set(CMAKE_CXX_STANDARD 20)

add_library(nostd INTERFACE)

target_include_directories(nostd INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)