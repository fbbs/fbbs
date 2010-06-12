#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(fbbs, 0.1, http://code.google.com/p/fbbs/issues)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([include/config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AM_PROG_CC_C_O

CFLAGS=

AC_PREFIX_DEFAULT(/home/bbs)

AC_ARG_ENABLE(www, [  --enable-www            compile www modules],
			  [if test "$enableval" = "yes" ; then ENABLE_WWW=www; fi], [ENABLE_WWW= ])
AC_SUBST(ENABLE_WWW)

AC_ARG_ENABLE(debug, [  --enable-debug          compile debug version],
			  [if test "$enableval" = "no" ; then ENABLE_DEBUG=""; fi], [ENABLE_DEBUG="-g -O0" ])
AC_SUBST(ENABLE_DEBUG)

AC_ARG_ENABLE(ssh, [  --enable-ssh            compile ssh module],
			  [if test "$enableval" = "yes" ; then ENABLE_SSH="sshbbsd"; ssh="true"; fi], [ENABLE_SSH= ;ssh="false"])
AC_SUBST(ENABLE_SSH)
AM_CONDITIONAL([SSH], [test x$ssh = xtrue])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile])
AC_OUTPUT
