#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(fdubbs, 2009, http://code.google.com/p/fdubbs/issues)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([include/config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([crypt], [crypt])
AC_CHECK_LIB([fcgi], [FCGI_Accept])
AC_CHECK_LIB([gcrypt], [gcry_cipher_encrypt])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([alarm atexit bzero dup2 ftruncate getaddrinfo getcwd gethostbyaddr gethostbyname gethostname gettimeofday inet_ntoa memchr memmove memset mkdir munmap poll rmdir select socket strcasecmp strchr strerror strncasecmp strpbrk strrchr strspn strstr strtol strtoul utime])

AC_CHECK_SIZEOF([time_t])

AM_PROG_CC_C_O

CFLAGS=

AC_PREFIX_DEFAULT(/home/bbs)

AC_ARG_ENABLE(www, [  --enable-www            compile www modules],
			  [if test "$enableval" = "yes" ; then ENABLE_WWW=www; fi], [ENABLE_WWW= ])
AC_SUBST(ENABLE_WWW)

AC_ARG_ENABLE(debug, [  --enable-debug          compile debug version],
			  [if test "$enableval" = "yes" ; then ENABLE_DEBUG="-g -O0"; fi], [ENABLE_DEBUG= ])
AC_SUBST(ENABLE_DEBUG)

AC_ARG_ENABLE(ssh, [  --enable-ssh            compile ssh module],
			  [if test "$enableval" = "yes" ; then ENABLE_SSH="sshbbsd"; ssh="true"; fi], [ENABLE_SSH= ;ssh="false"])
AC_SUBST(ENABLE_SSH)
AM_CONDITIONAL([SSH], [test x$ssh = xtrue])

AC_CONFIG_FILES([Makefile lib/Makefile libssh/Makefile src/Makefile www/Makefile
                 util/local_utl/Makefile])
AC_OUTPUT
