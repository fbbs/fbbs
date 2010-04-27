AM_CFLAGS = -O2 -pipe -Wall -Wno-implicit -export @ENABLE_DEBUG@
AM_CPPFLAGS = -D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS -DSHOW_IDLE_TIME -DDLM -DBACKUP_RANGE_DELETE -I../include
LIBS = -lcrypt -ldl -lgd
LDADD = ../lib/libBBS.a


bin_PROGRAMS = bbsd miscd @ENABLE_SSH@ chatd thread
EXTRA_PROGRAMS = bbsd miscd sshbbsd chatd thread

bbsd_SOURCES = bbsd.c io.c main.c
bbsd_LDADD = $(LDADD) announce.$(OBJEXT) bbs.$(OBJEXT) bcache.$(OBJEXT) \
             bm.$(OBJEXT) boards.$(OBJEXT) chat.$(OBJEXT) \
			 comm_lists.$(OBJEXT) delete.$(OBJEXT) deny.$(OBJEXT) \
			 edit.$(OBJEXT) fileshm.$(OBJEXT) goodbye.$(OBJEXT) \
			 help.$(OBJEXT) list.$(OBJEXT) mail.$(OBJEXT) \
			 maintain.$(OBJEXT) more.$(OBJEXT) namecomplete.$(OBJEXT) \
			 postheader.$(OBJEXT) read.$(OBJEXT) register.$(OBJEXT) \
			 screen.$(OBJEXT) sendmsg.$(OBJEXT) stuff.$(OBJEXT) \
			 talk.$(OBJEXT) userinfo.$(OBJEXT) vote.$(OBJEXT) xyz.$(OBJEXT) \
			 readabc.$(OBJEXT) five.$(OBJEXT) convert.$(OBJEXT) \
			 bbsnet.$(OBJEXT) users.$(OBJEXT)

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
