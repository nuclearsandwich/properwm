# properwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = properwm.c
OBJ = ${SRC:.c=.o}

all: options properwm

options:
	@echo properwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

properwm: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f properwm ${OBJ} properwm-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p properwm-${VERSION}
	@cp -R LICENSE Makefile README config.h config.mk \
		properwm.1 ${SRC} properwm-${VERSION}
	@tar -cf properwm-${VERSION}.tar properwm-${VERSION}
	@gzip properwm-${VERSION}.tar
	@rm -rf properwm-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f properwm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/properwm
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < properwm.1 > ${DESTDIR}${MANPREFIX}/man1/properwm.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/properwm.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/properwm
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/properwm.1

.PHONY: all options clean dist install uninstall
