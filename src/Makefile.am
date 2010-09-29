AM_CFLAGS = -O2 -pipe -std=c99 -pedantic -Wall -Wno-implicit -export @ENABLE_DEBUG@
AM_CPPFLAGS = -D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS -DSHOW_IDLE_TIME -DDLM -DBACKUP_RANGE_DELETE -I../include
LIBS = -lcrypt -ldl -lBBS
AM_LDFLAGS = -L../lib

bin_PROGRAMS = bbsd miscd @ENABLE_SSH@ chatd thread
EXTRA_PROGRAMS = bbsd miscd sshbbsd chatd thread

noinst_LIBRARIES = libfbcmn.a
libfbcmn_a_SOURCES = announce.c bbs.c bcache.c bm.c boards.c chat.c \
	comm_lists.c delete.c deny.c edit.c fileshm.c goodbye.c help.c list.c \
	mail.c maintain.c more.c namecomplete.c postheader.c read.c register.c \
	screen.c sendmsg.c stuff.c talk.c userinfo.c vote.c xyz.c readabc.c \
	five.c convert.c bbsnet.c users.c

bbsd_SOURCES = bbsd.c io.c main.c
bbsd_LDADD = ./libfbcmn.a

if SSH
sshbbsd_SOURCES = bbsd.c io.c main.c
sshbbsd_LDADD = $(bbsd_LDADD) -lfbssh -L../libssh
sshbbsd_CPPFLAGS = -DSSHBBS $(AM_CPPFLAGS)
endif

miscd_SOURCES = miscd.c io.c main.c
miscd_LDADD = $(bbsd_LDADD)

chatd_SOURCES = station.c
thread_SOURCES = thread.c

lib_LTLIBRARIES = admintool.la
admintool_la_SOURCES = admintool.c
admintool_la_LDFLAGS = -module -L../lib
admintool_la_CPPFLAGS = -D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS -DSHOW_IDLE_TIME -DBACKUP_RANGE_DELETE -I../include
libdir=${prefix}/so
