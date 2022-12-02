cmake_minimum_required(VERSION 3.16)
project(tlc_test)

set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED DUT_DIR)
    message(FATAL_ERROR "DUT_DIR is not set!")
endif()

if(DEFINED THREAD)
    add_definitions(-DVL_THREADED)
    find_package(Threads)
endif()

if(DEFINED TLLOG)
    add_definitions(-DTLLOG)
endif()

find_package(verilator)
find_package (SQLite3)

add_subdirectory(Emu)
add_subdirectory(TLAgent)
add_subdirectory(Fuzzer)
add_subdirectory(ChiselDB)

include_directories(${SQLite3_INCLUDE_DIRS})
add_executable(tlc_test main.cpp)
if(DEFINED THREAD)
    target_link_libraries(tlc_test Emu TLAgent Fuzzer ChiselDB ${SQLite3_LIBRARIES} Threads::Threads)
else()
    target_link_libraries(tlc_test Emu TLAgent Fuzzer ChiselDB ${SQLite3_LIBRARIES})
endif()
