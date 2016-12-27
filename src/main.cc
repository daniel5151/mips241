#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <cstdint>

#include "mem.h"
#include "bus.h"
#include "cpu.h"

#define ever (;;)

int readNum(std::istream & in) {
  std::string n_str;

  in >> n_str;
  std::stringstream n_ss (n_str);

  bool isDec = (n_str.length() < 2) || (n_str[0] != '0');

  int x;
  n_ss >> ( (isDec) ? std::dec : std::hex ) >> x;

  return x;
}

int main(int argc, char const *argv[]) {
  // Check arguments
  if (argc < 2 || argc > 3) {
    std::cerr 
      << "Usage: ./mips241 <filename> [-twoints]" 
      << std::endl;
    return -1;
  }

  bool use_twoints = argc == 3; // make better later lol

  //--------------------------------- SETUP ---------------------------------//

  // Setup and connect up System
  MIPS::RAM ram;
  MIPS::BUS bus (ram);
  MIPS::CPU cpu (bus);

  // Load program into RAM
  std::ifstream bin (argv[1], std::ifstream::binary);

  // Check for IO error
  if (!bin.is_open()) {
    std::cerr << "Cannot open file." << std::endl;
    return -1;
  }

  // This var is also used later to load in user data after program!
  uint32_t memaddr = 0x0;
  
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
    memaddr += 0x10;

    cpu.setRegister(1, memaddr); // $1 = Pointer to base of array

    for (int i = 0; i < arr_length; i++) {
      std::cerr << "Enter Array element " << i << ": ";
      ram.store(memaddr, readNum(std::cin));
      memaddr += 4;
    }
  }

  std::cerr << std::endl;

  ram.printFrom(0x0, 32, std::cerr);

  // Start CPU execution
  int cycle = 0;
  try {
    std::cerr << "Starting CPU..." << std::endl;
    do {
      // Execute cpu cycle
      cpu.do_cycle();
      cycle++;

      uint32_t startAddr = ((int)cpu.getPC() - 0x20 >= 0x0) ? cpu.getPC() - 0x20 : 0x0;
      ram.printFrom(startAddr, 16, std::cerr, cpu.getPC());

      ram.printFrom(cpu.getRegister(30) - 0x10, 12, std::cerr, cpu.getRegister(30));

      cpu.printState(std::cerr);
      std::cerr << "Cycle no. " << std::dec << cycle << std::endl << std::endl;
      if (cycle % 5 == 0) 
        getchar();

    } while (cpu.stillExecuting());

    std::cerr
      << std::endl
      << "Execution completed successfully!"
      << std::endl;
    cpu.printState(std::cerr);

  } catch (const std::string & msg) {
    std::cerr
      << "ERROR : "
      << msg
      << std::endl;
    cpu.printState(std::cerr);
  }

  return 0;
}