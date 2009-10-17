AM_CFLAGS = @ENABLE_DEBUG@ -O2 -Wall -std=c99 -pedantic
AM_CPPFLAGS = -D_POSIX_SOURCE -D_SVID_SOURCE -I../include
LIBS = -lcrypt -lfcgi
LDADD = ../lib/libBBS.a

bin_PROGRAMS = bbswebd
bbswebd_SOURCES = libweb.c bbsmain.c bbssec.c bbsall.c bbsboa.c bbslogin.c \
				  bbslogout.c bbsdoc.c bbscon.c bbspst.c bbssnd.c bbsqry.c \
				  bbsclear.c bbsupload.c bbspreupload.c bbs0an.c bbsanc.c \
				  bbsnot.c bbsmail.c bbsmailcon.c bbsdelmail.c bbstcon.c \
				  bbsmybrd.c bbserr.c bbsbrdadd.c bbsccc.c bbsfav.c \
				  bbspstmail.c bbssndmail.c bbsfall.c bbsplan.c bbsdel.c \
				  bbsfwd.c bbsinfo.c
