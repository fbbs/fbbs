#include "BBSLIB.inc"

int main() {
	char *cmd;
	char buf[256];
	char s1[256]="", s2[256]="", s3[356]="";
	int r;
	init_all();
	cmd=getparm("cmd");
	r=sscanf(cmd, "%80s %80s %80s", s1, s2, s3);
	if(!strcasecmp(s1, "/q")) {
		sprintf(buf, "/cgi-bin/bbs/bbsqry?userid=%s", s2);
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/s")) {
		sprintf(buf, "/cgi-bin/bbs/bbssendmsg?destid=%s&msg=%s", s2, s3);
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/m")) {
		sprintf(buf, "/cgi-bin/bbs/bbspstmail?userid=%s&title=%s", s2, s3);
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/logout")) {
		printf("<script>top.location='/cgi-bin/bbs/bbslogout'</script>");
		http_quit();
	}
	if(!strcasecmp(s1, "/top10")) {
		sprintf(buf, "/cgi-bin/bbs/bbstop10");
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/g")) {
		sprintf(buf, "/cgi-bin/bbs/bbssel?board=%s", s2);
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/l")) {
		sprintf(buf, "/cgi-bin/bbs/bbsusr");
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/f")) {
		sprintf(buf, "/cgi-bin/bbs/bbsfind");
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/z")) {
		if(s2[0]==0) {
			sprintf(buf, "/cgi-bin/bbs/bbsdict");
		} else {
			sprintf(buf, "/cgi-bin/bbs/bbsdict?type=1&word=%s", s2);
		}
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/i")) {
		if(s2[0]==0) {
			sprintf(buf, "/cgi-bin/bbs/bbsfreeip");
		} else {
			sprintf(buf, "/cgi-bin/bbs/bbsfreeip?ip=%s&type=1", s2);
		}
		redirect(buf);
		http_quit();
	}
	if(!strcasecmp(s1, "/h")) {
		printf("<pre>\n");
		printf("超级控制台命令<hr color=green>\n");
		printf("/q [userid]           查询网友\n");
		printf("/s [userid] [msg]     发送讯息\n");
		printf("/m [userid] [title]   发送信件\n");
		printf("/logout               注销登录\n");
		printf("/top10                查看10大\n");
		printf("/g [board]            跳转到版面\n");
		printf("/l                    环顾四方\n");
		printf("/f                    查询文章\n");
		printf("/z [word]             查中英文字典\n");
		printf("/i [ip]               IP地址查询\n");
		printf("/h                    显示本帮助界面\n");
		printf("http://xx.xx.xx       跳转到url地址\n\n");
		printf("</pre>\n");
		http_quit();
	}
	if(!strncasecmp(s1, "http://", 7)) {
		sprintf(buf, "<script>window.open('%s','','');history.go(-1)</script>", nohtml(s1));
		printf("%s", buf);
		http_quit();
	}
	http_fatal("未知命令");
}
