FLAGS = -I./deps/linenoise -I./deps/sds
PREFIX?=/usr/local

PANDOC=$(shell which pandoc)

default: clac

deps/linenoise/linenoise.o:
	@cd deps/linenoise && $(MAKE)

deps/sds/sds.o:
	@cd deps/sds && $(MAKE)

clac: clac.c deps/linenoise/linenoise.o deps/sds/sds.o
	$(CC) $(FLAGS) -Wall -Os -o clac clac.c -lm deps/linenoise/linenoise.o deps/sds/sds.o

clean:
	@echo cleaning
	@rm -f clac
	@rm -f deps/sds/sds.o
	@rm -f deps/linenoise/linenoise.o

install: clac
	@echo installing executable to ${PREFIX}/bin
	@mkdir -p ${PREFIX}/bin
	@cp -f clac ${PREFIX}/bin/clac
	@chmod 755 ${PREFIX}/bin/clac

install-man: man
	@echo installing manpage
	@mkdir -p ${PREFIX}/share/man/man1/
	@cp -f clac.1 ${PREFIX}/share/man/man1

uninstall:
	@echo removing executable from ${PREFIX}/bin
	@rm ${PREFIX}/bin/clac

uninstall-man: man
	@echo uninstalling manpage
	@rm -p ${PREFIX}/share/man/man1/clac.1

man: clac.1
	
clac.1: $(PANDOC)
	@echo calling pandoc to generate manpage
	@$(PANDOC) man/clac.md -s -t man -o clac.1

