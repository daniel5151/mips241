#include "bus.h"
#include "mem.h"

#include <cstdint>
#include <iostream>

MIPS::BUS::BUS(MIPS::RAM & mem) : mem(mem) {}
MIPS::BUS::~BUS() {}

uint32_t MIPS::BUS::load(uint32_t addr) {
  // Check if we are using any memory mapped I/O
  if (addr == 0xFFFF0004) {
    return getchar();
  } else {
    return mem.load(addr);
  }
}

void MIPS::BUS::store(uint32_t addr, uint32_t data) {
  // Check if we are using any memory mapped I/O
  if (addr == 0xFFFF000C) {
    std::cout << ((char)data);
  } else {
    mem.store(addr, data);
  }
}
