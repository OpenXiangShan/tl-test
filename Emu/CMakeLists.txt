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
if(DEFINED CHISELDB)
    set(ARG_CHISELDB, -DENABLE_CHISEL_DB)
endif()

# if ChiselDB is found at DUT_DIR
# then we need to enable ChiselDB at tl-test
file(GLOB DBWRITER "${DUT_DIR}/*Writer.v")
if(DBWRITER AND NOT DEFINED CHISELDB)
    message(FATAL_ERROR "ChiselDB found! requires -DCHISELDB=1")
endif()

verilate(Emu SOURCES "${DUT_DIR}/TestTop.v" INCLUDE_DIRS "${DUT_DIR}" ${ARG_TRACE} THREADS ${ARG_THREAD} VERILATOR_ARGS -Wno-Width --x-assign unique ${ARG_CHISELDB} -DSIM_TOP_MODULE_NAME=TestTop)
