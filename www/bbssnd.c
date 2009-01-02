#include "libweb.h"
 
int main() {
   	FILE *fp;
	char filename[80], dir[80], board[80], title[80], buf[80], *content;
	char *origtitle;
	int r, i, sig, o_id, o_gid;
	char *reply_mode = NULL;
	char tempfilename[51];
	struct fileheader x;
	struct boardheader *brd;
   	init_all();
	printf("<b>发表文章 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	if(!loginok) http_fatal("匆匆过客不能发表文章，请先登录");
   	strsncpy(board, getparm("board"), 80);
	origtitle = getparm("title");
   	strsncpy(title, ansi_filter(origtitle, origtitle), 80);
 	brd=getbcache(board);
	if(brd==0) http_fatal("错误的讨论区名称");
	strcpy(board, brd->filename);
  	for(i=0; i<strlen(title); i++)
		if(title[i]<=27 && title[i]>=-1) title[i]=' ';
	check_title(title);//add by money 2003.11.2 for trim title
	/* added by money 04.01.17 for judge "Re: " when post new article */
	reply_mode = getparm("replymode");
	o_id = atoi(getparm("id"));
	o_gid = atoi(getparm("gid"));
	if (*reply_mode != '1')
	{
		if (!strncasecmp(title, "Re:", 3) && !HAS_PERM(PERM_SYSOPS))
		{
			if (strlen(title) > 48)
				title[48] = '\0';
			sprintf(tempfilename, "Re：%s", &title[3]);
			strcpy(title, tempfilename);
		}
	}
	/* added end */
	sig=atoi(getparm("signature"));
   	content=getparm("text");
   	if(title[0]==0)
      		http_fatal("文章必须要有标题");
      	sprintf(dir, "boards/%s/.DIR", board);
        if(!has_post_perm(&currentuser, board))
	    	http_fatal("此讨论区是唯读的, 或是您尚无权限在此发表文章.");
	sprintf(filename, "boards/%s/deny_users", board);
	if(file_has_word(filename, currentuser.userid))
		http_fatal("很抱歉, 您被版务人员停止了本版的post权利.");
#ifdef SPARC
	if(abs(time(0) - *(int*)(u_info->from+34))<6) { //modified from 36 to 34 for sparc solaris by roly 02.02.28
		*(int*)(u_info->from+34)=time(0); //modified from 36 to 34 for sparc solaris by roly 02.02.28
#else
	if(abs(time(0) - *(int*)(u_info->from+36))<6) { //modified from 36 to 34 for sparc solaris by roly 02.02.28
		*(int*)(u_info->from+36)=time(0); //modified from 36 to 34 for sparc solaris by roly 02.02.28
#endif
		http_fatal("两次发文间隔过密, 请休息几秒后再试");
	}
#ifdef SPARC
	*(int*)(u_info->from+34)=time(0);//modified from 36 to 34 for sparc solaris by roly 02.02.28
#else
	*(int*)(u_info->from+36)=time(0);//modified from 36 to 34 for sparc solaris by roly 02.02.28
#endif
	sprintf(filename, "tmp/%d.tmp", getpid());
        unlink(filename);
	f_append(filename, content);
	r=post_article(board, title, filename, currentuser.userid, currentuser.username, fromhost, o_id, o_gid, sig-1);

	
	if(r<=0) http_fatal("内部错误，无法发文");
	sprintf(buf, "M.%d.A", r);
	brc_initial(currentuser.userid, board);
	brc_addlist(buf);
	brc_update(currentuser.userid, board);
	unlink(filename);
	if(!junkboard(board)) {
        	currentuser.numposts++;
		save_user_data(&currentuser);
		write_posts(currentuser.userid, board, title);
	}
	//sprintf(buf, "%-12s %16.16s posted[www] '%s' on %s\n", currentuser.userid, cn_Ctime(time(0))+6, title, board);
	sprintf(buf, "posted '%s' on %s", title, board);
	trace(buf);
	sprintf(buf, "bbsdoc?board=%s", board);
	redirect(buf);
}

int write_posts(char *id, char *board, char *title) {
 	FILE *fp;
	struct post_log x;
 	strcpy(x.author, id);
 	strcpy(x.board, board);
 	if(!strncmp(title, "Re: ", 4)) title+=4;
 	strsncpy(x.title, title, 61);
 	if(title[0]==0) return;
	if(if_exist_title(title)) return; 
	x.date=time(0);
 	x.number=1;
	fp=fopen("tmp/.post", "a");
	fwrite(&x, sizeof(struct post_log), 1, fp);
	fclose(fp);
}

int if_exist_title(char *title) {
        static struct {
                int hash_ip;
                char title[64][60];
        } my_posts;
        char buf1[256];
        int n;
        FILE *fp;
        sethomefile(buf1, currentuser.userid, "my_posts");
        fp=fopen(buf1, "r+");
        if(fp==NULL) fp=fopen(buf1, "w+");
        fread(&my_posts, sizeof(my_posts), 1, fp);
        for(n=0; n<64; n++)
                if(!strncmp(my_posts.title[n], title, 60)) {
                        fclose(fp);
                        return 1;
                };
        my_posts.hash_ip = (my_posts.hash_ip+1) & 63;
        strncpy(my_posts.title[my_posts.hash_ip], title, 60);
        fseek(fp, 0, SEEK_SET);
        fwrite(&my_posts, sizeof(my_posts), 1, fp);
        fclose(fp);
        return 0;
}
