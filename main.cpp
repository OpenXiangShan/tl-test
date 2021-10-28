#include <iostream>
#include "Emu/Emu.h"

int main(int argc, char **argv) {
    Emu emu(argc, argv);
    emu.reset(10);
    emu.execute(1000);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
