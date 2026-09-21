.POSIX:
.PHONY: all install uninstall clean

VERSION = 0.1

PREFIX = /usr/local
CC = cc
CFLAGS = -Wall -Wextra -DVERSION=\"$(VERSION)\"

all: bf
install: all
	cp -f bf $(DESTDIR)/$(PREFIX)/bin
uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/bf
clean:
	rm -f bf bf.core bf.o a.out
