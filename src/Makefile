# TODO: Make this makefile less terrible

mips241.js:
	# make sure emscripten is in your path!
	em++ --bind -o mips241.js bus.cc cpu.cc debug.cc embind.cc ram.cc ../_disasm/disasm.cc ../_asm/asm.cc ../_asm/kind.cc ../_asm/lexer.cc # not main.cc!

