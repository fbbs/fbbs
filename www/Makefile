BBSHOME = /home/bbs
CGIPATH = /home/httpd/cgi-bin/bbs
HTMPATH = /home/httpd/html

#CC 	= gcc -O2 -L/usr/ucblib -lucb -lsocket -lnsl 
CC      = gcc -Os libweb.c crypt.c
CGI	= bbsusr bbstop10 bbssec bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bbs0an \
	  bbslogout bbslogin bbsqry bbsnot bbsfind bbsmain bbsleft\
	  bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbsform bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsalluser bbsnotepad bbsmsg bbssendmsg bbsreg \
	  bbsmailmsg bbsnewmail bbsgetmsg bbssel bbscloak bbspstmail bbssndmail\
	  bbsmdoc bbsnick chatlogin bbsjs bbstfind bbsadl bbstcon bbstdoc \
	  bbsmywww bbsccc bbsufind bbsclear bbsstat bbsedit bbsman bbsparm \
	  bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
	  bbstopb10 bbsbfind bbstty bbsacount bbsuser bbschat bbsmaildown\
	  bbsuserinfo bbsmovie bbsmailman bbsgfind\
	  bbspostupload bbsdelf bbsfdoc bbspreupload bbsupload bbsrss
all: $(CGI)

install: all
	strip -s $(CGI)
	rm -rf $(CGIPATH)/*
	cp $(CGI) $(CGIPATH)
	chown -R bbs $(CGIPATH)
	chgrp -R bbs $(CGIPATH) 
	chmod +s $(CGIPATH)/*
clean:
	rm -rf $(CGI)
