#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <map>
#include <vector>

#include <cstdint>

#include "disasm.h"

#define ever (;;)

using namespace std;

void swap(char & a, char & b) {
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

  vector<string> outLines;

  map<int, int> branches; // <beq/bne line no, offset>

  // Load program
  ifstream bin (argv[1], ifstream::binary);

  // Check for IO error
  if (!bin.is_open()) {
    cerr << "Cannot open file." ;
    return -1;
  }


  // Disasm
  uint32_t word;

  bool isMERL = false;
  int fileLength;
  int codeLength;
  
  for ever {
    stringstream outLn;

    try { word = readMIPSbyte(bin); } 
    catch (const string & msg) {
      if (msg == "EOF") break;
    }

    if (outLines.size() == 0 && word == 0x10000002) {
      isMERL = true;
      outLn 
        << "    .word 0x10000002" 
        << " ; MERL cookie";
      outLines.push_back(outLn.str());
    }
    else if (outLines.size() == 1 && isMERL) {
      fileLength = (int)word;
      outLn 
        << "    .word " << toHex(word)
        << " ; File Length (in bytes) is "
        << fileLength;
      outLines.push_back(outLn.str());
    }
    else if (outLines.size() == 2 && isMERL) {
      codeLength = (int)word;
      outLn 
        << "    .word " << toHex(word)
        << " ; Code Length (in bytes) is "
        << codeLength;
      outLines.push_back(outLn.str());
    }
    else if (isMERL && outLines.size() * 4 >= codeLength) {
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

        outLn << "    .word 0x01       ; REL";
        outLines.push_back(outLn.str());

        outLn = stringstream(); // clear stringstream
        outLn << "    .word " << toHex(addr2relocate);
        outLines.push_back(outLn.str());

        outLines[(int)(addr2relocate / 4)] += " ; REL'd";
      }

      if (word == 0x05) {
        // ESD
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
              << "Abrupt EOF when trying to read a ESD"
              << endl;
            break;
          }
        }

        outLn 
          << "    .word 0x05       ; ESD - "
          << name;
        outLines.push_back(outLn.str());

        outLn = stringstream(); // clear stringstream
        outLn << "    .word " << toHex(addr2export);
        outLines.push_back(outLn.str());


        for (int i = 0; i < name_len; i++) {
          outLn = stringstream(); // clear stringstream
          outLn 
            << "    .word " << setw(3) << left << (int)name[i] 
            << " ; " << name[i];
          outLines.push_back(outLn.str());
        }

        int esdLine = (int)(addr2export / 4);
        outLines[esdLine] = string(name) + ":\n" + outLines[esdLine];

        delete [] name;
      }

      if (word == 0x11) {
        // ESR
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
              << "Abrupt EOF when trying to read a ESR"
              << endl;
            break;
          }
        }

        outLn 
          << "    .word 0x11       ; ESR - "
          << name;
        outLines.push_back(outLn.str());

        outLn = stringstream(); // clear stringstream
        outLn << "    .word " << toHex(addr2import);
        outLines.push_back(outLn.str());

        for (int i = 0; i < name_len; i++) {
          outLn = stringstream(); // clear stringstream
          outLn 
            << "    .word " << setw(3) << left << (int)name[i] 
            << " ; " << name[i];
          outLines.push_back(outLn.str());
        }
        

        outLines[(int)(addr2import / 4)] += " ; ESR - " + string(name);

        delete [] name;
      }
    }
    else {
      // Not MERL Stuff
      string         disasmLn      (MIPS::disasm(word));
      stringstream   disasmLn_ss   (disasmLn);
      vector<string> disasmLn_toks ;
      
      string buf;
      while (disasmLn_ss >> buf)
        disasmLn_toks.push_back(buf);

      if (disasmLn_toks[0] == "lis") {
        outLines.push_back("    " + disasmLn);

        uint32_t lisVal;
        try { lisVal = readMIPSbyte(bin); } 
        catch (const string & msg) {
          if (msg == "EOF") {
            cerr
              << "ERROR: "
              << "Abrupt EOF after lis instruction"
              << endl;
            break;
          }
        }
        outLn << "    .word " << toHex(lisVal);
        outLines.push_back(outLn.str());
      } 
      // else if (disasmLn_toks[0] == "bne" || disasmLn_toks[0] == "beq") {
      //   stringstream offset_ss (disasmLn_toks.back());
      //   int offset;
      //   offset_ss >> offset;

      //   branches[outLines.size()] = branches.size();
        
      //   outLn << "    " << disasmLn_toks[0] << "   ";
      //   for (auto it = disasmLn_toks.begin() + 1; it != disasmLn_toks.end() - 1; it++)
      //     outLn << *it << " ";
      //   outLn << "loop"; // Gets filled in later
        
      //   outLines.push_back(outLn.str());
      // } 
      else if (disasmLn == "jr    $31") {
        outLines.push_back("    " + disasmLn + "\n\n\n\n");
      }
      else {
        outLines.push_back("    " + disasmLn);
      }
    }
  }

  // Fill in branches
  // map<int, int> branch_targets; // <branch target ln, id>
  // for (auto it = branches.begin(); it != branches.end(); it++) {
  //   // Calculate branch target
  //   int target = it->first + it->second;

  //   // Add it to the targets map if it's not there yet, or use the existing
  //   //  branch id
  //   if (branch_targets.count(target) == 0)
  //     branch_targets[target] = branch_targets.size();

  //   // Add loop id to beq / bne instruction
  //   stringstream id_stoi; 
  //   id_stoi << branch_targets[target];
  //   outLines[it->first] += id_stoi.str();
  // }

  // for (auto it = branch_targets.begin(); it != branch_targets.end(); it++) {
  //   stringstream id_stoi;
  //   id_stoi << it->second;

  //   outLines[it->first] = "loop" + id_stoi.str() + ":\n" + outLines[it->first];
  // }

  // Output all of our hard work
  for (auto it = outLines.begin(); it != outLines.end(); it++)
    cout << *it << endl;

  return 0;
}