#! /bin/sh

CGIPATH= /usr/local/apache/cgi-bin/bbs
CGI     = bbsusr bbstop10 bbssec bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bb
s0an \
          bbslogout bbsleft bbslogin bbsqry bbsnot bbsfind \
          bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbsform bbspwd bbsplan bbsin
fo \
          bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bb
smail \
          bbsdelmsg bbssnd bbsalluser bbsnotepad bbsmsg bbssendmsg bbsreg \
          bbsmailmsg bbssndmail bbsnewmail bbspstmail bbsgetmsg bbssel bbscloak
\
          bbsmdoc bbsnick chatlogin bbsjs bbstfind bbsadl bbstcon bbstdoc \
          bbsdoreg bbsmywww bbsccc bbsufind bbsclear bbsstat bbsedit bbsman bbsp
arm \
          bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
          bbstopb10 bbsbfind bbstty bbsacount bbsuser bbschat bbsmaildown\
          bbsuserinfo bbsfind2 bbsmovie bbsmailman\

        rm -rf $(CGIPATH)/bbs
        cp $(CGI) $(CGIPATH)
        chmod +s $(CGIPATH)/*
        cp -R html/* $(HTMPATH)
        chown -R bbs $(CGIPATH)
        chgrp -R bbs $(CGIPATH)
