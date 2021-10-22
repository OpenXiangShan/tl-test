add_library(Emu Emu.h Emu.cpp)
verilate(Emu SOURCES "${DUT_DIR}/TestTop.v" "${DUT_DIR}/plusarg_reader.v")
