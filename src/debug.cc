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

using namespace std;
using namespace MIPS;

template <typename T>
string toHex(T const& x) {
  ostringstream out;
  out << "0x" << std::setfill('0') << std::setw(8) << std::hex << std::right << x;
  return out.str();
}

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
      << toHex(addr) 
      << " | "
      << toHex(ram.load(addr))
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
      << " = " << toHex(cpu.getRegister(i));
  }

  cerr <<  endl;
  cerr 
    << " PC = " << toHex(cpu.getiRegister("PC"))
    << "   "
    << " IR = " << toHex(cpu.getiRegister("IR"))
    << endl 
    << endl;

  cerr 
    << " lo = " << toHex(cpu.getiRegister("hi")) << "   "
    << " hi = " << toHex(cpu.getiRegister("lo"))
    << endl 
    << endl;

  cerr 
    << " RA = " << toHex(cpu.getiRegister("RA")) << "   "
    << " RB = " << toHex(cpu.getiRegister("RB")) 
    << endl;
  cerr 
    << " RZ = " << toHex(cpu.getiRegister("RZ"))
    << endl;
  cerr 
    << " RM = " << toHex(cpu.getiRegister("RM"))
    << endl;
  cerr 
    << " RY = " << toHex(cpu.getiRegister("RY"))
    << endl;

  cerr << "Stage: " << cpu.getStage() << endl;

  cerr
    << "Cycle no. " 
    << dec << cpu.getCycle() 
    << endl;

}

void Debugger::print_stackRAM() {
  bool oldHiglight_existed;
  char oldHiglight;

  cerr << "  ----------====== Stack RAM ======---------" << endl;
  uint32_t stackPtr = cpu.getRegister(30);

  oldHiglight_existed = (higlight.count(stackPtr) != 0);
  oldHiglight = (oldHiglight_existed) ? higlight[stackPtr] : ' ';

  higlight[stackPtr] = '>';
  printRAMFrom(stackPtr - 0x10, 12);

  if (oldHiglight_existed) higlight[stackPtr] = oldHiglight;
  else higlight.erase(stackPtr);
}

void Debugger::print_progRAM() {
  bool oldHiglight_existed;
  char oldHiglight;

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
}

void Debugger::print_CPU() {
  cerr << "-------------------------====== CPU State ======-------------------------";
  printCPUState();
}

void Debugger::debugPrint() {
  // "clear" screen
  cerr << string(100, '\n');

  print_stackRAM();
  print_progRAM();
  print_CPU();

  if (!watch.empty()) {
    cerr
      << endl
      << "     ADDR    |   HEXVAL   " << endl
      << "  -----------|------------" << endl;
    for (auto it = watch.begin(); it != watch.end(); it++) {
      cerr 
        << "  "
        << toHex(*it) 
        << " | "
        << toHex(ram.load(*it))
        << endl
        << endl;

    }
  }
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

  debugPrint();

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
      else if (tok == "peek") {
        uint32_t memaddr;
        in_ss >> hex >> memaddr;
        cerr << toHex(bus.load(memaddr)) << endl;
      }
      else if (tok == "poke") {
        uint32_t memaddr;
        uint32_t new_val;
        in_ss 
          >> hex >> memaddr
          >> hex >> new_val;

        bus.store(memaddr, new_val);
      } 
      else if (tok == "+watch") {
        uint32_t memaddr;
        in_ss >> hex >> memaddr;
        watch.insert(memaddr);
      }
      else if (tok == "-watch") {
        uint32_t memaddr;
        in_ss >> hex >> memaddr;
        watch.erase(memaddr);
      }
      else if (tok == "print") {
        debugPrint();
      }
      else if (tok == "help") {
        cerr
          << "  step              - exec one instruction"         << endl
          << "  run               - resume regular execution"     << endl
          << "  +bp <addr>        - add breakpoint at <addr>"     << endl
          << "  -bp <addr>        - remove breakpoint at <addr>"  << endl
          << "  peek <addr>       - print out contents of <addr>" << endl
          << "  poke <addr> <val> - edit contents of <addr>"      << endl
          << "  +watch <addr>     - keep an eye on <addr>"        << endl
          << "  -watch <addr>     - don't keep an eye on <addr>"  << endl
          << "  print             - dislpay debug data"           << endl;
      }

      else {
        valid = false;
        break;

      }
    }
    if (!valid) cerr << "Unknown command." << endl;
  }
}
