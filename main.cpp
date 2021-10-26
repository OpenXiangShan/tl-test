#include <iostream>
#include "TLAgent/Port.h"
#include "Emu/Emu.h"

int main(int argc, char **argv) {
    Emu emu(argc, argv);
    emu.reset(10);
    emu.execute(1000);
    tl_agent::Port<void, void, void, 64> p;
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
