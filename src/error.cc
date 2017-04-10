#include "error.h"

void MIPS::error(std::string msg) {
  throw msg;
}
