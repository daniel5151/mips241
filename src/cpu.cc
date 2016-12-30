#include "cpu.h"
#include "bus.h"

#include "disasm.h"

#include <cstdint>

MIPS::CPU::CPU(MIPS::BUS & bus) : MEM(bus) 
{
  // Internal flags and tracking vars
  isExecuting = true;
  cycles = 0;
  stage = S_FETCH;

  // Setup registers
  PC = 0x00000000;
  hi = lo = 0;
  RA = RB = RZ = RM = RY = 0;
  for (int i = 0; i < 32; i++) R[i] = 0;

  R[31] = 0x8123456C; // Exit address
  R[30] = 0x01000000; // Start of Stack
}

MIPS::CPU::~CPU() {}

/* ------------------------ CPU State R/W functions ------------------------ */

bool MIPS::CPU::stillExecuting() { return isExecuting; }

void MIPS::CPU::setRegister(int reg, uint32_t val) { R[reg] = val; }

int      MIPS::CPU::getCycle()           { return cycles; }
int      MIPS::CPU::getStage()           { return stage;  }
uint32_t MIPS::CPU::getPC()              { return PC;     }
uint32_t MIPS::CPU::getRegister(int reg) { return R[reg]; }
uint32_t MIPS::CPU::getiRegister(std::string reg) {
  #define IREG(name_str, var) if (reg == name_str) return var;
    IREG("RA", RA);
    IREG("RB", RB);
    IREG("RZ", RZ);
    IREG("RM", RM);
    IREG("RY", RY);
    IREG("IR", IR);
    IREG("PC", PC);
    IREG("hi", hi);
    IREG("lo", lo);
  #undef IREG

  return 0xBADA5555;
}

/* ---------------------------- Helper Methods ----------------------------- */

uint32_t MIPS::CPU::decodeOPcode(uint32_t IR) {
  bool hasImmediateValue = (IR >> 26) != 0;
  return (hasImmediateValue) 
    ? IR & 0xFC000000   // Mask off all register identifiers + immediate bits
    : IR & 0x0000003F;  // Mask off all register identifiers
}

/* ------------------------ CPU emulation functions ------------------------ */

void MIPS::CPU::do_cycle() {
  switch (stage) {
    case S_FETCH : fetch (); break;
    case S_DECODE: decode(); break;
    case S_EXEC  : exec  (); break;
    case S_MEMORY: memory(); break;
    case S_WBACK : wback (); break;
  }
  cycles++;
}

void MIPS::CPU::fetch() {
  // Check if we JR'd to the exit
  if (PC == 0x8123456C) {
    isExecuting = false;
    return;
  }

  // Do a memory read
  IR = MEM.load(PC);

  // Increment PC
  PC += 4;

  // Move onto next stage
  stage = S_DECODE;
}

void MIPS::CPU::decode() {
  uint8_t  $s, $t;

  $s = (IR >> 21) & 0x1F;
  $t = (IR >> 16) & 0x1F;

  RA = R[$s];
  RB = R[$t];

  stage = S_EXEC;
}

void MIPS::CPU::exec() {
  // Get signed values
  int32_t RA_s = static_cast<int32_t>(RA);
  int32_t RB_s = static_cast<int32_t>(RB);
  int16_t imm  = static_cast<int16_t>(IR & 0xFFFF);

  uint32_t opcode = decodeOPcode(IR);

  switch (opcode) {
    /* Arithmetic */
    case OP_ADD:   
      RZ = static_cast<uint32_t>(RA_s + RB_s);
      break;      
    case OP_SUB:   
      RZ = static_cast<uint32_t>(RA_s - RB_s);
      break;  
    case OP_SLT:   
      RZ = static_cast<uint32_t>(RA_s < RB_s);
      break;  
    case OP_SLTU:   
      RZ = RA < RB;
      break; 
    case OP_MFHI:   
      RZ = hi;
      break; 
    case OP_MFLO:   
      RZ = lo;
      break; 
    case OP_MULT:  
      lo = (static_cast<int64_t>(RA_s) * static_cast<int64_t>(RB_s));
      hi = (static_cast<int64_t>(RA_s) * static_cast<int64_t>(RB_s)) >> 32;
      break; 
    case OP_MULTU:   
      lo = (static_cast<uint64_t>(RA) * static_cast<uint64_t>(RB));            
      hi = (static_cast<uint64_t>(RA) * static_cast<uint64_t>(RB)) >> 32;
      break;
    case OP_DIV:   
      lo = RA_s / RB_s;           
      hi = RA_s % RB_s;
      break;  
    case OP_DIVU:   
      lo = RA / RB;             
      hi = RA % RB;
      break;

    /* Jumps and Branches */
    case OP_JR:  
      PC = RA;
      break;
    case OP_JALR:  
      RZ = PC;
      PC = RA;
      break;
    case OP_BEQ:  
      PC += (RA == RB) ? imm * 4 : 0;
      break;
    case OP_BNE:  
      PC += (RA != RB) ? imm * 4 : 0;
      break;

    /* Loads and Stores */
    case OP_SW:   
      RZ = RA + imm;
      RM = RB;
      break;
    case OP_LW:   
      RZ = RA + imm;
      break;

    /* Load Immediate Store */
    case OP_LIS:  
      RZ = PC; 
      PC += 4;
      break;

    default: 
      throw std::string("Invalid opcode"); 
      break;
  }

  stage = S_MEMORY;
}

void MIPS::CPU::memory() {
  switch (decodeOPcode(IR)) {
    case OP_LIS:
    case OP_LW: 
      RY = MEM.load(RZ); 
      break;
    case OP_SW:
      MEM.store(RZ, RM); 
      break;
    default:
      RY = RZ;
      break;
  }

  stage = S_WBACK;
}

void MIPS::CPU::wback() {
  int $d = (IR >> 11) & 0x1F;
  int $t = (IR >> 16) & 0x1F;

  switch (decodeOPcode(IR)) {
      case OP_MULT:
      case OP_MULTU:
      case OP_DIV:
      case OP_DIVU:
      case OP_JR:
      case OP_BEQ:
      case OP_BNE:
      case OP_SW:
        // No writeback
        break;
      case OP_ADD:    
      case OP_SUB:
      case OP_SLT:
      case OP_SLTU:
      case OP_MFHI:
      case OP_MFLO:
      case OP_LIS:
        // Writeback to $d
        if ($d != 0) R[$d] = RY;
        break;
      case OP_LW:
        // Writeback to $t
        if ($t != 0) R[$t] = RY;
        break;
      case OP_JALR  :
        // Writeback to $31
        R[31] = RY;
        break;
  }

  stage = S_FETCH;
}