#include <cstdint>
#include <sstream>
#include <iomanip>

#include "disasm.h"

#define COUTHEX(x) \
  "0x" << std::setfill('0') << std::setw(8) << std::right << std::hex << x << std::dec

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

std::string opcode2str(uint32_t opcode) {
  switch (opcode) {
    case OP_ADD:   return "add  ";
    case OP_SUB:   return "sub  ";
    case OP_SLT:   return "slt  ";
    case OP_SLTU:  return "sltu ";
    case OP_MFHI:  return "mfhi ";
    case OP_MFLO:  return "mflo ";
    case OP_MULT:  return "mult ";
    case OP_MULTU: return "multu";
    case OP_DIV:   return "div  ";
    case OP_DIVU:  return "divu ";
    case OP_LIS:   return "lis  ";
    case OP_JR:    return "jr   ";
    case OP_JALR:  return "jalr ";
    case OP_BEQ:   return "beq  ";
    case OP_BNE:   return "bne  ";
    case OP_LW:    return "lw   ";
    case OP_SW:    return "sw   ";
  }
  return "INVALID";
}

std::string MIPS::disasm(uint32_t word) {
  std::stringstream instr_ss;

  uint8_t  $s, $t, $d;

  int32_t imm;
  uint16_t imm_u;

  $s = (word >> 21) & 0x1F;
  $t = (word >> 16) & 0x1F;
  $d = (word >> 11) & 0x1F;

  imm_u = (word & 0xFFFF);

  bool isNeg = imm_u & 0x8000;
  if (isNeg) {
    imm_u -= 1;
    imm_u = ~imm_u;
    imm = - ((int32_t)imm_u);
  } else {
    imm = imm_u;
  }

  bool hasImmediateValue = (word >> 26) != 0;
  uint32_t opcode = (hasImmediateValue)
    ? word & 0xFC000000   // Mask off all register identifiers + immediate bits
    : word & 0x0000003F;  // Mask off all register identifiers

  switch (opcode) {
      case OP_MULT:
      case OP_MULTU:
      case OP_DIV:
      case OP_DIVU:
        // 2 src
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$s
          << ", "
          << "$" << (int)$t;
        break;
      case OP_BEQ:
      case OP_BNE:
        // 2 src + immediate
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$s
          << ", "
          << "$" << (int)$t
          << ", "
          << imm;
        break;

      case OP_LW:
        // 1 source + immediate + 1 dst
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$t
          << ", "
          << imm
          << "("
          << "$" << (int)$s
          << ")";
        break;
      case OP_SW:
        // 2 source + immediate
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$t
          << ", "
          << imm
          << "("
          << "$" << (int)$s
          << ")";
        break;

      case OP_ADD:
      case OP_SUB:
      case OP_SLT:
      case OP_SLTU:
        // 2 sources, 1 dst
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$d
          << ", "
          << "$" << (int)$s
          << ", "
          << "$" << (int)$t;
        break;

      case OP_MFHI:
      case OP_MFLO:
      case OP_LIS:
        // 1 dst
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$d;
        break;
      case OP_JALR:
      case OP_JR:
        // 1 src
        instr_ss
          << opcode2str(opcode) << " "
          << "$" << (int)$s;
        break;
      default:
        instr_ss
          << ".word "
          << COUTHEX( word );
  }

  return instr_ss.str();
}