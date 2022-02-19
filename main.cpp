#include <iostream>
#include "Emu/Emu.h"

int main(int argc, char **argv) {
    Emu emu(argc, argv);
    emu.reset(10);
    emu.execute(100000000);
    std::cout << std::endl << "Finished" << std::endl;
    return 0;
}
