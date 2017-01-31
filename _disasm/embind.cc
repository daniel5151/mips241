#include "disasm.h"

#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(mips241disasm) {
  emscripten::function("MIPS$$disasm", MIPS::disasm);
}