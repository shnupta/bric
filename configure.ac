AC_INIT([bric], [m4_esyscmd_s([cat ./.version])], [noone@localhost], ,['https://github.com/shnupta/bric'])
AM_INIT_AUTOMAKE([subdir-objects])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])
AC_PROG_CC([cc])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_INSTALL

AM_GNU_GETTEXT([external], [need-formatstring-macros])
AM_GNU_GETTEXT_VERSION([0.19.8])

AC_CONFIG_FILES([       Makefile
                        src/Makefile
			po/Makefile.in])
AC_OUTPUT
