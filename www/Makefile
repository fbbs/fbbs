BBSHOME = /home/bbs
CGIPATH = /home/httpd/fcgi
HTMPATH = /home/httpd/html

CC      = gcc -Os
OBJS	= libweb.o crypt.o
LIBNAME	= libweb.a
CFILES  = bbsmain.c bbsleft.c bbssec.c
TARGET  = bbswebd
CFLAGS  = -Wall

all: libweb
	$(CC) $(CFLAGS) $(CFILES) -o $(TARGET) -lweb -L. -lBBS -L../lib -lfcgi

libweb: $(OBJS)
	ar rs $(LIBNAME) $(OBJS)

install: all
	strip -s $(TARGET)
	rm -rf $(CGIPATH)/*
	cp $(TARGET) $(CGIPATH)
	chown -R bbs $(CGIPATH)
	chgrp -R bbs $(CGIPATH) 
	chmod +s $(CGIPATH)/*

clean:
	rm $(OBJS)
	rm $(LIBNAME)
	rm -rf $(TARGET)

%:: %.c
	$(CC) $(CFLAGS) $(CFILES) -o $@ $< -lweb -L. -lBBS -L../lib -lfcgi
