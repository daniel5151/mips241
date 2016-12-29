#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <cstring>
#include <cstdint>

#include "disasm.h"

#define ever (;;)

using namespace std;

char swap(char & a, char & b) {
  char tmp = a;
  a = b;
  b = tmp;
}

uint32_t readMIPSbyte(istream & bin) {
  char word_c[4];

  bin.read(word_c, 4);

  if(bin.eof()) throw string("EOF");

  // Reverse endiannessss
  swap(word_c[0], word_c[3]);
  swap(word_c[1], word_c[2]);

  return *(uint32_t *)word_c;
}

template <typename T>
string toHex(T const& x) {
  ostringstream out;
  out << "0x" << setfill('0') << setw(8) << hex << right << x;
  return out.str();
}

int main(int argc, char const *argv[]) {
  // Check arguments
  if (argc != 2) {
    cerr 
      << "Usage: ./disasm <filename>" 
      << endl;
    return -1;
  }

  // Load program
  ifstream bin (argv[1], ifstream::binary);

  // Check for IO error
  if (!bin.is_open()) {
    cerr << "Cannot open file." << endl;
    return -1;
  }

  // This var is also used later to load in user data after program!
  uint32_t memaddr = 0x0;
  uint32_t word;

  bool isMERL = false;
  int fileLength;
  int codeLength;
  
  for ever {
    try {
      word = readMIPSbyte(bin);
    } catch (const string & msg) {
      if (msg == "EOF") break;
    }

    if (memaddr == 0x0 && word == 0x10000002) {
      isMERL = true;
      cout 
        << ".word 0x10000002" 
        << " ; MERL cookie" 
        << endl;
    }
    else if (memaddr == 0x4 && isMERL) {
      fileLength = (int)word;
      cout 
        << ".word " << toHex(word)
        << " ; File Length (in bytes) is "
        << fileLength
        << endl;
    }
    else if (memaddr == 0x8 && isMERL) {
      codeLength = (int)word;
      cout 
        << ".word " << toHex(word)
        << " ; Code Length (in bytes) is "
        << codeLength
        << endl;
    }
    else if (isMERL && memaddr >= codeLength) {
      if (word == 0x01) {
        // Relocation Table
        uint32_t addr2relocate;
        try { addr2relocate = readMIPSbyte(bin); } 
        catch (const string & msg) {
          if (msg == "EOF") {
            cerr 
              << "ERROR: "
              << "Abrupt EOF when trying to read a REL"
              << endl;
            break;
          }
        }

        cout 
          << ".word 0x01       ; REL" 
          << endl;
        cout << ".word " << toHex(addr2relocate) 
          << " ; === ln " << (int)(addr2relocate / 4 + 1)
          << endl;
      }

      if (word == 0x05) {
        // ESD
        uint32_t addr2import;
        int   name_len;
        char* name;
        try { 
          addr2import = readMIPSbyte(bin);
          name_len    = readMIPSbyte(bin);

          name = new char [name_len + 1];
          for (int i = 0; i < name_len; i++)
            name[i] = (char)readMIPSbyte(bin);

          name[name_len] = '\0';
        } 
        catch (const string & msg) {
          if (msg == "EOF") {
            cerr 
              << "ERROR: "
              << "Abrupt EOF when trying to read a ESD"
              << endl;
            break;
          }
        }

        cout 
          << ".word 0x05       ; ESD - "
          << name
          << endl;
        cout 
          << ".word " << toHex(addr2import) 
          << " ; === ln " << (int)(addr2import / 4 + 1)
          << endl;
        for (int i = 0; i < name_len; i++) {
          cout 
            << ".word " << setw(3) << left << (int)name[i] 
            << " ; " << name[i] 
            << endl;
        }

        delete [] name;
      }

      if (word == 0x11) {
        // ESR
        uint32_t addr2export;
        int   name_len;
        char* name;
        try { 
          addr2export = readMIPSbyte(bin);
          name_len    = readMIPSbyte(bin);

          name = new char [name_len + 1];
          for (int i = 0; i < name_len; i++)
            name[i] = (char)readMIPSbyte(bin);

          name[name_len] = '\0';
        } 
        catch (const string & msg) {
          if (msg == "EOF") {
            cerr 
              << "ERROR: "
              << "Abrupt EOF when trying to read a ESR"
              << endl;
            break;
          }
        }

        cout 
          << ".word 0x11       ; ESR - "
          << name
          << endl;
        cout 
          << ".word " << toHex(addr2export) 
          << " ; === ln " << (int)(addr2export / 4 + 1)
          << endl;
        for (int i = 0; i < name_len; i++) {
          cout 
            << ".word " << setw(3) << left << (int)name[i] 
            << " ; " << name[i] 
            << endl;
        }
        
        delete [] name;
      }
    }
    else cout << MIPS::disasm(word) << endl;

    memaddr += 4;
  }

  return 0;
}