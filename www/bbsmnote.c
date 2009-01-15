#include "libweb.h"
FILE *fp;

int main() {
	FILE *fp;
	char *ptr, path[256], buf[10000], board[256];
   	init_all();
	printf("<b>编辑进版画面 · %s [讨论区: %s]</b><br>\n", BBSNAME, board);

	printpretable_lite();
	if(!loginok) http_fatal("匆匆过客，请先登录");
	strlcpy(board, getparm("board"), 30);
	if(!has_BM_perm(&currentuser, board)) http_fatal("您无权进行本操作");
	strlcpy(board, getbcache(board)->filename, 30);
	sprintf(path, "vote/%s/notes", board);
	if(!strcasecmp(getparm("type"), "update")) save_note(path);
   	printf("<form method=post action=bbsmnote?type=update&board=%s>\n", board);
	fp=fopen(path, "r");
	if(fp) {
		fread(buf, 9999, 1, fp);
		ptr=strcasestr_gbk(buf, "<textarea>");
		if(ptr) ptr[0]=0;
		fclose(fp);
	}
   	printf("<table width=610 border=1><tr><td>");
   	printf("<textarea name=text rows=20 cols=80 wrap=physicle>\n");
	printf("%s",buf);
   	printf("</textarea></table>\n");
	printposttable_lite();
   	printf("<input type=submit value=存盘>  ");
   	printf("<input type=reset value=复原>\n");
	http_quit();
}

int save_note(char *path) {
	char buf[10000];
	fp=fopen(path, "w");
	strlcpy(buf, getparm("text"), 9999);
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("进版画面修改成功。<br>\n");
	printf("<a href='javascript:history.go(-2)'>返回</a>");
	http_quit();
}
