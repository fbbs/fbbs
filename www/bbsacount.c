/*******************************************
     annotator : DeepOcean, Oct 20,2005
*******************************************/

#include "BBSLIB.inc"
#include "boardrc.inc"
char *eff_size();
int my_t_lines;

struct user_doccount {
	char userid[IDLEN+50];
	int  doccount;
	int  gcount;
	int  mcount;
	int  xcount;
	int  othercount;
	struct user_doccount * next;
};


int main()
{
	struct user_doccount *udcount,*udcounthead,*tail, *prev, *swap;

	FILE *fp;
	int type=0,docstart,docend,total,i,docmin,sortmode;
	char dir[80],  board[80], userid[80];
	struct boardheader *brd;
	struct fileheader x;
	init_all();
	printf("<b><font style='font-size: 18pt'>发文统计</font> · %s </b><br>\n", BBSNAME);
	type=atoi(getparm("type"));					//获取发文统计的统计方式：1:M文,2:G文,3:水文,4:未标记,5:总数,6:ID
	strsncpy(board, getparm("board"), 30);		//获取发文统计的版面名称
	brd=getbcache(board);						//判断该版面名称是否存在
	if(brd==0) http_fatal("错误的讨论区");		//版面不存在
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区名称");		//判断当前用户是否有该版面的读权限
	if(type==0) return show_form(board);		//没有提交统计方式（第一次进入该页面或非本页面代码提交了错误的数据）时不统计，返回提交发文统计的表格

	if(type>6) return http_fatal("非法访问页面！");		//added by DeepOcean. Oct 22,2005 用户从非本页面提交非法数据
if (brd ->flag & BOARD_DIR_FLAG)
		      http_fatal("你选择的是一个目录"); //add by Danielfree 06.3.5
if ((brd->flag & BOARD_CLUB_FLAG)&& (brd->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, board)&& !isclubmember(currentuser.userid, board))
			        http_fatal("您不是俱乐部版 %s 的成员，无权访问该版面", board);
	sortmode=atoi(getparm("mode"));				//获取排序方式：0:降序，1:升序
	docstart=atoi(getparm("start"));			//获取起始篇数
	docend=atoi(getparm("end"));				//获取终止篇数
	docmin=atoi(getparm("min"));				//获取统计阈值，3篇起统计

	sprintf(dir, "boards/%s/.DIR", board);

	fp=fopen(dir, "r");
	if (!fp) http_fatal("错误的讨论区名称");	//.DIR指向错误目录
	total=file_size(dir)/sizeof(struct fileheader);

//	if(docstart<=0) docstart=0;
//	if(docend<=0 || docend>total) docend=total;

	//modified by DeepOcean, Oct 20,2005  修正起始篇数和终止篇数
	if(docstart<=0 || docstart>total) docstart=0;
	if(docend<docstart || docend>total) docend=total;

	if(docmin<3 || docmin>total) docmin=3;		//统计阈值，3篇起统计

	printf("<br>统计讨论区'%s'内, %d 到 %d 的发文超过%d的ID", board, docstart,docend,docmin);

	sprintf(dir, "boards/%s/.DIR", board);

	//printf("docstart:%d,docend:%d",docstart,docend);

	if(fp)
	{
		udcount=malloc(sizeof(struct user_doccount));
		udcount->next = NULL;
		udcounthead=udcount;
		fseek(fp, docstart*sizeof(struct fileheader), SEEK_SET);

		for(i=0; i<=docend-docstart; i++)
		{
			char filename[80];
			char *ptr;
			if(fread(&x, sizeof(x), 1, fp)<=0) break;
			//added by iamfat 2002.08.10
			//check_anonymous(x.owner);
			//added end.
			//printf(userid_str(x.owner));

			//统计发文数量
			udcount=udcounthead;
			while (udcount->next!=NULL)
			{
				if (!strcmp(udcount->next->userid,userid_str(x.owner)))
				{
					udcount->next->doccount++;
/*					if(x.accessed[0] & FILE_DIGEST) udcount->next->gcount++;
					if(x.accessed[0]& FILE_MARKED) udcount->next->mcount++;
					if(x.accessed[0]& FILE_DELETED) udcount->next->xcount++;
					if(!(x.accessed[0]& FILE_DELETED || x.accessed[0] & FILE_DIGEST
					 || x.accessed[0] & FILE_MARKED)) udcount->next->othercount++;
*/
					//modified by DeepOcean. Oct 22,2005
					if (x.accessed[0] & (FILE_DIGEST | FILE_MARKED))
					{
						if (x.accessed[0] & (FILE_DIGEST)) udcount->next->gcount++;
						if (x.accessed[0] & (FILE_MARKED)) udcount->next->mcount++;
					}
					else
						if (x.accessed[0] & (FILE_DELETED)) udcount->next->xcount++;
						else udcount->next->othercount++;
					//finished.

					break;
				}
				udcount=udcount->next;
			}
			if (udcount->next==NULL)
			{
				udcount->next=malloc(sizeof(struct user_doccount));
				udcount->next->next=NULL;
				udcount->next->doccount=1;
				udcount->next->gcount=0;
				udcount->next->mcount=0;
				udcount->next->xcount=0;
				udcount->next->othercount=0;
/*
				if(x.accessed[0] & FILE_DIGEST) udcount->next->gcount++;
				if(x.accessed[0]& FILE_MARKED) udcount->next->mcount++;
				if(x.accessed[0]& FILE_DELETED) udcount->next->xcount++;
				if(!(x.accessed[0]& FILE_DELETED || x.accessed[0] & FILE_DIGEST
				 || x.accessed[0] & FILE_MARKED))udcount->next->othercount++;
*/
				//modified by DeepOcean. Oct 22,2005
				if (x.accessed[0] & (FILE_DIGEST | FILE_MARKED))
				{
					if (x.accessed[0] & (FILE_DIGEST)) udcount->next->gcount++;
					if (x.accessed[0] & (FILE_MARKED)) udcount->next->mcount++;
				}
				else
					if (x.accessed[0] & (FILE_DELETED)) udcount->next->xcount++;
					else udcount->next->othercount++;
				//finished.

				sprintf(udcount->next->userid,userid_str(x.owner));
			}
		}
	}
	//排序
	tail = NULL;
	while (1)
	{
		udcount = udcounthead;
		prev = NULL;
		while (udcount && udcount->next != tail)
		{
			if ((type == 5 && (sortmode ? (udcount->doccount > udcount->next->doccount) : (udcount->doccount < udcount->next->doccount)))
				|| (type == 1 && (sortmode ? (udcount->mcount > udcount->next->mcount) : (udcount->mcount < udcount->next->mcount)))
				|| (type == 2 && (sortmode ? (udcount->gcount > udcount->next->gcount) : (udcount->gcount < udcount->next->gcount)))
				|| (type == 3 && (sortmode ? (udcount->xcount > udcount->next->xcount) : (udcount->xcount < udcount->next->xcount)))
				|| (type == 6 && (sortmode ? (toupper(udcount->userid[22]) > toupper(udcount->next->userid[22])) : (toupper(udcount->userid[22]) < toupper(udcount->next->userid[22]))))
				|| (type == 4 && (sortmode ? (udcount->othercount > udcount->next->othercount) : (udcount->othercount < udcount->next->othercount))))
			{
				if (prev)
					prev->next = udcount->next;
				else
					udcounthead = udcount->next;
				swap = udcount->next->next;
				udcount->next->next = udcount;
				udcount->next = swap;
				if (prev)
					udcount = prev->next;
				else
					udcount = udcounthead;
			}
			prev = udcount;
			udcount = udcount->next;
		}
		if (udcount == udcounthead) break;
		if (udcount) tail = udcount;
	}

	printpretable();
	printf("<table width=100%%>\n");
	printf("<tr  class=pt9h ><td><font color=white>帐号</td><td><font color=white>发文</td><td><font color=white>M文</td><td><font color=white>G文</td><td><font color=white>水文</td><td><font color=white>无标记</td></tr>\n");
	int cc=0;
	while (udcounthead->next!=NULL)
	{
		udcount=udcounthead->next;
		if (udcount->doccount>=docmin)
		{
			printf("<tr class=%s><td> %s</td><td> %d</td><td> %d</td><td> %d</td><td> %d</td><td> %d</td></tr>",((cc++)%2)?"pt9dc":"pt9lc" ,udcount->userid,udcount->doccount,udcount->mcount,udcount->gcount,udcount->xcount,
			udcount->othercount );
		}
		free(udcounthead);
		udcounthead=udcount;
	}
	printf("</table>");
	printposttable();
	printf("<br>[<a href='bbsdoc?board=%s'>返回讨论区</a>] ", board);
	http_quit();
}


int show_form(char *board)
{
	printpretable_lite();
/*
	printf("<table><form action=bbsacount method=post>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'/><br /></td></tr>\n", board);
	printf("<tr><td>起始篇数: <input type=text maxlength=8 size=8 name=start /><br /></td></tr>\n");
	printf("<tr><td>终止篇数: <input type=text maxlength=8 size=8 name=end /></td></tr>\n");
	printf("<tr><td>统计阈值: <input type=text maxlength=3 size=8 name=min value=5 /></td></tr>\n");
	printf("<tr><td>统计方式: <select name=type><option value='5' >总数</option><option value='1' >M文</option><option value='2' >G文</option><option value='3' >水文</option><option value='4' >未标记</option><option value='6' >id</option></select></td></tr>\n");
	printf("<tr><td>排序方式: <select name=mode><option value='0'>降序</option><option value='1' >升序</option></select></td></tr>\n");
	printf("<tr><td><input type=submit value=递交查询结果 /></td></tr>\n");
*/
//modified by DeepOcean, Oct 20,2005
	printf("<script language=JavaScript src=/javascript/datachk.js></script>\n");	//部分输入数据的验证由客户端的javascript完成，减少无谓的负担
	printf("<table><form action=bbsacount method=post onSubmit='return DataCheck(this)';>\n");
	printf("<tr><td>版面名称: <input class=thinborder type=text maxlength=24 size=24 name=board value='%s'></td></tr>\n", board);
	printf("<tr><td>起始篇数: <input class=thinborder type=text maxlength=8 size=8 name=start></td></tr>\n");
	printf("<tr><td>终止篇数: <input class=thinborder type=text maxlength=8 size=8 name=end></td></tr>\n");
	printf("<tr><td>统计阈值: <input class=thinborder type=text maxlength=3 size=8 name=min value=5></td></tr>\n");
	printf("<tr><td>统计方式: <select class=thinborder name=type><option value='5'>总数</option><option value='1'>M文</option><option value='2'>G文</option><option value='3'>水文</option><option value='4'>未标记</option><option value='6'>id</option></select></td></tr>\n");
	printf("<tr><td>排序方式: <select class=thinborder name=mode><option value='0'>降序　</option><option value='1'>升序　</option></select></td></tr>\n");
	printf("<tr><td><input type=submit value=递交查询结果 name=submit></td></tr>");
//finished.
	printf("</form></table>");
	printposttable_lite();
	printf("[<a href='bbsdoc?board=%s'>返回上一页</a>] ", board);
	http_quit();
}
