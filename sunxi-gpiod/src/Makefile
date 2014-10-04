CC=gcc
AR=ar
RANLIB=ranlib

WFLAGS = -Wall -Werror -Wno-format
CFLAGS?=-O2
CPPFLAGS=
LDFLAGS=
LIBS= -lm

DESTDIR =
PREFIX = /usr

all: gpiod

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $<

gpiod: gpiod.o gpio_lib.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 gpiod $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f gpiod *.o

