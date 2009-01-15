#include "libweb.h"

int main() {
	FILE *fp;
	char buf[1024], title[80], board[80], dir[80], first_file[80];
	struct boardheader *x1;
	struct fileheader x, x0;
	int sum=0, total=0;
 	init_all();
	strlcpy(board, getparm("board"), 32);
	strlcpy(title, getparm("title"), 42);
	x1=getbcache(board);
	if(x1==0) 
	{
		printf("<center><b>同主题查找 · %s </b></center><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	strcpy(board, x1->filename);
	if(!has_read_perm(&currentuser, board)) 
	{
		printf("<center><b>同主题查找 · %s </b></center><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	sprintf(buf, "bbsman?board=%s&mode=1", board);
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	if(fp==0) 
	{
		printf("<center><b>同主题查找 · %s </b></center><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区目录");
	}
	printf("<center><b>同主题查找 · %s [讨论区: %s] [主题 '%s']</b><br>\n",	BBSNAME, board, nohtml(title));
	printpretable();
	printf("<table width=100%% border=0><tr class=pt9h><td><font color=white>编号<td><font color=white>作者<td><font color=white>日期<td><font color=white>标题\n");
	int cc=0;
	while(1) 
	{
		if(fread(&x, sizeof(x), 1, fp)==0) 
			break;
		//added by iamfat 2002.08.10
		//check_anonymous(x.owner);
		//added end.
		sum++;
		if(!strncmp(title, x.title, 40) || (!strncmp(title, x.title+4, 40) && !strncmp(x.title, "Re: ", 4))) 
		{
			if(total==0) 
				strcpy(first_file, x.filename);
			printf("<tr class=%s><td>%d",((cc++)%2)?"pt9dc":"pt9lc" , sum);
			printf("<td>%s", userid_str(x.owner));
			if(!(x.accessed[0]&(FILE_MARKED|FILE_DIGEST))) 
			{
				char buf2[20];
				sprintf(buf2, "&box%s=on", x.filename);
				if(strlen(buf)<500) 
					strcat(buf, buf2);
			}
			printf("<td>%6.6s", Ctime(atoi(x.filename+2))+4);
			printf("<td><a href=bbscon?b=%s&f=%s&n=%d>%s</a>\n", board, x.filename, sum, x.title);
			total++;
		}
	}
	fclose(fp);
	printf("</table>\n");
	printposttable();
	printf("<br>共找到 %d 篇 \n", total);
	printf("<a href=bbsdoc?board=%s>本讨论区</a> ", board);
	if(total>0) 
	{
		printf("<a href=bbstcon?board=%s&file=%s>本主题全部展开</a> ", board, first_file);
		if(has_BM_perm(&currentuser, board)) 
			printf("<a onclick='return confirm(\"确定同主题全部删除?\")' href=%s>同主题删除</a>", buf);
	}
	http_quit();
}
