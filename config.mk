VERSION = 2.0

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

INCS = `pkg-config --cflags loft pangocairo`
LIBS = `pkg-config --libs x11 loft pangocairo` -lpthread ${XINERAMALIBS}

CPPFLAGS = -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -g -std=c99 -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

CC = cc
