#include "libweb.h"
FILE *fp;

int main() {
	FILE *fp;
	char *ptr, plan[256], buf[10000];
   	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */

	if(!loginok) http_fatal("匆匆过客不能设置说明档，请先登录");
	sprintf(plan, "home/%c/%s/plans", toupper(currentuser.userid[0]), currentuser.userid);
	if(!strcasecmp(getparm("type"), "update")) save_plan(plan);
	printf("<b><font style='font-size: 18pt'>%s</font> · %s 设置个人说明档</b> \n", currentuser.userid, BBSNAME);
	printf("<center>\n");
	printpretable_lite();
   	printf("<form method=post action=bbsplan?type=update>\n");
	fp=fopen(plan, "r");
	if(fp) {
		fread(buf, 9999, 1, fp);
		ptr=strcasestr(buf, "<textarea>");
		if(ptr) ptr[0]=0;
		fclose(fp);
	}
   	printf("<table width=610 border=0><tr><td>");
   	printf("<textarea class=thinborder name=text rows=20 cols=80 nowrap>\n");
	printf("%s",buf);
   	printf("</textarea></table>\n");
   	printf("<input type=submit value=存盘> ");
   	printf("<input type=reset value=复原>\n");
	printposttable_lite();
   	printf("</center>\n");
	http_quit();
}

int save_plan(char *plan) {
	char buf[10000];
	fp=fopen(plan, "w");
	strsncpy(buf, getparm("text"), 9999);
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("个人说明档修改成功。");
	http_quit();
}
