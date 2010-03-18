AM_CFLAGS = -O2 -pipe -Wall -Wno-implicit -export @ENABLE_DEBUG@
AM_CPPFLAGS = -DLINUX -DTERMIOS -DSHOW_IDLE_TIME -DDLM -DBACKUP_RANGE_DELETE -I../include
LIBS = -lcrypt -ldl -lgd
LDADD = ../lib/libBBS.a

bin_PROGRAMS = bbsd miscd chatd thread bbsnet
bbsd_SOURCES = announce.c bbs.c bbsd.c bcache.c bm.c boards.c chat.c \
			   comm_lists.c delete.c deny.c edit.c fileshm.c goodbye.c \
			   help.c io.c list.c mail.c main.c maintain.c more.c \
			   namecomplete.c postheader.c read.c register.c screen.c \
			   sendmsg.c stuff.c talk.c userinfo.c vote.c xyz.c \
			   readabc.c five.c convert.c
miscd_SOURCES = announce.c bbs.c bcache.c bm.c boards.c chat.c \
				comm_lists.c delete.c deny.c edit.c fileshm.c goodbye.c \
				help.c io.c list.c mail.c main.c maintain.c miscd.c more.c \
				namecomplete.c postheader.c read.c register.c screen.c \
				sendmsg.c stuff.c talk.c userinfo.c vote.c xyz.c \
				readabc.c convert.c
chatd_SOURCES = station.c
thread_SOURCES = thread.c
bbsnet_SOURCES = bbsnet.c

lib_LTLIBRARIES = admintool.la
admintool_la_SOURCES = admintool.c
admintool_la_LDFLAGS = -module -L../lib
admintool_la_CPPFLAGS = -DLINUX -DTERMIOS -DSHOW_IDLE_TIME -DBACKUP_RANGE_DELETE -I../include
libdir=${prefix}/so
