cmake_minimum_required(VERSION 3.16)
project(tlc_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

if(NOT DEFINED DUT_DIR)
    message(FATAL_ERROR "DUT_DIR is not set!")
endif()

if(DEFINED THREAD)
    add_definitions(-DVL_THREADED)
    find_package(Threads)
endif()

find_package(verilator)

add_subdirectory(Emu)
add_subdirectory(TLAgent)
add_subdirectory(Fuzzer)

add_executable(tlc_test main.cpp)
if(DEFINED THREAD)
    target_link_libraries(tlc_test Emu TLAgent Fuzzer Threads::Threads)
else()
    target_link_libraries(tlc_test Emu TLAgent Fuzzer)
endif()
