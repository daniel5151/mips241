# TODO: Make this makefile less terrible

mips241.js:
	# make sure emscripten is in your path!
	em++ -s ALLOW_MEMORY_GROWTH=1 --bind -o mips241.js bus.cc cpu.cc debug.cc embind.cc ram.cc ../_disasm/disasm.cc ../_asm/assemble.cc ../_asm/kind.cc ../_asm/lexer.cc # not main.cc!

