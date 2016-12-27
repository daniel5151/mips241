#ifndef MEM_H_
#define MEM_H_

#include <cstdint>
#include <map>

namespace MIPS {
  class RAM {
    std::map<uint16_t,uint32_t*> mem;
  public:
    RAM();
    ~RAM();

    void     store(uint32_t memaddr, uint32_t word);
    uint32_t load(uint32_t memaddr);

    void printFrom(uint32_t addr, int n, std::ostream & out, uint32_t higlight = 0x1);
  };
}

#endif