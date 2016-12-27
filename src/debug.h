#ifndef DEBUG_H_
#define DEBUG_H_

#include "bus.h"
#include "mem.h"
#include "cpu.h"

#include "disasm.h"

namespace MIPS {
  class Debugger {
  private:
  public:
    Debugger();
    ~Debugger();
  }
}

#endif