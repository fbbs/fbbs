AM_CFLAGS = -O2 -Wall -std=c99 -pedantic @ENABLE_DEBUG@
AM_CPPFLAGS = -D_XOPEN_SOURCE=600 -I../include
LIBS = -lcrypt -lfcgi
LDADD = ../lib/libBBS.a

bin_PROGRAMS = bbswebd
bbswebd_SOURCES = libweb.c bbsmain.c bbssec.c bbsall.c bbslogin.c \
				  bbslogout.c bbsdoc.c bbscon.c bbspst.c bbssnd.c bbsqry.c \
				  bbsclear.c bbsupload.c bbsann.c \
				  bbsnot.c bbsmail.c bbstcon.c \
				  bbsmybrd.c bbserr.c bbsfav.c \
				  bbsfall.c bbsplan.c bbsdel.c \
				  bbsinfo.c bbssel.c bbsrss.c bbstop10.c
