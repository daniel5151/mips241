#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <cstdint>

#include <unistd.h>

#include "ram.h"
#include "bus.h"
#include "cpu.h"

#include "debug.h"

#define ever (;;)

uint32_t readNum(std::istream & in) {
  std::string n_str;

  in >> n_str;
  std::stringstream n_ss (n_str);

  bool isDec = !(n_str[0] == '0' && n_str[1] == 'x') && !(n_str[1] == '0' && n_str[2] == 'x');

  long int x;
  n_ss >> ( (isDec) ? std::dec : std::hex ) >> x;

  return x;
}

int main(int argc, char const *argv[]) {
  // Check arguments
  if (argc < 2 || argc > 4) {
    std::cerr
      << "Usage: ./mips241 <filename> [--cin] [--twoints] [--debug]"
      << std::endl
      << std::endl
      << "  By default, mips241 operates in `mips.array` mode."    << std::endl
      << "  To switch to `mips.twoints` mode, pass in [--twoints]" << std::endl
      << std::endl
      << "    --cin     : Read program from cin"          << std::endl
      << "    --twoints : operate in `mips.twoints` mode" << std::endl
      << "    --debug   : enable interactive debug mode"  << std::endl
      << std::endl
      << "  Known issues:"  << std::endl
      << "    - Don't use [--debug] in conjunction with [--cin], it won't work." << std::endl
      << "    - [--cin] mode doesn't let you input initial register values." << std::endl
      << "        It's not that it's impossible, i'm just lazy and did not" << std::endl
      << "        implement it yet :)" << std::endl
      << std::endl
      << std::endl;
    return -1;
  }

  std::string filename;
  bool use_debug_mode = false;
  bool use_twoints = false;
  bool use_cin = false;
  for (int i = 1; i < argc; i++) {
    std::string arg = string(argv[i]);

    if (arg == "--twoints") use_twoints = true;
    if (arg == "--debug" ) use_debug_mode = true;
    if (arg == "--cin") use_cin = true;

    if (i == 1) filename = arg;
  }

  //--------------------------------- SETUP ---------------------------------//

  // Setup and connect up System
  MIPS::RAM ram;
  MIPS::BUS bus (ram);
  MIPS::CPU cpu (bus);

  // Open file
  std::ifstream f (filename, std::ifstream::binary);
  if (!f.is_open() && !use_cin) {
    std::cerr << "Cannot open file `" << filename << "`" << std::endl;
    return -1;
  }

  std::istream &bin = (use_cin) ? std::cin : f;

  // Insert executable into virtual memory

  uint32_t memaddr = 0x0; // base memaddr

  char word[4];
  for ever {
    bin.read(word, 4);

    if(bin.eof()) break;

    // Reverse endiannessss
    char x;
    x = word[0];
    word[0] = word[3];
    word[3] = x;
    x = word[1];
    word[1] = word[2];
    word[2] = x;

    // Store it in RAM (with some funky casting)
    ram.store(memaddr, *(uint32_t *)word );

    memaddr += 4;
  }

  // Accept user input
  if (use_twoints) {
    // Let user input 2 integers
    std::cerr << "Enter value for Register 1: ";
    cpu.setRegister(1, readNum(std::cin));
    std::cerr << "Enter value for Register 2: ";
    cpu.setRegister(2, readNum(std::cin));
  } else {
    // Let user enter Array of values
    std::cerr << "Enter length of Array: ";
    int arr_length = readNum(std::cin);
    cpu.setRegister(2, arr_length); // $2 = length of array

    // Start populating the array 16 words after the end of the program
    // this was chosen relatively arbitrarilly...
    memaddr += 0x10;

    cpu.setRegister(1, memaddr); // $1 = Pointer to base of array

    for (int i = 0; i < arr_length; i++) {
      std::cerr << "Enter Array element " << i << ": ";
      ram.store(memaddr, readNum(std::cin));
      memaddr += 4;
    }
  }

  // If this program is running interactively, clear cin before starting cpu
  //  emulation
  bool isTTY = isatty(fileno(stdin));
  if (isTTY) { std::string dummy; getline(std::cin, dummy); }


  std::cerr << std::endl;

  // Start CPU execution
  MIPS::Debugger debugger (cpu, ram, bus);

  try {
    std::cerr << "Starting CPU..." << std::endl;
    do {
      if (use_debug_mode) debugger.debugREPL();

      // Execute cpu cycle
      cpu.do_cycle();

      std::cout << bus.getOutput();

    } while (cpu.stillExecuting());

    std::cerr
      << "Execution completed successfully!"
      << std::endl;
    debugger.printCPUState();

  } catch (const std::string & msg) {
    std::cerr
      << "ERROR : "
      << msg
      << std::endl;
    debugger.printCPUState();
    return -1;
  }

  return 0;
}
