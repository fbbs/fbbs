#include "libweb.h"

char *eff_size();
int my_t_lines;
/* Function: 
 * 	get the sec number of a board
 * Write: roly			Date:2002.01.22
 */
int get_sec_number(char* board) {
	int i;
	char board_seccode;
	struct boardheader *x1;
	x1=getbcache(board);
	
	for (i=0;i<SECNUM;i++) {
		if (strchr(seccode[i],x1->title[0])) return i;
	}
	return -1;
}


int main() {
	FILE *fp;
	char board[80], dir[80], *ptr;
	struct boardheader *x1;
	struct fileheader x;
	int i, start, total;
 	int boardsec;
	int isreply=0;
	char path[256];
	
 	init_all();
	strlcpy(board, getparm("board"), 32);
	x1=getbcache(board);
	if (x1 == NULL) 
		http_fatal("错误的讨论区");
	strcpy(board, x1->filename);
	if(!has_read_perm(&currentuser, board)) 
		http_fatal("错误的讨论区");
	
        if (x1 ->flag & BOARD_DIR_FLAG)
           http_fatal("你选择的是一个目录"); //add by Danielfree 06.3.5
	
        if ((x1->flag & BOARD_CLUB_FLAG)
	 && (x1->flag & BOARD_READ_FLAG )
	 && !has_BM_perm(&currentuser, board)
	 && !isclubmember(currentuser.userid, board))
		http_fatal("您不是俱乐部版 %s 的成员，无权访问该版面", board);
		
	sprintf(dir, "boards/%s/.DIR", board);
    fp=fopen(dir, "r");
    total=file_size(dir)/sizeof(struct fileheader);
	start=atoi(getparm("start"));//added "-1" by roly 02.04.07
	my_t_lines=atoi(getparm("my_t_lines"));
	if(my_t_lines<10 || my_t_lines>40) my_t_lines=TLINES;
        if(strlen(getparm("start"))==0 || start>total-my_t_lines) start=total-my_t_lines+1;
  	if(start<1)
		start=1;
	brc_initial(currentuser.userid, board);
	printf("<body>");
	
	printf("<table width=100%% border=0 ><tr><td width=85%% align=left>\n");

	sprintf(path, "%s/info/boards/%s/icon.jpg",BBSHOME,board);
	if(dashf(path))
	{
		printf("<img src=/info/boards/%s/icon.jpg align=absmiddle width=32 height=32>",board);
	}
	
	//printf("<nobr>\n");
	printf("<b>");
		
	sprintf(path,"%s/info/boards/%s/banner.jpg",BBSHOME, board);
	if(dashf(path))
		printf("<img src=/info/boards/%s/banner.jpg align=absmiddle height=32>", board);
	else
		printf("<font style='font-size: 18pt'>%s</font> ·",board);


	printf(" %s 版主[%s] 文章数[%d]</b> <a href=bbsbrdadd?board=%s>预定本版</a>\n", BBSNAME, userid_str(x1->BM), total, board);
	printf("<td width=15%% align=right>\n");
	printf("<form name=form1 action=bbsdoc?board=%s method=post>", board);
	printf("<input border=0 src=/images/button/forward.gif type=image align=absmiddle> 第 <input class=thinborder type=text name=start size=4> 篇");
	printf("</form></td></tr></table>\n");

	printf("<table border=0 cellspacing=0 cellpadding=0 width=100%%><tr valign=top><td width=70%%>");
	showheadline(board);
	printf("</td><td width=30%% nowrap>");
	printpretable();
	printf("<b>版主推荐文章</b><br>");
	showrecommend(board,3,0);
	printposttable();
	printf("</td></tr></table>");											


	if(total<=0) http_fatal("本讨论区目前没有文章");
	
	printpretable();
	printf("<table width=100%% height=100%%><tr valign=top><td>");
	printf("<table bgcolor=#ffffff width=100%%>\n");
    printf("<tr class=pt9h bgcolor=#cccccc><th nowrap>序号</th><th nowrap>状态</th><th nowrap>作者</th><th nowrap>日期</th><th nowrap>标题\n"); //</th><th nowrap>人气\n");
	if(fp) 
	{
		fseek(fp, (start-1)*sizeof(struct fileheader), SEEK_SET);
		int cc=0;
		for(i=0; i<my_t_lines; i++) 
		{
			char filename[80];
			char *ptr, *font1="", *font2="";
			int noreply;
			if(fread(&x, sizeof(x), 1, fp)<=0) 
				break;
			//added by iamfat 2002.08.10
			//check_anonymous(x.owner);
			//added end.
			ptr=flag_str2(x.accessed[0], !brc_unread(x.filename));
			if(ptr[0]=='N') 
			{
				font1="<font color=#909090>";
				font2="</font>";
			}
			isreply=!strncmp(x.title, "Re: ", 4);
			if (x.accessed[0] & FILE_NOREPLY)
				noreply = 1;
			else
				noreply = 0;
						
			sprintf(filename, "boards/%s/%s", board, x.filename);
			printf("<tr class=%s><td nowrap>%d<td align=center nowrap><b>%s%s%s</b><td nowrap><b>%s</b>",
				((cc++)%2)?"pt9dc":"pt9lc", start+i, font1, ptr, font2, userid_str(x.owner));
         	printf("<td nowrap>%12.12s", Ctime(atoi(x.filename+2))+4);
         	printf("<td width=100%><a href=bbscon?b=%s&f=%s&n=%d>%s<font style='color:#%s'>%s</font> </a>%s</td></tr>",
				board, x.filename, start+i,
				isreply? noreply?"<img src=/images/types/reply_noreply.gif align=absmiddle border=0> ":"<img src=/images/types/reply.gif align=absmiddle border=0> "
				:noreply?"<img src=/images/types/text_noreply.gif align=absmiddle border=0> ":"<img src=/images/types/text.gif align=absmiddle border=0>",
				noreply?"000000":"000000",
				nohtml(isreply?(x.title+4):x.title), 
				eff_size(filename));
/*
		printf("<td><font color=%s>%d</font>\n",
#ifdef SPARC
					*(int*)(x.title+72)>99 ? "red" : "black", *(int*)(x.title+72));//modified by roly from 73 to 72 for sparc solaris
#else
                	*(int*)(x.title+73)>99 ? "red" : "black", *(int*)(x.title+73));//modified by roly from 73 to 72 for sparc solaris
#endif*/
      	}
		FILE *fpnotice;
		sprintf(dir, "boards/%s/.NOTICE", board);
		//printf("<td width=100%><h1/></td>");
		if (fpnotice = fopen(dir, "r")) {
			char filename[80];
			char *ptr, *font1="", *font2="";
			while ( fread(&x, sizeof(x), 1, fpnotice) == 1 ) {
				ptr=flag_str2(x.accessed[0], !brc_unread(x.filename));
				if(ptr[0]=='N') {
					font1="<font color=#909090>";
					font2="</font>";
				}
				isreply=!strncmp(x.title, "Re: ", 4);
				sprintf(filename, "boards/%s/%s", board, x.filename);
				printf("<tr class=%s><td nowrap>[∞]<td align=center nowrap>"
					"<b>%s%s%s</b><td nowrap><b>%s<b>",
					((cc++)%2)?"pt9dc":"pt9lc",
					font1,
					ptr,
					font2,
					userid_str(x.owner));
				printf("<td nowrap>%12.12s", Ctime(atoi(x.filename+2))+4);
				
				printf("<td width=100%><a href=bbscon?b=%s&f=%s&n=%d>%s%s </a>%s</td></tr>",
					board, x.filename, start+i,
					isreply? "<img src=/images/types/reply.gif align=absmiddle border=0> ":"<img src=/images/types/text.gif align=absmiddle border=0> ",
					nohtml(isreply?(x.title+4):x.title),
					eff_size(filename));
			}
			fclose(fpnotice);
		}
	
	printf("</table>");
	sprintf(path,"%s/info/boards/%s/code",BBSHOME, board);
	if(dashf(path))	
	{
		printf("</td><td width=250>");
		showrawcontent(path);
	}
	printf("</td></tr></table>");
	printposttable();
		
	}
	/* comment by roly */
	//printf("<a href=bbsfdoc?board=%s>文件上载</a> ", board);
	/* comment end */
	
	boardsec = get_sec_number(board);
	if (boardsec>=0 && boardsec<SECNUM) 
		printf("<a href=bbsboa?s=%d><img border=0 src=/images/button/home.gif align=absmiddle>当前分类讨论区</a> \n" ,boardsec);
	// added by roly 02.01.22
	{
		char upload_path[256];
		sprintf(upload_path,"%s/upload/%s",BBSHOME,board);
		if(dashd(upload_path))
		{
			printf("<a href=bbsfdoc?board=%s><img border=0 src=/images/button/attach.gif style='border:2px solid #ffffff' align=absmiddle>附件区</a> ",board);
		}
	}
	printf("<a href=bbspst?board=%s><img border=0 src=/images/button/edit.gif align=absmiddle>发表文章</a> \n", board);
	printf("<a href='javascript:location=location'><img border=0 src=/images/button/reload.gif align=absmiddle>刷新</a> ");
	if(start>0)
		printf("<a href=bbsdoc?board=%s&start=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一页</a> ", board, start-my_t_lines);
	if(start<total-my_t_lines)
		printf("<a href=bbsdoc?board=%s&start=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一页</a> ", board, start+my_t_lines);
	printf("<a href=bbstdoc?board=%s><img border=0 src=/images/button/content.gif align=absmiddle>主题模式</a>  ", board);
	if(has_BM_perm(&currentuser, board)) 
		printf("<a href=bbsmdoc?board=%s><img border=0 src=/images/button/bm.gif align=absmiddle>管理模式</a>  ", board);
	printf("<a href=bbsnot?board=%s>进版画面</a> ", board);
	printf("<a href=bbsgdoc?board=%s>文摘区</a> ", board);
	printf("<a href=bbs0an?path=%s>精华区</a> ", anno_path_of(board));
	//printf("<a href=/an/%s.tgz>下载精华区</a> ", board);
	//printf("<a href=%s.tgz>下载精华区</a> ",anno_path_of(board));
	//modified by iamfat 2002.08.19
	//if(has_BM_perm(&currentuser, board))
	//if(HAS_PERM(PERM_OBOARDS))
		printf("<a href=bbsbfind?board=%s>版内查询</a>\n", board);
	/* added by roly 2002.01.22 */
	if (has_BM_perm(&currentuser, board))
    printf("<a href=bbsacount?board=%s>发文统计</a>\n", board);
	/* add end */
	if(loginok) 
		printf("<a href=bbsclear?board=%s&start=%d>清除未读</a>\n", board, start);
	printf("<a href=bbsrss?board=%s>RSS</a>\n", board);
	fclose(fp);
	printf("</body>\n");
	http_quit();
}

char *eff_size(char *file) 
{
	FILE *fp;
	static char buf[512];
	int i, size, size2=0;
	int hasimg=0;
	char *attach_img="<img border=0 align=absmiddle src=/images/types/image.gif>";
	char *p;
	size=file_size(file);
	if(size>3000|| size==0) 
		goto E;
	size=0;
	fp=fopen(file, "r");
	if(fp==0) 
		return "-";
	for(i=0; i<3; i++)
		if(fgets(buf, 255, fp)==0) 
			break;
	while(1) 
	{
		if(fgets(buf, 255, fp)==0) 
			break;
		if(!strcmp(buf, "--\n")) 
			break;
		if(!strncmp(buf, ": ", 2)) 
			continue;
		if(!strncmp(buf, "<A1><BE> <D4><DA> ", 4)) 
			continue;
		if(strstr(buf, "<A1><F9> <C0><B4><D4><B4>:<A3><AE>")) 
			continue;
		for(i=0; buf[i]; i++) if(buf[i]<0) 
			size2++;
		size+=strlen(trim(buf));
                if(p=strstr(buf, "http://"))
	        {
	                 p=strtok(p," \r\n");
			 if(strstr(p, ".jpg")||strstr(p, ".JPG")
			    ||strstr(p, ".bmp")||strstr(p,".BMP")
			    ||strstr(p, ".gif")||strstr(p,".GIF")
			    ||strstr(p, ".png")||strstr(p,".PNG")
			    ||strstr(p, ".jpeg")||strstr(p,".JPEG"))
			 {
			 	hasimg=1;
			 }
		}
	}
	fclose(fp);
	E:
    if(size<2048)
		sprintf(buf, "%s (<font style='font-size:12px; color:#008080'>%d<D7><D6></font>)", hasimg?attach_img:"", size-size2/2);
	else 
		sprintf(buf, "%s (<font style='font-size:12px; color:#f00000'>%d.%d<C7><A7><D7><D6></font>)", hasimg?attach_img:"", size/1000, (size/100)%10);
	return buf;
}
