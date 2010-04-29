AM_CFLAGS = -O2 -pipe -std=c99 -Wall @ENABLE_DEBUG@
AM_CPPFLAGS = -D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS -I../include
noinst_LIBRARIES = libBBS.a
libBBS_a_SOURCES = board.c boardrc.c brdcache.c fileio.c goodbrd.c log.c \
				   mail.c mmap.c mmdecode.c modetype.c pass.c record.c post.c \
				   shm.c string.c stuffs.c sysconf.c ucache.c uinfo.c hash.c
