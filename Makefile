mips241:
	cd src && $(MAKE)
	mv src/mips241 mips241

.PHONY: clean
clean:
	rm -rf src/.o/ mips241

.PHONY: cleandep
cleandep:
	rm -rf src/.d/

.PHONY: cleaner
cleaner:
	rm -rf src/.d/
	rm -rf src/.o/ mips241

# TODO: Make this makefile less terrible
emscripted:
	# make sure emscripten is in your path!
	em++ -s DISABLE_EXCEPTION_CATCHING=0 -O3 --bind -o ./public/mips241.js ./embind.cc ./src/bus.cc ./src/cpu.cc ./src/debug.cc ./src/ram.cc ./_disasm/disasm.cc ./_asm/asm.cc ./_asm/error.cc ./_asm/kind.cc ./_asm/lexer.cc # not main.cc!

.PHONY: clean_em
clean_em:
	rm -rf ./public/mips241.js ./public/mips241.js.mem