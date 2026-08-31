.POSIX:
.PHONY: all install uninstall clean

VERSION = 0.1

PREFIX = /usr/local
CC = cc
CFLAGS = -DVERSION=\"$(VERSION)\"
LDLIBS = -lm

all: bf
install: bf
	cp -f bf $(DESTDIR)/$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/bf

bf: bf.c

clean:
	rm -f bf bf.core bf.o a.out
