BBSHOME = /home/bbs
CGIPATH = /home/httpd/fcgi
HTMPATH = /home/httpd/

CC      = gcc
CFILES  = libweb.c bbsmain.c bbsleft.c bbssec.c bbsfoot.c \
		  bbsall.c bbsboa.c bbslogin.c bbslogout.c bbsdoc.c bbscon.c \
		  bbspst.c bbssnd.c bbsqry.c bbsclear.c bbsupload.c bbspreupload.c \
		  bbs0an.c bbsanc.c bbsnot.c bbsmail.c bbsmailcon.c \
		  bbsdelmail.c bbstcon.c bbsmybrd.c bbserr.c bbsbrdadd.c bbsccc.c \
		  bbsfav.c bbspstmail.c bbssndmail.c
COBJS   = $(CFILES:.c=.o)
DEPS    = $(CFILES:.c=.d)
TARGET  = bbswebd
DEFS    = -D_POSIX_SOURCE -D_SVID_SOURCE
CFLAGS  = -Os -Wall -std=c99 -pedantic $(DEFS)

install: all
	strip -s $(TARGET)
	rm -rf $(CGIPATH)/*
	cp $(TARGET) $(CGIPATH)
	chown -R bbs:bbs $(CGIPATH)

new: install
	cp -r ../html $(HTMPATH)
	sh ./compress_xsl.sh $(HTMPATH)html/xsl

all: $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(TARGET) -lcrypt -lBBS -L../lib -lfcgi

clean:
	rm -f $(COBJS) $(DEPS) $(TARGET)

sinclude $(DEPS)

%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -M $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@
