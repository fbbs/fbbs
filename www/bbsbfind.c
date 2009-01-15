#include "libweb.h"

int main() {
	FILE *fp;
	int num=0, total=0, type, dt, mg=0, og=0;
	char dir[80], title[80], title2[80], title3[80], board[80], userid[80];
	struct boardheader *brd;
	struct fileheader x;
	init_all();
	printf("<center>%s -- 版内文章搜索<hr color=green><br>\n", BBSNAME);
	type=atoi(getparm("type"));
	strlcpy(board, getparm("board"), 30);
	if(type==0) return show_form(board);
	strlcpy(title, getparm("title"), 60);
	strlcpy(title2, getparm("title2"), 60);
	strlcpy(title3, getparm("title3"), 60);
	strlcpy(userid, getparm("userid"), 60);
	dt=atoi(getparm("dt"));
	if(!strcasecmp(getparm("mg"), "on")) mg=1;
	if(!strcasecmp(getparm("og"), "on")) og=1;
	if(dt<0) dt=0;
	if(dt>9999) dt=9999;
	brd=getbcache(board);
	if(brd==0) http_fatal("错误的讨论区");
	strcpy(board, brd->filename);
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if (brd ->flag & BOARD_DIR_FLAG)
			http_fatal("你选择的是一个目录"); //add by Danielfree 06.3.5
	if ((brd->flag & BOARD_CLUB_FLAG)&& (brd->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, board)&& !isclubmember(currentuser.userid, board))
		http_fatal("您不是俱乐部版 %s 的成员，无权访问该版面", board);	
	//modified by iamfat 2002.08.19
	//if(!has_BM_perm(&currentuser, board))http_fatal("对不起, 您无法使用搜索功能!\n");
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("讨论区错误或没有目前文章");
	printf("查找讨论区'%s'内, 标题含: '%s' ", board, nohtml(title));
	if(title2[0]) printf("和 '%s' ", nohtml(title2));
	if(title3[0]) printf("不含 '%s' ", nohtml(title3));
	printf("作者为: '%s', '%d'天以内的%s文章.<br>\n", 
		userid[0] ? userid_str(userid) : "所有作者", dt, mg ? "精华" : "所有");
	printpretable();
	printf("<table width=100%%  bgcolor=#ffffff>\n");
	printf("<tr class=pt9h><th nowrap>编号<th nowrap>标记<th nowrap>作者<th nowrap>日期<th nowrap>标题\n");
	int cc=0;
	int isreply=0;
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)==0) break;
		num++;
		//added by iamfat 2002.08.10
		//check_anonymous(x.owner);
		//added end.
		if(title[0] && !strcasestr_gbk(x.title, trim(title))) continue;
		if(title2[0] && !strcasestr_gbk(x.title, trim(title2))) continue;
		if(userid[0] && strcasecmp(x.owner, trim(userid))) continue;
		if(title3[0] && strcasestr_gbk(x.title, trim(title3))) continue;
		if(abs(time(0)-atoi(x.filename+2))>dt*86400) continue;
		if(mg && !(x.accessed[0] & FILE_MARKED) && !(x.accessed[0] & FILE_DIGEST)) continue;
		if(og && !strncmp(x.title, "Re: ", 4)) continue;
		total++;
		printf("<tr class=%s nowrap><td>%d", ((cc++)%2)?"pt9dc":"pt9lc" , num);
		printf("<td nowrap align=center><b>%s</b>", flag_str(x.accessed[0]));
		printf("<td nowrap><b>%s</b>", userid_str(x.owner));
		printf("<td nowrap>%12.12s", 4+Ctime(atoi(x.filename+2)));

		isreply=!strncmp(x.title, "Re: ", 4);
		//fix bug: 显示bug by DeepOcean:
		printf("<td nowrap width=100%%><a href=bbscon?b=%s&f=%s&n=%d>%s%s </a>\n", board, x.filename, num, isreply? "<img src=/images/types/reply.gif align=absmiddle border=0> "
		                :"<img src=/images/types/text.gif align=absmiddle border=0> ",nohtml(isreply?(x.title+4):x.title));
		if(total>=999) break;
	}
	fclose(fp);
	printf("</table>\n");
	printposttable();
	printf("<br>共找到 %d 篇文章符合条件", total);
	if(total>999) printf("(匹配结果过多, 省略第1000以后的查询结果)");
	printf("<br>\n");
	printf("[<a href=bbsdoc?board=%s>返回本讨论区</a>] [<a href='javascript:history.go(-1)'>返回上一页</a>]", board);
	http_quit();
}

int show_form(char *board) {
	printf("<table><form action=bbsbfind?type=1 method=post>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n", board);
	printf("<tr><td>标题含有: <input type=text maxlength=50 size=20 name=title> AND ");
	printf("<input type=text maxlength=50 size=20 name=title2>\n");
	printf("<tr><td>标题不含: <input type=text maxlength=50 size=20 name=title3>\n");
	printf("<tr><td>作者帐号: <input type=text maxlength=12 size=12 name=userid><br>\n");
	printf("<tr><td>时间范围: <input type=text maxlength=4  size=4  name=dt value=7> 天以内<br>\n");
	printf("<tr><td>精华文章: <input type=checkbox name=mg> ");
	printf("不含跟贴: <input type=checkbox name=og><br><br>\n");
	printf("<tr><td><input type=submit value=递交查询结果>\n");
	printf("</form></table>");
	if(HAS_PERM(PERM_OBOARDS)&&HAS_PERM(PERM_SPECIAL0)){//添加权限位控制0  06.1.2
	printf("[<a href='bbsdoc?board=%s'>返回上一页</a>] [<a href=bbsfind>全站文章查询</a>]", board);
	}
	else {
	  printf("[<a href='bbsdoc?board=%s'>返回上一页</a>]", board);
	}
	http_quit();
}
