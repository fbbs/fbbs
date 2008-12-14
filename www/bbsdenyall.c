#include "libweb.h"

struct deny {
	char id[80];
	//char exp[80];
	//int free_time;
	char description[256]; //added by roly 02.03.22
} denyuser[256];
int denynum=0;
/* added by roly  02.03.22 */
int getdenyrecord(char* buf,FILE *stream)
{
	int ch,i;

	ch = fgetc( stream );
   	for( i=0; (i < 512 ) && ( feof( stream ) == 0 && ch!=0x0A); i++ )
   	{
      		buf[i] = (char)ch;
      		ch = fgetc( stream );
   	}
	buf[i]=0x0A;
	if (feof( stream ) != 0) return -1;
	return 0;
}

/* add end */

int loaddenyuser(char *board) {
	FILE *fp;
	char path[80], buf[256];
	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "r");
	if(fp==0) return;
	while(denynum<100) {
		int i,j;
		//if(fgets(buf, 80, fp)==0) break;
		if (getdenyrecord(buf,fp)==-1) break; //modified by roly 02.03.22
		//sscanf(buf, "%s %s %d", denyuser[denynum].id, denyuser[denynum].exp, &denyuser[denynum].free_time);
		sscanf(buf, "%s", denyuser[denynum].id);		
		i = strlen(denyuser[denynum].id);
		j = 0;
		while (buf[i]!=0x0A) {
			denyuser[denynum].description[j]=buf[i];
			i++;j++;
		}
		denyuser[denynum].description[j]=0x0A;
		//strcpy(denyuser[denynum].description,buf+strlen(denyuser[denynum].id)); //added by roly 02.03.22
		denynum++;
	}
	fclose(fp);
}

/*
int savedenyuser(char *board) {
	FILE *fp;
	int i;
	char path[80], buf[256], *exp;
	sprintf(path, "boards/%s/deny_users", board);
	if(fp==0) return;
	for(i=0; i<denynum; i++) {
		int m;
		exp=denyuser[i].exp;
		for(m=0; exp[m]; m++) {
			if(exp[m]<=32 && exp[m]>0) exp[m]='.';
		}
		fprintf(fp, "%-12s %s %d\n", denyuser[i].id, denyuser[i].exp, denyuser[i].free_time);
	}
	fclose(fp);
}
*/
int main() {
	int i; 
	char board[80];
	init_all();
   	if(!loginok) http_fatal("您尚未登录, 请先登录");
	strsncpy(board, getparm("board"), 30);
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(!has_BM_perm(&currentuser, board)) http_fatal("您无权进行本操作");
	loaddenyuser(board);
   	printf("<center>\n");
   /*	
	printf("%s -- 被封用户名单 [讨论区: %s]<hr color=green><br>\n", BBSNAME, board);
   	printf("本版共有 %d 人被封<br>", denynum);
   	printf("<table border=0  bgcolor=#ffffff><tr class=pt9 bgcolor=#70a6ff><td><font color=white>序号<td><font color=white>用户帐号<td><font color=white>被封原因<td><font color=white>原定解封日期<td><font color=white>管理\n");
   	for(i=0; i<denynum; i++) {
		printf("<tr class=pt9 bgcolor=#f2f2f2><td>%d", i+1);
		printf("<td><a href=bbsqry?userid=%s>%s</a>", denyuser[i].id, denyuser[i].id);
		printf("<td>%s\n", nohtml(denyuser[i].exp));
		printf("<td>%s\n", Ctime(denyuser[i].free_time)+4);
		printf("<td>[<a onclick='return confirm(\"确实解封吗?\")' href=bbsdenydel?board=%s&userid=%s>解封</a>]", 
			board, denyuser[i].id);
	}
   	printf("</table><hr color=green>\n");
*/
	printf("%s -- 被封用户名单 [讨论区: %s]<hr color=green><br>\n", BBSNAME, board);
   	printf("本版共有 %d 人被封<br>", denynum);
   	printf("<table border=0  bgcolor=#ffffff><tr class=pt9 bgcolor=#70a6ff><td><font color=white>序号<td><font color=white>用户帐号<td><font color=white>被封原因<td><font color=white>解封日期<td><font color=white>管理\n");
   	int cc=0;
	for(i=0; i<denynum; i++) {
		printf("<tr class=%s ><td>%d",((cc++)%2)?"pt9dc":"pt9lc" , i+1);
		printf("<td><a href=bbsqry?userid=%s>%s</a>", denyuser[i].id, denyuser[i].id);
		printf("<td>%s\n", nohtml(denyuser[i].description));
		printf("<td>[<a onclick='return confirm(\"确实解封吗?\")' href=bbsdenydel?board=%s&userid=%s>解封</a>]", 
			board, denyuser[i].id);
	}
   	printf("</table><hr color=green>\n");


	printf("[<a href=bbsdenyadd?board=%s>设定新的不可POST用户</a>]</center>\n", board);
	http_quit();
}
