#include "BBSLIB.inc"
/*
struct deny {
	char id[80];
	char exp[80];
	int free_time;
} denyuser[256];
int denynum=0;

int loaddenyuser(char *board) {
	FILE *fp;
	char path[80], buf[256];
	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "r");
	if(fp==0) return;
	while(denynum<100) {
		if(fgets(buf, 80, fp)==0) break;
		sscanf(buf, "%s %s %d", denyuser[denynum].id, denyuser[denynum].exp, &denyuser[denynum].free_time);
		denynum++;
	}
	fclose(fp);
}
*/
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


int savedenyuser(char *board) {
	FILE *fp;
	int i;
	char path[80], buf[256], *exp;
	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "w");
	if(fp==0) return;
	for(i=0; i<denynum; i++) {		
		if(denyuser[i].id[0]==0) continue;	
		fprintf(fp, "%-12s %s", denyuser[i].id, denyuser[i].description);
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
	fp=fopen(path, "w");
	if(fp==0) return;
	for(i=0; i<denynum; i++) {
		int m;
		exp=denyuser[i].exp;
		if(denyuser[i].id[0]==0) continue;
		for(m=0; exp[m]; m++) {
			if(exp[m]<=32 && exp[m]>0) exp[m]='.';
		}
		fprintf(fp, "%-12s %s %d\n", denyuser[i].id, denyuser[i].exp, denyuser[i].free_time);
		denynum++;
	}
	fclose(fp);
}
*/
int main() {
	int i; 
	char board[80], *userid;
	init_all();
   	if(!loginok) http_fatal("您尚未登录, 请先登录");
	strsncpy(board, getparm("board"), 30);
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(!has_BM_perm(&currentuser, board)) http_fatal("您无权进行本操作");
	loaddenyuser(board);
	userid=getparm("userid");
   	for(i=0; i<denynum; i++) {
		if(!strcasecmp(denyuser[i].id, userid)) {
			denyuser[i].id[0]=0;
			savedenyuser(board);
			printf("已经给 %s 解封. <br>\n", userid);
			inform(board, userid);
			printf("[<a href=bbsdenyall?board=%s>返回被封名单</a>]", board);
			http_quit();
		}
	}
	http_fatal("这个用户不在被封名单中");
	http_quit();
}

int inform(char *board, char *user) {
        FILE *fp;
        char path[80], title[80];
        sprintf(title, "恢复 %s 在 %s 版的 POST 权力", user, board);
        sprintf(path, "tmp/%d.tmp", getpid());
	fp=fopen(path, "w");
        fprintf(fp, "【本公告由自动发信系统自动张贴】\n\n");
        fprintf(fp, "%s 恢复 %s 在 %s 版POST权.\n",  currentuser.userid, user, board);
        fclose(fp);
        post_article(board, title, path, "deliver", "自动发信系统", "自动发信系统", -1, -1, -1);
        post_article("Notice",title,path,"deliver", "自动发信系统", "自动发信系统", -1, -1, -1);
	//added by roly
	post_mail(user, title, path, currentuser.userid, currentuser.username, fromhost, -1);
        unlink(path);
        printf("系统已经发信通知了%s.<br>\n", user);
}

