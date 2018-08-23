# Any automake files in these directories will get processed. Some
# common directories to put here are "man" and "etc"
SUBDIRS = src

# the docs listed in EXTRA_DIST will be installed to a "doc" when make
# install is run
EXTRA_DIST=         \
	README.md         \
	USAGE.md					\
	LICENSE

packextrasdir=@docdir@
packextras_DATA=$(EXTRA_DIST)

ACLOCAL = true
AUTOCONF = true
AUTOHEADER = true
AUTOMAKE = true

ACLOCAL_AMFLAGS = --install -I m4

distdir = $(PACKAGE)-$(VERSION)
