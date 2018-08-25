bin_PROGRAMS=bric
AM_CFLAGS=-std=gnu99 -Wall -g -W -pedantic

bric_SOURCES =	bric.c		\
		locking.c	\
		tagstack.c	\
		handling.c

DEFS = -DLOCALEDIR=\"$(localedir)\" -DSYSCONFDIR=\"$(sysconfdir)\" @DEFS@
LIBS = @LIBINTL@ @LIBS@
bric_LDADD = @LIBINTL@ -lm
