#ifndef DISASM_H_
#define DISASM_H_

#include <cstdint>
#include <string>

namespace MIPS {
  std::string disasm(uint32_t word);
}

#endif