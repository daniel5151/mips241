#ifndef DEBUG_H_
#define DEBUG_H_

#include "bus.h"
#include "mem.h"
#include "cpu.h"

#include <map>
#include <set>

using namespace std;

namespace MIPS {
  class Debugger {
  private:
    CPU & cpu;
    RAM & ram;
    BUS & bus;

    map<uint32_t, char> higlight; // What to higlight in RAM

    bool cycleStep; // If we want to step cycle-by-cycle
    bool step;      // If we want to step instr-by-instr

    set<uint32_t> breakpoints; // Which addresses to break on
    set<uint32_t> watch;       // Which addresses to watch

    string prevInput; // Previous REPL input

    void print_CPU();
    void print_stackRAM();
    void print_progRAM();

    void debugPrint();

  public:
    Debugger(CPU & cpu, RAM & ram, BUS & bus);
    ~Debugger();

    void printCPUState();
    void printRAMFrom(uint32_t addr, int n);
    void debugREPL();
  };
}

#endif