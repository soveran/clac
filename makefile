FLAGS = -I./deps/linenoise -I./deps/sds -lm
PREFIX?=/usr/local

default: clac

deps/linenoise/linenoise.o:
	@cd deps/linenoise && $(MAKE)

deps/sds/sds.o:
	@cd deps/sds && $(MAKE)

clac: clac.c deps/linenoise/linenoise.o deps/sds/sds.o
	$(CC) $(FLAGS) -Wall -Os -o clac clac.c deps/linenoise/linenoise.o deps/sds/sds.o

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

uninstall:
	@echo removing executable from ${PREFIX}/bin
	@rm ${PREFIX}/bin/clac
