BBSHOME = /home/bbs
CGIPATH = /home/httpd/fcgi
HTMPATH = /home/httpd/html

CC      = gcc
OBJS	= libweb.o crypt.o
LIBNAME	= libweb.a
COBJS   = bbsmain.o bbsleft.o bbssec.o bbsfoot.o bbsgetmsg.o \
		  bbsall.o bbsboa.o
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

all: libweb $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(TARGET) -lweb -L. -lBBS -L../lib -lfcgi

libweb: $(OBJS)
	ar rs $(LIBNAME) $(OBJS)

clean:
	rm $(OBJS)
	rm $(COBJS)
	rm $(LIBNAME)
	rm -rf $(TARGET)
