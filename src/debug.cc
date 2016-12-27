#include "debug.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstdint>

#include <vector>
#include <map>

#include "bus.h"
#include "mem.h"
#include "cpu.h"

#include "disasm.h"

#define COUTHEX(x) \
  "0x" << std::setfill('0') << std::setw(8) << std::right << std::hex << x << std::dec

using namespace std;
using namespace MIPS;

Debugger::Debugger(CPU & cpu, RAM & ram, BUS & bus)
: cpu(cpu), ram(ram), bus(bus)
{
  prevInput = "step";

  cycleStep = false;

  step = false;

  // Break at the start of the program
  breakpoints.insert(0x00000000);
};

Debugger::~Debugger() {}

void Debugger::printRAMFrom(uint32_t addr, int n) {
  cerr
    << "     ADDR    |   HEXVAL   :     MIPS ASM    " << endl
    << "  -----------|------------------------------" << endl;

  while (n > 0) { 
    cerr 
      << ( (higlight.count(addr) != 0) ? higlight[addr] : ' ') << " "
      << "0x" << setfill('0') << setw(8) << hex << addr 
      << " | "
      << "0x" << setfill('0') << setw(8) << hex << ram.load(addr)
      << " : " 
      << disasm(ram.load(addr)) 
      << endl;

    addr += 4;
    n--;
  }

  cerr << endl;
}

void Debugger::printCPUState() {
  for (int i = 1; i < 32; i++) {
    if (i % 4 - 1 == 0 && i != 0)
      cerr << endl;

    cerr 
      << setfill(' ') << ( (i % 4 - 1 != 0) ? "   " : "" ) << "$" 
      << setw(2) << left << dec << i 
      << " = " << COUTHEX(cpu.getRegister(i));
  }

  cerr <<  endl;
  cerr 
    << " PC = " << COUTHEX(cpu.getiRegister("PC"))
    << "   "
    << " IR = " << COUTHEX(cpu.getiRegister("IR"))
    << endl 
    << endl;

  cerr 
    << " lo = " << COUTHEX(cpu.getiRegister("hi")) << "   "
    << " hi = " << COUTHEX(cpu.getiRegister("lo"))
    << endl 
    << endl;

  cerr 
    << " RA = " << COUTHEX(cpu.getiRegister("RA")) << "   "
    << " RB = " << COUTHEX(cpu.getiRegister("RB")) 
    << endl;
  cerr 
    << " RZ = " << COUTHEX(cpu.getiRegister("RZ"))
    << endl;
  cerr 
    << " RM = " << COUTHEX(cpu.getiRegister("RM"))
    << endl;
  cerr 
    << " RY = " << COUTHEX(cpu.getiRegister("RY"))
    << endl;

  cerr << "Stage: " << cpu.getStage() << endl;
}

void Debugger::debugREPL() {
  // If we are not stepping each cycle, then we might be able
  //  to skip some instructions
  if (!cycleStep) {
    // If we are not in Stage 0, don't debug
    if (cpu.getStage() != 0) return;

    // If we are stepping, or there is a breakpoint, debug
    if (step || breakpoints.count(cpu.getPC()) != 0) {
      // do the debug
    } else return;
  }

  // "clear" screen
  cerr << string(100, '\n');

  bool oldHiglight_existed;
  char oldHiglight;

  // Printout Stack RAM
  cerr << "  ----------====== Stack RAM ======---------" << endl;
  uint32_t stackPtr = cpu.getRegister(30);

  oldHiglight_existed = (higlight.count(stackPtr) != 0);
  oldHiglight = (oldHiglight_existed) ? higlight[stackPtr] : ' ';

  higlight[stackPtr] = '>';
  printRAMFrom(stackPtr - 0x10, 12);

  if (oldHiglight_existed) higlight[stackPtr] = oldHiglight;
  else higlight.erase(stackPtr);

  // Printout program RAM
  cerr << "  ---------====== Program RAM ======--------" << endl;
  uint32_t currentPC = cpu.getPC();

  oldHiglight_existed = (higlight.count(currentPC) != 0);
  oldHiglight = (oldHiglight_existed) ? higlight[currentPC] : ' ';

  higlight[currentPC] = '>';
  uint32_t startAddr = ((int)currentPC - 0x20 >= 0x0) 
    ? currentPC - 0x20
    : 0x0;
  printRAMFrom(startAddr, 16);

  if (oldHiglight_existed) higlight[currentPC] = oldHiglight;
  else higlight.erase(currentPC);

  // Printout CPU state
  cerr << "-------------------------====== CPU State ======-------------------------";
  printCPUState();

  // Printout CPU cycles
  cerr 
    << "Cycle no. " 
    << dec << cpu.getCycle() 
    << endl 
    << endl;

  // the worst REPL you've ever seen
  for (;;) {
    cerr << prevInput << "> "; // prompt

    string input;
    getline(cin, input);

    if (input == "") {
      input = prevInput; // Redo prev input
    }

    prevInput = input; // Save new input in history

    stringstream in_ss (input);

    bool valid = true;

    string tok;
    while (in_ss >> tok) {
           if (tok == "step") { step = true;  return; } 
      else if (tok == "run")  { step = false; return; }
      else if (tok == "+bp") {
        uint32_t bp_addr;
        in_ss >> hex >> bp_addr;
        breakpoints.insert(bp_addr);
        higlight[bp_addr] = '!';
      }
      else if (tok == "-bp") {
        uint32_t bp_addr;
        in_ss >> hex >> bp_addr;
        breakpoints.erase(bp_addr);
           higlight.erase(bp_addr);
      }

      else {
        valid = false;
        break;

      }
    }
    if (!valid) cerr << "Unknown command." << endl;
  }
}
