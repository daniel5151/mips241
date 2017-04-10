#include "bus.h"
#include "ram.h"

#include <cstdint>
#include <iostream>

MIPS::BUS::BUS(MIPS::RAM & ram) : ram(ram), outputBuffer("") {}
MIPS::BUS::~BUS() {}

uint32_t MIPS::BUS::load(uint32_t addr) {
  // Check if we are using any memory mapped I/O
  if (addr == 0xFFFF0004) {
    if (inputBuffer.empty()) {
      std::string input;
      getline(std::cin, input);
      input += "\n";
      for (int i = 0; i < input.length(); i++)
        inputBuffer.push_back(input[i]);
    }
    char in = inputBuffer.front();
    inputBuffer.pop_front();
    return in;
  } else {
    return ram.load(addr);
  }
}

void MIPS::BUS::store(uint32_t addr, uint32_t data) {
  // Check if we are using any memory mapped I/O
  if (addr == 0xFFFF000C) {
    outputBuffer = std::string(1, (char)data);
  } else {
    ram.store(addr, data);
  }
}

std::string MIPS::BUS::getOutput() {
  std::string out = outputBuffer;
  outputBuffer = ""; // clear internal buffer
  return out;
}
