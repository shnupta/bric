# Any automake files in these directories will get processed. Some
# common directories to put here are "man" and "etc"
SUBDIRS = po

# the docs listed in EXTRA_DIST will be installed to a "doc" when make
# install is run
EXTRA_DIST=		\
	README.md	\
	USAGE.md	\
	LICENSE		\
	INSTALL		\
	CONTRIBUTING.md	\
	TRANSLATE.md

packextrasdir=@docdir@
packextras_DATA=$(EXTRA_DIST)

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
sysconfdir = @sysconfdir@

bin_PROGRAMS=bric
AM_CFLAGS=-std=gnu99 -Wall -g -W -pedantic

bric_SOURCES =	src/bric.c		\
		src/locking.c	\
		src/tagstack.c	\
		src/handling.c

DEFS = -DLOCALEDIR=\"$(localedir)\" -DSYSCONFDIR=\"$(sysconfdir)\" @DEFS@
LIBS = @LIBINTL@ @LIBS@
bric_LDADD = @LIBINTL@ -lm

ACLOCAL_AMFLAGS = --install -I m4
