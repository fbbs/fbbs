#include "libweb.h"

int my_t_lines;

int main() 
{
	FILE *fp;
	char board[80], dir[80], *ptr;
	struct boardheader *x1;
	struct fileheader x;
	int i, start, total;
 	init_all();
	strsncpy(board, getparm("board"), 32);
	x1=getbcache(board);
	if(x1==0) 
	{
		printf("<b>管理模式 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	strcpy(board, x1->filename);
	if(!has_read_perm(&currentuser, board)) 
	{
		printf("<b>管理模式 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	if(!has_BM_perm(&currentuser, board))
	{
		printf("<b>管理模式 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("您没有权限访问本页");
	}
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	if(fp==0) 
	{
		printf("<b>管理模式 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区目录");
	}
	total=file_size(dir)/sizeof(struct fileheader);
	start=atoi(getparm("start"));
	if(strlen(getparm("start"))==0 || start>total-20) 
		start=total-20;
        my_t_lines=atoi(getparm("my_t_lines"));
	if(my_t_lines<10 || my_t_lines>40) my_t_lines=TLINES;
	if(strlen(getparm("start"))==0 || start>total-my_t_lines) start=total-my_t_lines+1;
	if(start<1)start=1;
						
	printf("<nobr>\n");

        printf("<table width=100%% border=0 ><tr><td width=85%% align=left>\n");

	printf("<img src=/images/icons/bm.jpg align=absmiddle border=0><b><font color=#FF6633 style='font-size: 18pt'>%s</font> · %s 管理模式 版主[%s] 文章数[%d]</b>\n",board,BBSNAME, userid_str(x1->BM), total);

        printf("<td width=15%% align=right>\n");
        printf("<form name=form1 action=bbsmdoc?board=%s method=post>", board);
        printf("<input border=0 src=/images/button/forward.gif type=image align=absmiddle> 第 <input class=thinborder type=text name=start size=4> 篇");
        printf("</form></td></tr></table>\n");

	if(total<=0) 
	{
		printpretable_lite();
		http_fatal("本讨论区目前没有文章");
	}
	
	printf("<br><br>");
        if(start>1)
	     printf("<a href=bbsmdoc?board=%s&start=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一页</a>  ",        board, start<my_t_lines ? 1 : start-my_t_lines);
        if(start<total-my_t_lines+1)
	        printf("<a href=bbsmdoc?board=%s&start=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一页</a>  ",      board, start+my_t_lines+1);

	printf("[<a href=bbsdoc?board=%s>一般模式</a>]  ", board);
        //commented by iamfat 2002.09.19
        //同步太麻烦了 让斑竹去telnet封人吧
        //printf("[<a href=bbsdenyall?board=%s>封人名单</a>]  ", board);
 	printf("[<a href=bbsmnote?board=%s>编辑进版画面</a>]  ", board);
		
	printf("<form name=form2 method=post action=bbsman>\n");

	printf("<input type=hidden name=mode value=''>\n");
	printf("<input type=hidden name=board value='%s'>\n", board);
	printf("<input type=button value=删除 onclick='document.form2.mode.value=1; document.form2.submit();'>\n");
	printf("<input type=button value=加M onclick='document.form2.mode.value=2; document.form2.submit();'>\n");
	printf("<input type=button value=加G onclick='document.form2.mode.value=3; document.form2.submit();'>\n");
	printf("<input type=button value=不可Re onclick='document.form2.mode.value=4; document.form2.submit();'>\n");
	printf("<input type=button value=清除MG onclick='document.form2.mode.value=5; document.form2.submit();'>\n");

	
	printpretable();
	printf("<table bgcolor=#ffffff>\n");
	printf("<tr class=pt9h><th nowrap>序号</th><th nowrap>管理</th><th nowrap>状态</th><th nowrap>作者</th><th nowrap>日期</th><th nowrap>标题</th></tr>\n");
	fseek(fp, (start-1)*sizeof(struct fileheader), SEEK_SET);
	int cc=0;
   	for(i=0; i<my_t_lines; i++) 
	{
		int isreply=0;
		char filename[80];
		if(fread(&x, sizeof(x), 1, fp)<=0) 
			break;
		//added by iamfat 2002.08.10
		//check_anonymous(x.owner);
		//added end.
		sprintf(filename, "boards/%s/%s", board, x.filename);
		printf("<tr class=%s><td nowrap align=right>%d</td>",((cc++)%2)?"pt9dc":"pt9lc" , start+i);
		printf("<td nowrap><input style='height:18px' name=box%s type=checkbox></td>", x.filename);
		printf("<td nowrap align=center><b>%s</b></td><td nowrap><b>%s</b></td>", flag_str(x.accessed[0]), userid_str(x.owner));
		printf("<td nowrap>%12.12s</td>", Ctime(atoi(x.filename+2))+4);
                
		isreply=!strncmp(x.title, "Re: ", 4);

		printf("<td nowrap width=100%%><a href=bbscon?b=%s&f=%s&n=%d>%s%36.36s </a></td></tr>",board,x.filename,start+i,
		isreply? "<img src=/images/types/reply.gif align=absmiddle border=0> "
		:"<img src=/images/types/text.gif align=absmiddle border=0> ",nohtml(isreply?(x.title+4):x.title));
   	}
   	printf("</table>\n");
	printposttable();
	printf("</form>\n");
	fclose(fp);
	http_quit();
}
