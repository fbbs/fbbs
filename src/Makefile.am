AM_CFLAGS = -O2 -pipe -std=c99 -pedantic -Wall @ENABLE_DEBUG@
AM_CPPFLAGS = -D_XOPEN_SOURCE=500 -I../include
LDADD = ../lib/libfbbs.a

bin_PROGRAMS = bbsd cached

bbsd_SOURCES = bbsd.c sysconf.c termio.c screen.c main.c stuff.c more.c

cached_SOURCES = cached.c ucache.c
