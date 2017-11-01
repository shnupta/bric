all:

	$(CC) -g -o bric bric.c tagstack.c -Wall -W -pedantic -std=gnu99 -lm

install: all
	cp bric /usr/local/bin/bric

clean:
	rm -f bric
