BBSHOME = /home/bbs
CGIPATH = /home/httpd/cgi-bin/bbs
HTMPATH = /home/httpd/html

CC      = gcc -Os
OBJS	= libweb.o crypt.o
LIBNAME	= libweb.a
CGI		= bbs0an bbsacount bbsall bbsanc bbsbfind bbsboa bbsbrdadd bbsccc \
		  bbsclear bbscloak bbscon bbsdel bbsdelf bbsdelmail bbsdelmsg \
		  bbsdoc bbsedit bbsfadd bbsfall bbsfdel bbsfdoc bbsfoot bbsfriend \
		  bbsfwd bbsgcon bbsgdoc bbsgetmsg bbsgfind bbsinfo bbsleft \
		  bbslogin bbslogout bbsmail bbsmailcon bbsmaildown bbsmailman \
		  bbsmailmsg bbsmain bbsman bbsmdoc bbsmnote bbsmsg bbsmybrd \
		  bbsmywww bbsnewmail bbsnick bbsnot bbsplan bbspreupload bbspst \
		  bbspstmail bbspwd bbsqry bbsrss bbssec bbssel bbssendmsg bbssig \
		  bbssnd bbssndmail bbstcon bbstdoc bbstfind bbstop10 bbstopb10 \
		  bbsufind bbsupload bbsuser bbsuserinfo bbsusr

all: libweb $(CGI)

libweb: $(OBJS)
	ar rs $(LIBNAME) $(OBJS)

install: all
	strip -s $(CGI)
	rm -rf $(CGIPATH)/*
	cp $(CGI) $(CGIPATH)
	chown -R bbs $(CGIPATH)
	chgrp -R bbs $(CGIPATH) 
	chmod +s $(CGIPATH)/*

clean:
	rm $(OBJS)
	rm $(LIBNAME)
	rm -rf $(CGI)

%:: %.c
	$(CC) -o $@ $< -lweb -L. -lBBS -L../lib -lfcgi
