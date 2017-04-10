#ifndef BUS_H_
#define BUS_H_

#include <cstdint>
#include <deque>
#include <string>

#include "ram.h"

namespace MIPS {
  class BUS {
  private:
    RAM & ram;

    std::deque<char> inputBuffer;
    std::string outputBuffer;
  public:
    BUS(RAM & ram);
    ~BUS();

    uint32_t load (uint32_t addr);
    void     store(uint32_t addr, uint32_t data);

    std::string getOutput();
  };
}

#endif
