#include "libweb.h"

struct deny {
	char id[80];
//	char exp[80];
//	int free_time;
//	char freetime[80];
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

int savedenyuser(char *board) {
/* modified by roly  02.03.22*/

	FILE *fp;
	int m;
	char path[80], buf[256], *exp;
	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "a+");
	if(fp==0) return;
	
	
	if(denyuser[denynum].id[0]==0) return;
	fprintf(fp, "%-12s %s\n", denyuser[denynum].id, denyuser[denynum].description);

	fclose(fp);
/* modify end */
}

int main() {
	int i; 
	struct tm* tmtime;
	time_t daytime;

	char exp[80], board[80], *userid;
	int dt;
	init_all();
   	if(!loginok) http_fatal("您尚未登录, 请先登录");
	strsncpy(board, getparm("board"), 30);
	strsncpy(exp, getparm("exp"), 30);
	dt=atoi(getparm("dt"));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(!has_BM_perm(&currentuser, board)) http_fatal("您无权进行本操作");
	loaddenyuser(board);
	userid=getparm("userid");
	if(userid[0]==0) return show_form(board);
	if(getuser(userid)==0) http_fatal("错误的使用者帐号");
	strcpy(userid, getuser(userid)->userid);
	if(dt<1 || dt>150) http_fatal("请输入被封天数(1-150)");
	if(exp[0]==0) http_fatal("请输入封人原因");
   	for(i=0; i<denynum; i++)
		if(!strcasecmp(denyuser[i].id, userid)) http_fatal("此用户已经被封");
	if(denynum>40) http_fatal("太多人被封了");
	strsncpy(denyuser[denynum].id, userid, 13);
	//strsncpy(denyuser[denynum].exp, exp, 30);
	//denyuser[denynum].free_time=time(0)+dt*86400;

	/* added by roly 02.03.22 */
	daytime = time(0)+dt*86400;
	tmtime=gmtime(&daytime);
        sprintf(denyuser[denynum].description, "%-40s %02d年%02d月%02d日解",
		 exp , tmtime->tm_year%100, tmtime->tm_mon+1,tmtime->tm_mday);
	/* added end */
	savedenyuser(board);
	printf("封禁 %s 成功<br>\n", userid);
	inform(board, userid, exp, dt);
	printf("[<a href=bbsdenyall?board=%s>返回被封帐号名单</a>]", board);
	http_quit();
}

int show_form(char *board) {
	printf("<center>%s -- 版务管理 [讨论区: %s]<hr color=green>\n", BBSNAME, board);
	printf("<form action=bbsdenyadd><input type=hidden name=board value='%s'>", board);
	printf("封禁使用者<input name=userid size=12> 本版POST权 <input name=dt size=2> 天, 原因<input name=exp size=20>\n");
	printf("<input type=submit value=确认></form>");
}

int inform(char *board, char *user, char *exp, int dt) {
	FILE *fp;
	char path[80], title[80];
	struct tm* tmtime;
	time_t daytime;

	daytime = time(0)+dt*86400;
	tmtime=gmtime(&daytime);

	sprintf(title, "取消 %s 在 %s 版的 POST 权力", user, board);
	sprintf(path, "tmp/%d.tmp", getpid());
	fp=fopen(path, "w");
	fprintf(fp, "【本公告由自动发信系统自动张贴】\n\n");
	//fprintf(fp, "%s被版务人员[%s]封禁了本版POST权[%d]天.\n", user, currentuser.userid, dt);
	//fprintf(fp, "原因是: %s\n", exp);
	fprintf(fp, "执行人  : %s\n封人原因: %s\n被封天数: %d\n解封日期: [%02d年%02d月%02d日]\n解封方式：发信申请\n", 
         	currentuser.userid,exp,dt,tmtime->tm_year%100,tmtime->tm_mon+1,tmtime->tm_mday);


	
	fclose(fp);
	post_article(board, title, path, "deliver", "自动发信系统", "自动发信系统", -1, -1, -1);
	post_article("Notice",title,path,"deliver", "自动发信系统", "自动发信系统", -1, -1, -1);
	post_mail(user, title, path, currentuser.userid, currentuser.username, fromhost, -1);
	unlink(path);
	printf("系统已经发信通知了%s.<br>\n", user);
}
