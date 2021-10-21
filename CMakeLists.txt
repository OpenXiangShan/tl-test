cmake_minimum_required(VERSION 3.20)
project(tlc_test)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(TLAgent)
add_executable(tlc_test main.cpp TLAgent)
