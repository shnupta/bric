all:
	$(CC) -g -o bric bric.c -Wall -W -pedantic -std=c99 -lm

install: all
	cp bric /usr/local/bin/bric


clean:
	rm bric
