#include "BBSLIB.inc"
char *stat1();
int my_t_lines;

int main() {
	FILE *fp;
	char board[80], dir[80], *ptr;
	struct boardheader *x1;
	struct fileheader *data;
	int i, start, total2=0, total, sum=0;
	char path[256];
 	init_all();
	strsncpy(board, getparm("board"), 32);
	x1=getbcache(board);
	 if ((x1->flag & BOARD_CLUB_FLAG)
	      && (x1->flag & BOARD_READ_FLAG )
		       && !has_BM_perm(&currentuser, board)
			        && !isclubmember(currentuser.userid, board))
					        http_fatal("您不是俱乐部版 %s 的成员，无权访问该版面", board);
	if(x1==0) http_fatal("错误的讨论区");
	strcpy(board, x1->filename);
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	sprintf(dir, "boards/%s/.DIR", board);
        fp=fopen(dir, "r");
        if(fp==0) http_fatal("错误的讨论区目录");
	total=file_size(dir)/sizeof(struct fileheader);
	data=calloc(sizeof(struct fileheader), total);
	if(data==0) http_fatal("内存溢出");
	total=fread(data, sizeof(struct fileheader), total, fp);
	fclose(fp);
	for(i=0; i<total; i++) if(strncmp(data[i].title, "Re: ", 4)) total2++;
	start=atoi(getparm("start"));
        my_t_lines=atoi(getparm("my_t_lines"));
        if(my_t_lines<10 || my_t_lines>40) my_t_lines=TLINES;
        if(strlen(getparm("start"))==0 || start>total2-my_t_lines) start=total2-my_t_lines+1;
	if(start<1) start=1;
	printf("<body>");
	printf("<nobr>\n");

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

	printf(" %s 主题阅读 版主[%s] 文章%d, 主题%d个</b>\n", BBSNAME, userid_str(x1->BM), total, total2);

        printf("<td width=15%% align=right>\n");
        printf("<form name=form1 action=bbstdoc?board=%s method=post>", board);
        printf("<input border=0 src=/images/button/forward.gif type=image align=absmiddle> 第 <input class=thinborder type=text name=start size=4> 篇");
        printf("</form></td></tr></table>\n");

        printf("<table cellspacing=0 cellpadding=0 border=0 width=100%%><tr valign=top><td width=70%%>");
	showheadline(board);
	/*
        sprintf(path,"%s/info/boards/%s/headline.txt",BBSHOME, board);
        if(dashf(path))
        {
                printpretable();
                printf("<b>HEADLINE</b><br>");
                printpremarquee("100%%", "48");
                showcontent(path);
                printpostmarquee();
                printposttable();
        }*/
        printf("</td><td width=30%% nowrap>");
        printpretable();
        printf("<b>版主推荐文章</b><br>");
        showrecommend(board,3,0);
        printposttable();
        printf("</td></tr></table>");


	if(total<=0) http_fatal("本讨论区目前没有文章");
	if(true){
	printf("<a href=bbspst?board=%s><img border=0 src=/images/button/edit.gif align=absmiddle>发表文章</a> ", board);
	if(start>0)
		printf("<a href=bbstdoc?board=%s&start=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一页</a> ", board, start-my_t_lines);
	if(start<total2-my_t_lines)
		printf("<a href=bbstdoc?board=%s&start=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一页</a> ", board, start+my_t_lines);
	printf("<a href=bbsnot?board=%s>进版画面</a> ", board);
	printf("<a href=bbsdoc?board=%s>一般模式</a> ", board);
	printf("<a href=bbsgdoc?board=%s>文摘区</a> ", board);
	printf("<a href=bbs0an?path=%s>精华区</a> ", anno_path_of(board));
	printf("<a href=%s.tgz>下载精华区</a>  <br>", anno_path_of(board));
	}
	printpretable();
	printf("<table width=100%% height=100%%><tr valign=top><td>");
	printf("<table bgcolor=#ffffff width=100%%>\n");
   	printf("<tr class=pt9h bgcolor=#70a6ff><th nowrap>序号</th><th nowrap>状态</th><th nowrap>作者<th nowrap>日期<th nowrap width=100%%>标题\n"); //<td><font color=white>回帖/人气\n");
	int cc=0;
	for(i=0; i<total; i++) {
		if(!strncmp(data[i].title, "Re: ", 4)) continue;
		sum++;
		if(sum<start) continue;
		if(sum>start+my_t_lines) break;
	//added by iamfat 2002.08.10
	//check_anonymous(data[i].owner);
	//added end.
		printf("<tr class=%s><td nowrap align=right>%d<td nowrap align=center><b>%s</b><td nowrap><b>%s</b>",
			((cc++)%2)?"pt9dc":"pt9lc" ,sum, flag_str(data[i].accessed[0]), userid_str(data[i].owner));
         	printf("<td nowrap>%6.6s", Ctime(atoi(data[i].filename+2))+4);
        	printf("<td nowrap width=100%%><img src=/images/types/text.gif align=absmiddle border=0> <a href=bbstcon?board=%s&file=%s>%s</a>", //<td>%s",
			board, data[i].filename,
			nohtml(data[i].title)); 
			//stat1(data, i, total));
      	}
      	printf("</table>\n");
        sprintf(path,"%s/info/boards/%s/code",BBSHOME, board);
        if(dashf(path))
        {
                printf("</td><td width=250>");
                showrawcontent(path);
        }
        printf("</td></tr></table>");
 
	printposttable();
	if(true){
	printf("<a href=bbspst?board=%s><img border=0 src=/images/button/edit.gif align=absmiddle>发表文章</a> ", board);
	if(start>0)
		printf("<a href=bbstdoc?board=%s&start=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一页</a> ", board, start-my_t_lines);
	if(start<total2-my_t_lines)
		printf("<a href=bbstdoc?board=%s&start=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一页</a> ", board, start+my_t_lines);
	printf("<a href=bbsnot?board=%s>进版画面</a> ", board);
	printf("<a href=bbsdoc?board=%s>一般模式</a> ", board);
	printf("<a href=bbsgdoc?board=%s>文摘区</a> ", board);
	printf("<a href=bbs0an?path=%s>精华区</a> ", anno_path_of(board));
	printf("<a href=%s.tgz>下载精华区</a>  <br>", anno_path_of(board));
	}
	free(data);
	http_quit();
}

char *stat1(struct fileheader *data, int from, int total) {
	static char buf[256];
	char *ptr=data[from].title;
#ifdef SPARC
	int i, re=0, click=*(int*)(data[from].title+72);//modified by roly from 73 to 72 for sparc solaris
#else
	int i, re=0, click=*(int*)(data[from].title+73);//modified by roly from 73 to 72 for sparc solaris
#endif
	for(i=from; i<total; i++) {
		if(!strncmp(ptr, data[i].title+4, 40)) {
			re++;
#ifdef SPARC
			click+=*(int*)(data[i].title+72);//modified by roly from 73 to 72 for sparc solaris
#else
			click+=*(int*)(data[i].title+73);//modified by roly from 73 to 72 for sparc solaris
#endif
		}
	}
	sprintf(buf, "<font color=%s>%d</font>/<font color=%s>%d</font>", 
		re>9 ? "red" : "black", re, click>499 ? "red" : "black", click);
	return buf;
}
