#ifndef CPU_H_
#define CPU_H_

#include "bus.h"

#include <iostream>
#include <cstdint>

namespace MIPS {
  class CPU {
    bool isExecuting;
    int  cycles;

    // General Purpose Registers
    uint32_t R[32];

    // Internal Registers
    uint32_t IR;
    uint32_t PC;

    uint32_t hi;
    uint32_t lo;

    // Memory interface
    BUS & MEM;

    // ---- Control Variables ---- //
    enum Stage {
      S_FETCH, 
      S_DECODE, 
      S_EXEC, 
      S_MEMORY, 
      S_WBACK
    };
    Stage stage;

    // ---- opcodes ---- //
    enum OPCODE {
      OP_ADD   = 0x00000020,    
      OP_SUB   = 0x00000022,
      OP_SLT   = 0x0000002A,
      OP_SLTU  = 0x0000002B,
      OP_MFHI  = 0x00000010,
      OP_MFLO  = 0x00000012,
      OP_MULT  = 0x00000018,
      OP_MULTU = 0x00000019,
      OP_DIV   = 0x0000001A,
      OP_DIVU  = 0x0000001B,
      OP_LIS   = 0x00000014,
      OP_JR    = 0x00000008,
      OP_JALR  = 0x00000009,
      OP_BEQ   = 0x10000000,
      OP_BNE   = 0x14000000,
      OP_LW    = 0x8C000000,
      OP_SW    = 0xAC000000
    };

    // ---- Inter-stage Registers ---- //
    uint32_t RA;
    uint32_t RB;
    uint32_t RZ;
    uint32_t RM;
    uint32_t RY;

    // ---- Stages ---- //
    void fetch ();
    void decode();
    void exec  ();
    void memory();
    void wback ();

    // ---- Helpers ---- //
    uint32_t decodeOPcode(uint32_t IR);

  public:
    CPU(BUS & meminterface);
    ~CPU();

    bool stillExecuting();

    int      getCycle();
    int      getStage();
    uint32_t getRegister(int reg);
    uint32_t getiRegister(std::string reg);
    uint32_t getPC();

    void setRegister(int reg, uint32_t val);

    void do_cycle();
  };
}

#endif