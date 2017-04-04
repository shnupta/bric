all:
	$(CC) -o bric bric.c -Wall -W -pedantic -std=c99

install: all
	cp bric /usr/local/bin/bric


clean:
	rm bric
