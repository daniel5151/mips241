#ifndef BUS_H_
#define BUS_H_

#include <cstdint>
#include <deque>

#include "mem.h"

namespace MIPS {
  class BUS {
  private:
    RAM & mem;

    std::deque<char> inputBuffer;
  public:
    BUS(RAM & mem);
    ~BUS();

    uint32_t load (uint32_t addr);
    void     store(uint32_t addr, uint32_t data);
  };
}


#endif