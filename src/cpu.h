#ifndef CPU_H_
#define CPU_H_

#include "bus.h"

#include <iostream>
#include <cstdint>

namespace MIPS {
  class CPU {
    bool isExecuting;

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
      OP_ADD   = 0b00000000000000000000000000100000,    
      OP_SUB   = 0b00000000000000000000000000100010,
      OP_SLT   = 0b00000000000000000000000000101010,
      OP_SLTU  = 0b00000000000000000000000000101011,
      OP_MFHI  = 0b00000000000000000000000000010000,
      OP_MFLO  = 0b00000000000000000000000000010010,
      OP_MULT  = 0b00000000000000000000000000011000,
      OP_MULTU = 0b00000000000000000000000000011001,
      OP_DIV   = 0b00000000000000000000000000011010,
      OP_DIVU  = 0b00000000000000000000000000011011,
      OP_LIS   = 0b00000000000000000000000000010100,
      OP_JR    = 0b00000000000000000000000000001000,
      OP_JALR  = 0b00000000000000000000000000001001,
      OP_BEQ   = 0b00010000000000000000000000000000,
      OP_BNE   = 0b00010100000000000000000000000000,
      OP_LW    = 0b10001100000000000000000000000000,
      OP_SW    = 0b10101100000000000000000000000000
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
    static uint32_t decodeOPcode(uint32_t IR);

  public:
    CPU(BUS & meminterface);
    ~CPU();

    static void word2instr(uint32_t IR);

    void printState(std::ostream & out);

    bool stillExecuting();

    uint32_t getRegister(int reg);
    void setRegister(int reg, uint32_t val);

    uint32_t getPC();

    void do_cycle();
  };
}

#endif