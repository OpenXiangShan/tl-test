#include <iostream>
#include <csignal>
#include "Emu/Emu.h"

int main(int argc, char **argv) {
    std::signal(SIGABRT, abortHandler);

    Emu emu(argc, argv);
    emu.reset(10);
    emu.execute(emu.exe_cycles);
    std::cout << std::endl << "Finished" << std::endl;
    return 0;
}
