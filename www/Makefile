BBSHOME = /home/bbs
CGIPATH = /home/httpd/fcgi
HTMPATH = /home/httpd/html

CC      = gcc
COBJS   = libweb.o bbsmain.o bbsleft.o bbssec.o bbsfoot.o bbsgetmsg.o \
		  bbsall.o bbsboa.o bbslogin.o bbslogout.o bbsdoc.o bbscon.o \
		  bbspst.o bbssnd.o bbsqry.o bbsclear.o bbsupload.o bbspreupload.o \
		  bbs0an.o bbsanc.o bbsnot.o bbssendmsg.o bbsmail.o
TARGET  = bbswebd
DEFS    = -D_POSIX_SOURCE -D_SVID_SOURCE
CFLAGS  = -Os -Wall -std=c99 -pedantic $(DEFS)

install: all
	strip -s $(TARGET)
	rm -rf $(CGIPATH)/*
	cp $(TARGET) $(CGIPATH)
	chown -R bbs $(CGIPATH)
	chgrp -R bbs $(CGIPATH) 
	chmod +s $(CGIPATH)/*

all: $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(TARGET) -lcrypt -lBBS -L../lib -lfcgi

clean:
	rm $(COBJS)
	rm -rf $(TARGET)
