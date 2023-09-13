add_library(Emu Emu.h Emu.cpp)

if(DEFINED TRACE)
    set(ARG_TRACE TRACE)
endif()
if(DEFINED THREAD)
    set(ARG_THREAD ${THREAD})
else()
    set(ARG_THREAD 1)
endif()

verilate(Emu SOURCES "${DUT_DIR}/TestTop.v" INCLUDE_DIRS "${DUT_DIR}" ${ARG_TRACE} THREADS ${ARG_THREAD} VERILATOR_ARGS -Wno-WIDTH -DSIM_TOP_MODULE_NAME=TestTop)
