#include "libweb.h"

int main() {
	FILE *fp;
	char *board, path[512], buf[512], ch, tmp[80], *ptr; 
	struct boardheader *x1;
	init_all();
	strlcpy(path, getparm("path"), 511);
    if(strstr(path,"bbslist"))
            http_fatal("错误的讨论区");
	board=getbfroma(path);
	if(board[0]) 
	{
		if (!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
		x1=getbcache(board);
		if ((x1->flag & BOARD_CLUB_FLAG)
			&& (x1->flag & BOARD_READ_FLAG )
			&& !has_BM_perm(&currentuser, board)
			&& !isclubmember(currentuser.userid, board))
			http_fatal("您不是俱乐部版 %s 的成员，无权访问该文件", board);
	}
	buf[0]=0;
	if(board[0]) sprintf(buf, "%s", board);
	printf("<center><b>%s · %s 精华区文章阅读</b></center><br>\n", board, BBSNAME);
	if(strstr(path, ".Search") || strstr(path, ".Names")|| strstr(path, "..")|| strstr(path, "SYSHome"))
	{
		printpretable_lite();
		http_fatal("错误的文件名");
	}
	sprintf(buf, "0Announce%s", path);
	printpretable();
	printf("<table border=0 width=100%%>");
	printf("<tr><td><pre class=ansi>");
	fp=fopen(buf, "r");
	if(fp==0) 
		printf("错误的文件名");
	else
	{
		while(1) 
		{
			if(fgets(buf, 256, fp)==0) 
				break;
			hhprintf("%s", buf);
		}
		fclose(fp);
	}
   	printf("</pre>\n</table>\n");
	printposttable();
	printf("<center>[<a href='javascript:history.go(-1)'>返回上一页</a>]  ");
   	if(board[0]) 
		printf("[<a href=bbsdoc?board=%s>本讨论区</a>] ", board);
	printf("</center>\n");
	http_quit();
}
