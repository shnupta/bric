EXTRA_DIST=         \
	README.md         	\
	LICENSE

bin_PROGRAMS=bric
AM_CFLAGS= -std=gnu99 -Wall -g -W -pedantic -lm

bric_SOURCES = 	bric.c					\
								src/locking.c 	\
								tagstack.c

# bric_LDFLAGS = 

ACLOCAL_AMFLAGS = -I m4

PACKAGE = @PACKAGE@
VERSION = @VERSION@

ACLOCAL = true
AUTOCONF = true
AUTOHEADER = true
AUTOMAKE = true

distdir = $(PACKAGE)-$(VERSION)
