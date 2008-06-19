#include "BBSLIB.inc"
FILE *fp;

int main() {
	FILE *fp;
	char *ptr, path[256], buf[10000];
   	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */

	if(!loginok) http_fatal("匆匆过客不能设置签名档，请先登录");
	printf("<font style='font-size: 18pt'>%s</font> · %s 设置签名档 \n", currentuser.userid, BBSNAME);
	sprintf(path, "home/%c/%s/signatures", 
		toupper(currentuser.userid[0]), currentuser.userid);
	if(!strcasecmp(getparm("type"), "1")) save_sig(path);
	printpretable_lite();
	printf("<form method=post action=bbssig?type=1>\n");

	fp=fopen(path, "r");
	bzero(buf, 10000);
	if(fp) {
		fread(buf, 9999, 1, fp);
		ptr=strcasestr(buf, "<textarea>");
		if(ptr) ptr[0]=0;
		fclose(fp);
	}
   	printf("签名档每6行为一个单位, 可设置多个签名档.<br>");
   	printf("<textarea class=ansi name=text rows=20 cols=80 nowrap>\n");
	printf("%s",buf);
   	printf("</textarea></br>\n");
   	printf("<input type=submit value=存盘> ");
   	printf("<input type=reset value=复原>\n");
   	printf("</form>\n");
	printposttable_lite();

	http_quit();
}

int save_sig(char *path) {
	char *buf;
	fp=fopen(path, "w");
	buf=getparm("text");
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("签名档修改成功。");
	http_quit();
}
