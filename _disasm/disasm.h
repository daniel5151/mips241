#ifndef DISASM_H_
#define DISASM_H_

#include <cstdint>

namespace MIPS {
  std::string disasm(uint32_t word);
}

#endif