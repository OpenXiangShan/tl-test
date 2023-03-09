add_library(Emu Emu.h Emu.cpp)

if(DEFINED TRACE)
    set(ARG_TRACE TRACE)
endif()
if(DEFINED THREAD)
    set(ARG_THREAD ${THREAD})
else()
    set(ARG_THREAD 1)
endif()

verilate(Emu SOURCES "${DUT_DIR}/env/tb_top.v" 
    INCLUDE_DIRS "${DUT_DIR}/env" "${DUT_DIR}/sram" "${DUT_DIR}/rtl" "${DUT_DIR}/model/axi_mem"
    ${ARG_TRACE} THREADS ${ARG_THREAD}
    TRACE_THREADS
    VERILATOR_ARGS --assert --trace-fst --no-timing -Wno-TIMESCALEMOD)
    # VERILATOR_ARGS --coverage-line