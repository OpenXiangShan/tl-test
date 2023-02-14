#include "Emu/Emu.h"
#include <iostream>

int main(int argc, char **argv) {
  try {
    Emu emu(argc, argv);
    emu.reset(10);
    emu.execute(emu.exe_cycles);
  } catch (...) {
    std::cout << std::endl << "\033[1;31mFailed!\033[0m" << std::endl;
    return 1;
  }
  std::cout << std::endl << "\033[1;32mSuccessfully Finished!\033[0m" << std::endl;
  return 0;
}
