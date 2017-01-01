mips241:
	cd src; make
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