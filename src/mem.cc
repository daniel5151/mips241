#include "mem.h"

#include "disasm.h"

#include <cstdint>

#include <sstream>
#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::hex << x ) ).str()

MIPS::RAM::RAM () {}
MIPS::RAM::~RAM () {
  for (auto it = mem.begin(); it != mem.end(); ++it) {
    delete [] it->second;
  }
}

void MIPS::RAM::store(uint32_t memaddr, uint32_t word) {
  if (memaddr % 4 != 0) {
    throw "Unaligned Access - Cannot Store to 0x" + SSTR(memaddr);
    return;
  }

  uint16_t page   = (memaddr >> 16) & 0xFFFF;
  uint16_t offset =  memaddr        & 0xFFFF;

  if (mem.count(page) == 0) {
    mem[page] = new uint32_t [(0xFFFF + 1) / 4](); // word addressable
  }

  mem[page][offset / 4] = word;
};

uint32_t MIPS::RAM::load(uint32_t memaddr) {
  if (memaddr % 4 != 0) {
    throw "Unaligned Access - Cannot Load from 0x" + SSTR(memaddr);
    return -1;
  }

  uint16_t page   = (memaddr >> 16) & 0xFFFF;
  uint16_t offset =  memaddr        & 0xFFFF;

  if (mem.count(page) == 0) {
    mem[page] = new uint32_t [(0xFFFF + 1) / 4](); // word addressable
  }

  return mem[page][offset / 4];
};