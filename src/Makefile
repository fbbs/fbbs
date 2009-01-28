#############################################################
####   Setup the following site-specific information     ####
#############################################################

# The home directory for user 'bbs'
BBSHOME=$(BBS_HOME)

# The uid/gid info for user 'bbs' and 'bbsadm'
BBSUID=$(BBS_UID)
BBSGRP=$(BBSGID)

OS_DEF   = $(OS_DEF)
CC       = gcc
CFLAGS   = $(CFLAGS)
LIBS     = $(LIBS)

INSTALL  = $(INSTALL)

CSIE_DEF = $(CSIE_DEF)

#############################################################
###        You needn't modify the following stuff      ######
#############################################################
PROGNAME = bbsd
CFILE	 = admintool.c announce.c bbs.c bbsd.c bcache.c bm.c\
	   boards.c chat.c comm_lists.c delete.c edit.c fileshm.c goodbye.c \
	   help.c io.c list.c mail.c main.c maintain.c more.c \
	   namecomplete.c postheader.c read.c \
	   register.c screen.c sendmsg.c stuff.c talk.c term.c \
	   userinfo.c vote.c xyz.c readabc.c five.c

COBJS	 = admintool.o announce.o bbs.o bbsd.o bcache.o bm.o\
	   boards.o chat.o comm_lists.o delete.o edit.o fileshm.o goodbye.o \
	   help.o io.o list.o mail.o main.o maintain.o more.o \
	   namecomplete.o postheader.o read.o \
	   register.o screen.o sendmsg.o stuff.o talk.o term.o \
	   userinfo.o vote.o xyz.o readabc.o five.o 

COBJS1	 = admintool.o announce.o bbs.o bcache.o bm.o\
	   boards.o chat.o comm_lists.o delete.o edit.o fileshm.o goodbye.o \
	   help.o io.o list.o mail.o main.o maintain.o more.o \
	   namecomplete.o postheader.o read.o \
	   register.o screen.o sendmsg.o stuff.o talk.o term.o \
	   userinfo.o vote.o xyz.o readabc.o five.o

GARBAGE  = a.out core installchatd chatd thread *~ *.BAK miscd

SO	 = admintool.so

# The -DINVISIBLE makes bbs users invisible to 'finger' and 'who'.
# Simply remove it if you don't want this feature.

DEFINES	=  $(CSIE_DEF) $(OS_DEF)
EXTRADEF = -DBACKUP_RANGE_DELETE

.SUFFIXES: .o .c

.c.o:	;$(CC) $(CFLAGS) $(DEFINES) $(EXTRADEF) -c $*.c

#--------------------- Dependency starts here -------------------
all:  $(PROGNAME) chatd thread bbsnet so miscd

so: admintool.so

admintool.so: admintool.c
	$(CC) $(CFLAGS) $(OS_DEF) $(EXTRADEF) -c admintool.c
	ld -s -L../lib -lBBS -G admintool.o -o admintool.so

installso: admintool.so
	echo "install .so packages .."
	$(INSTALL) -m 550 -g $(BBSGRP) -o $(BBSUID) admintool.so $(BBSHOME)/so


$(PROGNAME): $(COBJS)
	$(CC) -o $(PROGNAME) $(CFLAGS) $(CSIE_DEF) $(COBJS) $(LIBS) $(EXTRADEF) -L../lib -lBBS
	@echo "Program size: `cat $(PROGNAME)|wc -c` bytes"

chatd: station.c
	$(CC) $(CFLAGS) -o chatd $(OS_DEF) station.c $(LIBS) -L../lib -lBBS

miscd: miscd.c $(COBJS1)
	$(CC) $(CFLAGS) -o miscd $(OS_DEF) miscd.c $(COBJS1) $(LIBS) -L../lib -lBBS

thread: thread.c ../lib/record.c
	$(CC) $(CFLAGS) -o thread $(OS_DEF) thread.c ../lib/record.c $(LIBS) -DTHREAD_C -L../lib -lBBS

install:
	sh Install.sh

installbbs: bbsd
	$(INSTALL) -m 550 -g $(BBSGRP) -o $(BBSUID) bbsd $(BBSHOME)/bin/bbsd.new
#	-cp bbsd $(BBSHOME)/bin/bbsd.new
#	-rm -f $(BBSHOME)/bin/bbsd.old
#	-mv $(BBSHOME)/bin/bbsd $(BBSHOME)/bin/bbsd.old
	-rm -f $(BBSHOME)/bin/bbsd
	mv $(BBSHOME)/bin/bbsd.new $(BBSHOME)/bin/bbsd

installmiscd: miscd
	$(INSTALL) -m 550 -g $(BBSGRP) -o $(BBSUID) miscd $(BBSHOME)/bin/miscd.new
	-rm -f $(BBSHOME)/bin/miscd.old
	-mv $(BBSHOME)/bin/miscd $(BBSHOME)/bin/miscd.old
	mv $(BBSHOME)/bin/miscd.new $(BBSHOME)/bin/miscd

installbbsnet: bbsnet
	$(INSTALL) -m 550 -g $(BBSGRP) -o $(BBSUID) bbsnet $(BBSHOME)/bin/bbsnet.new
#	-rm -f $(BBSHOME)/bin/bbsnet.old
#	-mv $(BBSHOME)/bin/bbsnet $(BBSHOME)/bin/bbsnet.old
	-rm -f $(BBSHOME)/bin/bbsnet
	mv $(BBSHOME)/bin/bbsnet.new $(BBSHOME)/bin/bbsnet

installthread: thread
	$(INSTALL) -m 550 -g $(BBSGRP) -o $(BBSUID) thread $(BBSHOME)/bin/thread.new
#	-rm -f $(BBSHOME)/bin/thread.old
#	-mv $(BBSHOME)/bin/thread $(BBSHOME)/bin/thread.olda
	-rm -f $(BBSHOME)/bin/thread
	mv $(BBSHOME)/bin/thread.new $(BBSHOME)/bin/thread

installchatd: chatd
	$(INSTALL) -m 550 -g $(BBSGRP) -o $(BBSUID) chatd $(BBSHOME)/bin/chatd.new
#	-rm -f $(BBSHOME)/bin/chatd.old
#	-mv $(BBSHOME)/bin/chatd $(BBSHOME)/bin/chatd.old
	-rm -f $(BBSHOME)/bin/chatd
	mv $(BBSHOME)/bin/chatd.new $(BBSHOME)/bin/chatd

clean: /tmp
	-rm -fr $(GARBAGE) $(SO) $(COBJS) $(PROGNAME) $(LNFILES)
	-rm -fr bbsnet 
	-rm -fr admintool.o $(GAMEOBJS)

cleanall: clean
	-rm -f Install.sh
	-rm -f ../include/config.h
	-rm -f ../include/chat.h
	-rm -f bbs chatd thread

tags: /tmp
	ctags $(CFILE)

update: installbbs installchatd installthread installbbsnet installso installmiscd

bbsnet: bbsnet.c
	$(CC) ${CFLAGS} ${LIBS} bbsnet.c -o bbsnet

# DO NOT DELETE
