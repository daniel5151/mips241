/* Where the real magic happens */

#include "bus.h"
#include "cpu.h"
#include "debug.h"
#include "ram.h"

#include "../_disasm/disasm.h"

#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(mips241) {
  emscripten::class_<MIPS::RAM>("MIPS::RAM")
    .constructor()
    .function("load", &MIPS::RAM::load)
    .function("store", &MIPS::RAM::store)
    ;

  emscripten::class_<MIPS::BUS>("MIPS::BUS")
    .constructor<MIPS::RAM&>()
    .function("load", &MIPS::BUS::load)
    .function("store", &MIPS::BUS::store)
    .function("getOutput", &MIPS::BUS::getOutput)
    ;

  emscripten::class_<MIPS::CPU>("MIPS::CPU")
    .constructor<MIPS::BUS&>()
    .function("stillExecuting", &MIPS::CPU::stillExecuting)
    .function("getCycle",       &MIPS::CPU::getCycle)
    .function("getStage",       &MIPS::CPU::getStage)
    .function("getRegister",    &MIPS::CPU::getRegister)
    .function("getiRegister",   &MIPS::CPU::getiRegister)
    .function("getPC",          &MIPS::CPU::getPC)
    .function("setRegister",    &MIPS::CPU::setRegister)
    .function("do_cycle",       &MIPS::CPU::do_cycle)
    ;

  emscripten::class_<MIPS::Debugger>("MIPS::Debugger")
    .constructor<MIPS::CPU&, MIPS::RAM&, MIPS::BUS&>()
    .function("printCPUState", &MIPS::Debugger::printCPUState)
    .function("printRAMFrom", &MIPS::Debugger::printRAMFrom)
    .function("addhiglight", &MIPS::Debugger::addhiglight)
    .function("removehiglight", &MIPS::Debugger::removehiglight)
    ;

  emscripten::function("MIPS$$disasm", MIPS::disasm);
}