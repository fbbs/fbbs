# Generated automatically from Makefile.in by configure.

# Makefile for Firebird BBS 3.0 SM series
# deliver from SEEDNet BBS generation 1
# $Id: Makefile 308 2006-10-16 15:13:10Z danielfree $
#################################################################
# Makefile for SEEDNet BBS v1.0
# Id: Makefile.in,v 1.11 1999/03/23 13:30:23 edwardc Exp 
#################################################################
# 1. This package is derived from Firebird BBS 1.5, which was 
#    developed by the Department of Computer Science & Information
#    Engineering, Chung Chen University of Taiwan, ROC.
#    and under Firebird 2.66M the package is developed by the 
#    Board of System Administrators, Malaysian Students' Association In Taiwan.
#
# 2. The example site is bbs.seed.net.tw (139.175.252.15)
#
# 3. The developers of this package can be reached by
#    SYSOP.bbs@bbs.seed.net.tw
#
# 4. This Makefile is for "LINUX"
#
# This Makefile is generated automatic by configure

#############################################################
####   Setup the following site-specific information     ####
#############################################################

# The home directory and uid/gid info for user 'bbs'
BBSHOME  = /home/bbs
BBSUID   = bbs
BBSGRP   = bbs

OS_DEF   = -DLINUX -DTERMIOS
CC       = gcc-3.3.6
CFLAGS   = -Os -pipe -fomit-frame-pointer -export -Wunused -I../include
#for main host
#CFLAGS   = -Os -DFDQUAN -export -Wunused -I../include   #for  water host
#CFLAGS   =-march=pentium3 -O3 -pipe -fomit-frame-pointer -export -Wunused -I../include
#LIBS     = -ltermcap -ldl -lrpcsvc -lbsd -lnsl -lcrypt
LIBS	 = -lncurses -lcrypt -ldl -lgd
#-ldmalloc

INSTALL  = /usr/bin/install -c 

CSIE_DEF = -DSHOW_IDLE_TIME -DWITHOUT_CHROOT -DDLM -DNO_IO
#-DDMALLOC

MAKEFLAG = CC=$(CC) BBSHOME="$(BBSHOME)" OS_DEF="$(OS_DEF)" CFLAGS="$(CFLAGS)" LIBS="$(LIBS)" INSTALL="$(INSTALL)" CSIE_DEF="$(CSIE_DEF)" BBS_UID=$(BBSUID) BBSGID=$(BBSGRP)

DIRS	 = lib src

# You needn't change the make rule, if you can't understand what are you doing

all:
	@echo "== Making main program and relatve src =="
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; make $(MAKEFLAG) all ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done
	echo "done."

install: all
	@echo "== Installing Everything ... =="
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; make install ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done

update: all
	@echo "== update binary ... =="
	@echo "-->src/"
	(cd src; make $(MAKEFLAG) update; cd ..)
	@echo "<--src/"

clean:
	@echo "== cleanning junks, object files, and binaries .. =="
	-rm -fr *~ *.o *.BAK
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; make clean ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done
	echo "done."

cleanall: clean
	@echo "== cleanning all objfile, binaries .. =="
	-rm -fr *~ *.o *.BAK
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; make cleanall ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done
