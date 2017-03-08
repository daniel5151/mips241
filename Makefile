# TODO: Make this makefile less terrible

mips241:
	# make sure emscripten is in your path!
	em++ -O3 --bind -o ./dist/mips241.js ./src/bus.cc ./src/cpu.cc ./src/debug.cc ./src/embind.cc ./src/ram.cc ./_disasm/disasm.cc ./_asm/asm.cc ./_asm/kind.cc ./_asm/lexer.cc # not main.cc!

dev:
	# make sure emscripten is in your path!
	em++ --bind -o ./dist/mips241.js ./src/bus.cc ./src/cpu.cc ./src/debug.cc ./src/embind.cc ./src/ram.cc ./_disasm/disasm.cc ./_asm/asm.cc ./_asm/kind.cc ./_asm/lexer.cc # not main.cc!

.PHONY: clean
clean:
	rm -rf ./dist/mips241.js ./dist/mips241.js.mem