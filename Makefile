CC=gcc
CFLAGS=-Wall -g -W -pedantic -std=gnu99 -lm

DEPENDENCIES=src/locking.o

all: $(DEPENDENCIES)
	$(CC) bric.c $(DEPENDENCIES) $(CFLAGS) -o bric

install: all
	cp bric /usr/local/bin/bric

clean:
	rm -f bric
	find src/ -type f -iname "*.o" -exec rm {} +

.c.o:
	$(CC) -c $< -o $@
