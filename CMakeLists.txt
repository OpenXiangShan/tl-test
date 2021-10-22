cmake_minimum_required(VERSION 3.16)
project(tlc_test)

set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED DUT_DIR)
    message(FATAL_ERROR "DUT_DIR is not set!")
endif()

find_package(verilator)

add_subdirectory(Emu)
add_subdirectory(TLAgent)

add_executable(tlc_test main.cpp)
target_link_libraries(tlc_test Emu TLAgent)
