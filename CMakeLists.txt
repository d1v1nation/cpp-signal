cmake_minimum_required(VERSION 3.6)
project(signal)

set(CMAKE_CXX_COMPILER g++-7)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCE_FILES main.cpp signal.h small_cont.h)
add_executable(signal ${SOURCE_FILES})