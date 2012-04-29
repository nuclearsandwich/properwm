# properwm version
VERSION = 6.0

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = -I${X11INC}
LIBS = -L${X11LIB} -lX11 -lpthread ${XINERAMALIBS} `pkg-config --libs loft strext`

# flags
CPPFLAGS = -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS   = -g -std=c99 -Wall -Os ${INCS} ${CPPFLAGS} `pkg-config --cflags loft strext`
LDFLAGS  = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
