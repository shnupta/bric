# Any automake files in these directories will get processed. Some
# common directories to put here are "man" and "etc"
SUBDIRS = po

# the docs listed in EXTRA_DIST will be installed to a "doc" when make
# install is run
EXTRA_DIST=	README.md USAGE.md LICENSE INSTALL CONTRIBUTING.md \
	TRANSLATE.md AUTHORS

packextrasdir=@docdir@
packextras_DATA=$(EXTRA_DIST)

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
sysconfdir = @sysconfdir@

bin_PROGRAMS=bric

# std=gnu99 is used to prevent warnings related to use of getline()
AM_CFLAGS=-std=gnu99 -Wall -g

bric_SOURCES =	\
		src/bric.c	\
		src/locking.c	\
		src/tagstack.c	\
		src/handling.c	\
		src/gettext.h

# syntax lang headers
SYNTAX_LANG_H = \
	src/modules/syntax/brain.h	\
	src/modules/syntax/d.h \
	src/modules/syntax/go.h \
	src/modules/syntax/java.h	\
	src/modules/syntax/pascal.h	\
	src/modules/syntax/ruby.h	\
	src/modules/syntax/swift.h	\
	src/modules/syntax/ccpp.h	\
	src/modules/syntax/dockerfile.h	\
	src/modules/syntax/haxe.h	\
	src/modules/syntax/javascript.h	\
	src/modules/syntax/php.h	\
	src/modules/syntax/rust.h	\
	src/modules/syntax/bash.h	\
	src/modules/syntax/csharp.h	\
	src/modules/syntax/html.h	\
	src/modules/syntax/makefile.h	\
	src/modules/syntax/python.h	\
	src/modules/syntax/sql.h

noinst_HEADERS =	\
	$(SYNTAX_LANG_H)	\
	src/bric.h	\
	src/editor.h	\
	src/handling.h	\
	src/locking.h	\
 	src/modules/syntax/editor_syntax.h	\
 	src/modules/syntax/syntax.h	\
 	src/modules/tag/tagfuncs.h	\
 	src/modules/tag/tagstack.h

DEFS = -DLOCALEDIR=\"$(localedir)\" -DSYSCONFDIR=\"$(sysconfdir)\" @DEFS@
LIBS = @LIBINTL@ @LIBS@
bric_LDADD = @LIBINTL@ -lm

ACLOCAL_AMFLAGS = --install -I m4
