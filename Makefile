CC=gcc
CFLAGS=-Wall -g -W -pedantic -std=gnu99 -lm
DEPENDENCIES=src/locking.o

all: $(DEPENDENCIES)
	$(CC) modules/syntax/bash/bash.c \
	modules/syntax/brain/brain.c \
	modules/syntax/ccpp/ccpp.c \
	modules/syntax/csharp/csharp.c \
	modules/syntax/d/d.c \
	modules/syntax/go/go.c \
	modules/syntax/html/html.c \
	modules/syntax/java/java.c \
	modules/syntax/javascript/javascript.c \
	modules/syntax/makefile/makefile.c \
	modules/syntax/pascal/pascal.c \
	modules/syntax/php/php.c \
	modules/syntax/python/python.c \
	modules/syntax/ruby/ruby.c \
	modules/syntax/rust/rust.c \
	modules/syntax/sql/sql.c \
	modules/syntax/syntax.c \
	bric.c tagstack.c src/llth.c src/synthigh.c \
	src/erow_func.c src/termupd.c src/findmode.c \
	$(DEPENDENCIES) $(CFLAGS) -o bric

install: all
	cp bric /usr/local/bin/bric

clean:
	rm -f bric
	find src/ -type f -iname "*.o" -exec rm {} +

.c.o:
	$(CC) -c $< -o $@
