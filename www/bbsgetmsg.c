#include "libweb.h"

int main() {
	static char buf[256], buf2[256]=".";
	char toid[13];
	int topid;
        init_all();
/*   added by roly, patch of NJU0.9  */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
/*   added end */	
	printf("<style type=text/css>\n");
        printf("A {color: #000000}\n");
        printf("</style>\n");
	printf("<body marginwidth=0 marginheight=0>");
	if(loginok==0) {
		http_quit();
	}
	sethomefile(buf, currentuser.userid, "wwwmsg");
	if(file_size(buf)>0) {
		int total;
		char *p;
		printf("<bgsound src=/msg.wav>\n");
		printf("<body onkeypress='checkrmsg(event.keyCode)' style='BACKGROUND-COLOR: #fffffff; font-weight:bold;'>");
		total=file_size(buf)/129;
		get_record(buf2, 129, 0, buf);
		del_record(buf, 129, 0);
		printf("<table width=\"100%%\">\n");
		printf("<tr><td>");
		buf2[111]=0;
		//hprintf(buf2);
		sscanf(buf2+12, "%s", toid);
		sscanf(buf2+122, "%d", &topid);
		p=strstr(buf2,"):");
		if(!p)p=buf2+23;
		else p+=2;
		printf("<b>%s</b>:",toid);
		hprintf(p);
		printf("<td align=right><a target=view href=bbssendmsg?destid=%s&destpid=%d>[»ØÑ¶Ï¢]</a> <a href=bbsgetmsg>[ºöÂÔ]</a>", toid, topid);
		http_quit();
	}
	refreshto("bbsgetmsg", 60);
}
