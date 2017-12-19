
SRC=dwmstatus.c config.h datetime.h mailmonitor.h \
net_if.h ram.h timer.h weather.h

BIN=dwmstatus

# xml and curl compiler flags
CFLAGS=`xml2-config --cflags` `curl-config --cflags` `pkg-config x11 --cflags`

# xml/curl/X11 linker flags, optimization level, remove symbol table
LDFLAGS=`xml2-config --libs` `curl-config --libs` `pkg-config x11 --libs` -O2 -s

all: ${BIN}

config.h: 
	cp config.def.h $@

${BIN}.o: ${SRC}

${BIN}: ${BIN}.o

install: all
	install -m 755 ${BIN} ${DESTDIR}${PREFIX}/bin

.PHONY: all install
