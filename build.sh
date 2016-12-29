# this should be a makefile...

cd src
g++ -o ../mips241 main.cc cpu.cc mem.cc bus.cc disasm.cc debug.cc -std=c++11 -Wall