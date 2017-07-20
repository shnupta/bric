all:
	$(CC) -o bric bric.c -Wall -W -pedantic -std=gnu99

install: all
	cp bric /usr/local/bin/bric


clean:
	rm bric
