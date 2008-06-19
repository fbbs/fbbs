#include "BBSLIB.inc"

int main() {
	FILE *fp;
	char buf[512], path[512];
	init_all();
	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */
	printf("<b>查看消息 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	if(!loginok) http_fatal("匆匆过客无法查看消息, 请先登录");
	sethomefile(path, currentuser.userid, "msgfile.me");
	fp=fopen(path, "r");
	if(fp==0) http_fatal("没有任何消息");
	printf("<pre>\n");
	while(1) {
		if(fgets(buf, 256, fp)<=0) break;
		hprintf("%s", buf);
	}
	fclose(fp);
	printf("</pre>");
	printposttable_lite();
	printf("<br><a onclick='return confirm(\"您真的要清除所有消息吗?\")' href=bbsdelmsg>清除所有消息</a> ");
	printf("    <a href=bbsmailmsg>消息寄回信箱</a>");
	http_quit();
}
