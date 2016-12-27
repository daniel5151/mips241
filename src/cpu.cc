#include "cpu.h"
#include "bus.h"

#include "disasm.h"

#include <cstdint>

#include <iostream>
#include <iomanip>

#define COUTHEX(x) \
  "0x" << std::setfill('0') << std::setw(8) << std::right << std::hex << x << std::dec

int64_t uint32_to_int64(uint32_t x) {
  bool isNeg = x & 0x80000000;
  if (isNeg) {
    x -= 1;
    x = ~x;
    return -( (int64_t)x );
  }

  return x;
}

int32_t uint16_to_int32(uint16_t x) {
  bool isNeg = x & 0x8000;
  if (isNeg) {
    x -= 1;
    x = ~x;
    return -( (int32_t)x );
  }
  return x;
}

uint32_t int64_to_uint32(int64_t x) {
  if (x < 0) {
    uint32_t nx;
    nx = -x;
    nx = ~nx;
    nx += 1;
    return nx;
  }
  return x;
}








MIPS::CPU::CPU(MIPS::BUS & bus) : MEM(bus) 
{ 
  isExecuting = true;
  cycles = 0;

  stage = S_FETCH;

  PC = 0x00000000;

  hi = lo = 0;
  RA = RB = RZ = RM = RY = 0;


  for (int i = 0; i < 32; i++) 
    R[i] = 0;

  R[31] = 0x8123456C;
  R[30] = 0x01000000;
}

MIPS::CPU::~CPU() {}

/* ------------------------ CPU State R/W functions ------------------------ */

bool MIPS::CPU::stillExecuting() { return isExecuting; }

void MIPS::CPU::setRegister(int reg, uint32_t val) { R[reg] = val; }

int      MIPS::CPU::getCycle()           { return cycles; }
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

/* ------------------------- Static Helper Methods ------------------------- */

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

  std::cerr << COUTHEX(PC) << " : ";

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

  std::cerr << MIPS::disasm(IR) << std::endl;

  stage = S_EXEC;
}

void MIPS::CPU::exec() {
  int64_t RA_s = uint32_to_int64(RA); // Get signed value
  int64_t RB_s = uint32_to_int64(RB); // Get signed value

  int32_t imm = uint16_to_int32( IR & 0xFFFF );

  uint32_t opcode = MIPS::CPU::decodeOPcode(IR);

  switch (opcode) {
    /* Arithmetic */
    case MIPS::CPU::OP_ADD: { 
      RZ = int64_to_uint32(RA_s + RB_s);
    } break;      
    case MIPS::CPU::OP_SUB: { 
      RZ = int64_to_uint32(RA_s - RB_s);
    } break;  
    case MIPS::CPU::OP_SLT: { 
      RZ = int64_to_uint32(RA_s < RB_s);
    } break;  
    case MIPS::CPU::OP_SLTU: { 
      RZ = RA < RB;
    } break; 
    case MIPS::CPU::OP_MFHI: { 
      RZ = hi;
    } break; 
    case MIPS::CPU::OP_MFLO: { 
      RZ = lo;
    } break; 
    case MIPS::CPU::OP_MULT: { 
      lo = int64_to_uint32(RA_s * RB_s);        
      hi = int64_to_uint32((RA_s * RB_s) >> 32); 
    } break; 
    case MIPS::CPU::OP_MULTU: { 
      lo =  (uint64_t)RA * (uint64_t)RB;            
      hi = ((uint64_t)RA * (uint64_t)RB) >> 32;
    } break;
    case MIPS::CPU::OP_DIV: { 
      lo = int64_to_uint32(RA_s / RB_s);           
      hi = int64_to_uint32(RA_s % RB_s);
    } break;  
    case MIPS::CPU::OP_DIVU: { 
      lo = RA / RB;             
      hi = RA % RB;
    } break;

    /* Jumps and Branches */
    case MIPS::CPU::OP_JR: {
      PC = RA;
    } break;
    case MIPS::CPU::OP_JALR: {
      RZ = PC;
      PC = RA;
    } break;
    case MIPS::CPU::OP_BEQ: {
      PC += (RA == RB) ? imm * 4 : 0;
    } break;
    case MIPS::CPU::OP_BNE: {
      PC += (RA != RB) ? imm * 4 : 0;
    } break;

    /* Loads and Stores */
    case MIPS::CPU::OP_SW: { 
      RZ = RA + imm;
      RM = RB;
    } break;
    case MIPS::CPU::OP_LW: { 
      RZ = RA + imm;
    } break;

    /* Load Immediate Store */
    case MIPS::CPU::OP_LIS: {
      RZ = PC; 
      PC += 4;
    } break;

    default: 
      throw std::string("Invalid opcode"); 
      break;
  }

  stage = S_MEMORY;
}

void MIPS::CPU::memory() {
  std::string indent = "                                   ";
  switch (MIPS::CPU::decodeOPcode(IR)) {
    case MIPS::CPU::OP_LW: {
      RY = MEM.load(RZ);

      std::cerr 
        << indent << "Load  : ["
        << COUTHEX(RZ)
        << "] = "
        << COUTHEX(RY)
        << std::endl;
    } break;
    case MIPS::CPU::OP_SW: {
      std::cerr 
        << indent << "Store : ["
        << COUTHEX(RZ)
        << "] <- "
        << COUTHEX(RZ)
        << std::endl;

      MEM.store(RZ, RM);
    } break;

    case MIPS::CPU::OP_LIS: {
      RY = MEM.load(RZ);

      std::cerr 
        << indent << "Load  : ["
        << COUTHEX(RZ)
        << "] = "
        << COUTHEX(RY)
        << std::endl;
    } break;

    default:
      RY = RZ;
      break;
  }

  stage = S_WBACK;
}

void MIPS::CPU::wback() {
  int $d = (IR >> 11) & 0x1F;
  int $t = (IR >> 16) & 0x1F;

  switch (MIPS::CPU::decodeOPcode(IR)) {
      case OP_MULT:
      case OP_MULTU:
      case OP_DIV:
      case OP_DIVU:
      case OP_JR:
      case OP_BEQ:
      case OP_BNE:
      case OP_SW:
        // No writeback
        // std::cerr << "  No Write Back" << std::endl;
        break;
      case OP_ADD:    
      case OP_SUB:
      case OP_SLT:
      case OP_SLTU:
      case OP_MFHI:
      case OP_MFLO:
      case OP_LIS:
        R[$d] = RY;
        break;
      case OP_LW:
        // Writeback to $d
        // std::cerr << "  Write Back to $" << (int)$d << std::endl;
        R[$t] = RY;
        break;
      case OP_JALR  :
        // std::cerr << "  Write Back to $31" << std::endl;
        R[31] = RY;
        break;
  }





  stage = S_FETCH;
}


/* ------------------------- CPU Utility Functions ------------------------- */

void MIPS::CPU::printState(std::ostream & out) {
  for (int i = 1; i < 32; i++) {
    if (i % 4 - 1 == 0 && i != 0) {
      out << std::endl;
    }
    out << std::setfill(' ') << ( (i % 4 - 1 != 0) ? "   " : "" ) << "$" 
        << std::setw(2) << std::left << std::dec << i 
        << " = " << COUTHEX(R[i]);
  }

  out << std:: endl;
  out << " PC = " << COUTHEX(PC)
      << "   "
      << " IR = " << COUTHEX(IR)
      << std::endl 
      << std::endl;

  out << " lo = " << COUTHEX(hi) << "   "
      << " hi = " << COUTHEX(lo)
      << std::endl 
      << std::endl;

  out << " RA = " << COUTHEX(RA) << "   "
      << " RB = " << COUTHEX(RB) 
      << std::endl;
  out << " RZ = " << COUTHEX(RZ)
      << std::endl;
  out << " RM = " << COUTHEX(RM)
      << std::endl;
  out << " RY = " << COUTHEX(RY)
      << std::endl;
  out << "Stage: " << stage << std::endl;
}