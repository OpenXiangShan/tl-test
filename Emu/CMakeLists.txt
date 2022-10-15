add_library(Emu Emu.h Emu.cpp)

if(DEFINED TRACE)
    set(ARG_TRACE TRACE)
endif()
if(DEFINED THREAD)
    set(ARG_THREAD ${THREAD})
else()
    set(ARG_THREAD 1)
endif()

verilate(Emu SOURCES "${DUT_DIR}/TestTop.v" "${DUT_DIR}/STD_CLKGT_func.v" ${ARG_TRACE} THREADS ${ARG_THREAD})
