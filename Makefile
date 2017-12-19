
SRC=dwmstatus-bram.c config.h datetime.h mailmonitor.h \
net_if.h ram.h timer.h weather.h

# xml and curl compiler flags
CFLAGS=`xml2-config --cflags` `curl-config --cflags` `pkg-config x11 --cflags`

# xml/curl/X11 linker flags, optimization level, remove symbol table
LDFLAGS=`xml2-config --libs` `curl-config --libs` `pkg-config x11 --libs` -O2 -s

all: dwmstatus-bram

config.h: 
	cp config.def.h $@

dwmstatus-bram.o: ${SRC}

dwmstatus-bram: dwmstatus-bram.o

install: all
	install -m 755 dwmstatus-bram ${DESTDIR}${PREFIX}/bin

.PHONY: all install
