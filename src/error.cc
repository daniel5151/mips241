#include "error.h"

void MIPS::error(std::string msg) {
  throw msg;
}

// If emscripten

// #include <emscripten.h>
// void MIPS::error(std::string str) {
//   stringstream js; js << "throw '" << str << "'";
//   emscripten_run_script(js.str().c_str());
// }
