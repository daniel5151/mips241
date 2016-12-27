#ifndef DEBUG_H_
#define DEBUG_H_

#include "bus.h"
#include "mem.h"
#include "cpu.h"

namespace MIPS {
  class Debugger {
  private:
    CPU & cpu;
    RAM & ram;
    BUS & bus;
  public:
    Debugger(CPU & cpu, RAM & ram, BUS & bus);
    ~Debugger();

    void debug();
  };
}

#endif