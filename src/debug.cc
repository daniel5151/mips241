#include "debug.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <vector>

#include "bus.h"
#include "mem.h"
#include "cpu.h"

#include "disasm.h"

using namespace std;

MIPS::Debugger::Debugger(CPU & cpu, RAM & ram, BUS & bus)
: cpu(cpu), ram(ram), bus(bus)
{};

MIPS::Debugger::~Debugger() {};

void MIPS::Debugger::debug() {
  cout << string(100, '\n');

  // Printout program RAM
  uint32_t startAddr = ((int)cpu.getPC() - 0x20 >= 0x0) ? cpu.getPC() - 0x20 : 0x0;
  ram.printFrom(startAddr, 16, cerr, cpu.getPC());

  // Printout Stack RAM
  ram.printFrom(cpu.getRegister(30) - 0x10, 12, cerr, cpu.getRegister(30));

  // Printout CPU state
  cpu.printState(cerr);

  // Printout CPU cycles
  cerr 
    << "Cycle no. " 
    << dec << cpu.getCycle() 
    << endl 
    << endl;

  // REPL

  if (cpu.getCycle() % 5 != 0) return;

  for (;;) {
    cout << "> ";

    string input;
    getline(cin, input);

    if (input == "") return;

    stringstream in_ss (input);
  }
}
