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
    find_package (SQLite3)
    include_directories(${SQLite3_INCLUDE_DIRS})
    add_subdirectory(ChiselDB)
endif()

find_package(verilator)

add_subdirectory(Emu)
add_subdirectory(TLAgent)
add_subdirectory(Fuzzer)

add_executable(tlc_test main.cpp)
if(DEFINED THREAD AND TLLOG)
    target_link_libraries(tlc_test Emu TLAgent Fuzzer ChiselDB ${SQLite3_LIBRARIES} Threads::Threads)
elseif(DEFINED THREAD)
    target_link_libraries(tlc_test Emu TLAgent Fuzzer Threads::Threads)
elseif(DEFINED TLLOG)
    target_link_libraries(tlc_test Emu TLAgent Fuzzer ChiselDB ${SQLite3_LIBRARIES})
else()
    target_link_libraries(tlc_test Emu TLAgent Fuzzer)
endif()
