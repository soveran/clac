FLAGS = -I./deps/linenoise -I./deps/sds
PREFIX?=/usr/local
MANPREFIX?=${PREFIX}/share/man
STRIP?=strip

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
	@echo stripping executable
	@${STRIP} clac
	@echo installing executable to ${PREFIX}/bin
	@mkdir -p ${PREFIX}/bin
	@cp -f clac ${PREFIX}/bin/clac
	@chmod 755 ${PREFIX}/bin/clac

	@echo installing manual pages to ${MANPREFIX}/man1
	@mkdir -p ${MANPREFIX}/man1
	@cp -f clac.1 ${MANPREFIX}/man1/clac.1
	@chmod 644 ${MANPREFIX}/man1/clac.1

uninstall:
	@echo removing executable from ${PREFIX}/bin
	@rm ${PREFIX}/bin/clac
	@echo removing manual pages from ${MANPREFIX}/man1
	@rm ${MANPREFIX}/man1/clac.1

test:
	@sh test/tests.sh

.PHONY: clean install uninstall test
