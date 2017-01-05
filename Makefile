mips241.js:
	cd src; make
	mv src/mips241.js mips241.js


.PHONY: clean
clean:
	rm -rf mips241.js