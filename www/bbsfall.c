#include "libweb.h"

int main() {
	int i; 
	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */

   	if(!loginok) http_fatal("您尚未登录, 请先登录");
	loadfriend(currentuser.userid);
    	printf("<center>\n");
  	printf("<b>好友名单 · %s [使用者: %s]</b><br>\n", BBSNAME, currentuser.userid);
   	printf("您共设定了 %d 位好友<br>", friendnum);
	
	printf("<table align=center border=0 cellpadding=0 cellspacing=0 width=400>\n");
	printf("	<tr height=6>\n");
	printf("		<td width=6><img border=0 src='/images/lt.gif'></td>\n");
	printf("		<td background='/images/t.gif' width=100%%></td>\n");
	printf("		<td width=6><img border=0 src='/images/rt.gif'></td>\n");
	printf("	</tr>\n");
	printf("	<tr  height=100%%>\n");
	printf("		<td width=6 background='/images/l.gif'>\n");
	printf("		<td width=100%%>\n");

   	printf("<table width=100%% border=0  bgcolor=#ffffff><tr class=pt9h ><td><font color=white>序号<td><font color=white>好友代号<td><font color=white>好友说明<td><font color=white>删除好友");
   	int cc=0;
	for(i=0; i<friendnum; i++) {
		printf("<tr class=%s><td>%d",((cc++)%2)?"pt9dc":"pt9lc" , i+1);
		printf("<td><a href=bbsqry?userid=%s>%s</a>", fff[i].id, fff[i].id);
		printf("<td>%s\n", nohtml(fff[i].exp));
		printf("<td>[<a onclick='return confirm(\"确实删除吗?\")' href=bbsfdel?userid=%s>删除</a>]", fff[i].id);
	}
   	printf("</table>\n");
	printposttable();
	printf("[<a href=bbsfadd>添加新的好友</a>]</center>\n");
	http_quit();
}
