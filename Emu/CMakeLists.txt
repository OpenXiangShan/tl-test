if(DEFINED CHISELDB)
    add_library(Emu Emu.h Emu.cpp ${DUT_DIR}/chisel_db.cpp ${DUT_DIR}/chisel_db.h)
else()
    add_library(Emu Emu.h Emu.cpp)
endif()

if(DEFINED TRACE)
    set(ARG_TRACE TRACE)
endif()
if(DEFINED THREAD)
    set(ARG_THREAD ${THREAD})
else()
    set(ARG_THREAD 1)
endif()

file(GLOB_RECURSE VSRCS "${DUT_DIR}/*.v")
message(STATUS "vsrc: ${VSRCS}")

# TestTop.v must be the first in SOURCES
set(VTOP "${DUT_DIR}/TestTop.v")
list(FILTER VSRCS EXCLUDE REGEX ${VTOP})

# if ChiselDB is found at DUT_DIR
# then we need to enable ChiselDB at tl-test
file(GLOB DBWRITER "${DUT_DIR}/*Writer.v")
if(DBWRITER AND NOT DEFINED CHISELDB)
    message(FATAL_ERROR "ChiselDB found! requires -DCHISELDB=1")
endif()

if(DEFINED CHISELDB)
    verilate(Emu SOURCES ${VTOP} ${VSRCS} ${ARG_TRACE} THREADS ${ARG_THREAD}
     VERILATOR_ARGS -Wno-Width -I${DUT_DIR} -DENABLE_CHISEL_DB)
else()
    verilate(Emu SOURCES ${VTOP} ${VSRCS} ${ARG_TRACE} THREADS ${ARG_THREAD} VERILATOR_ARGS -Wno-Width)
endif()