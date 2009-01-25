#include "libweb.h"

static int cmpboard(const void *b1, const void *b2)
{
	return strcasecmp(((struct boardheader *)b1)->filename,
		((struct boardheader *)b2)->filename);
}

static int filenum(char *board) {
	char file[256];
	sprintf(file, "boards/%s/.DIR", board);
	return file_size(file)/sizeof(struct fileheader);
}

static int board_read(char *board) {
	char buf[256], path[256];
	FILE *fp;
	struct fileheader x;
	int total;
	if(!loginok) return 1;
	bzero(&x, sizeof(x));
	sprintf(buf, "boards/%s/.DIR", board);
	total=file_size(buf)/sizeof(struct fileheader);
	if(total<=0) return 1;
	fp=fopen(buf, "r+");
	fseek(fp, (total-1)*sizeof(struct fileheader), SEEK_SET);
	fread(&x, sizeof(x), 1, fp);
	fclose(fp);
	brc_initial(currentuser.userid, board);
	return !brc_unread(x.filename);
}

int bbsboa_main(void)
{
	struct boardheader data[MAXBOARD], *x;
	int i, total=0, sec1;
	char *cgi="bbsdoc", *ptr, *my_sec;
	char path[256];
	char *parent_name = NULL;
    struct boardheader *parent = NULL;
    int parent_bid = 0;

	sec1 = atoi(getsenv("QUERY_STRING"));
	if(sec1 < 0 || sec1 >= SECNUM)
		http_fatal("错误的参数");
	if(atoi(getparm("my_def_mode")) != 0)
		cgi = "bbstdoc";
	/* rqq: 2006.2.14: allow board directory listing via the
     * board=boardname parameter. NOTE: this implementation 
     * reles on the getbnum() routine to check user permissiong.
     */
    parent_name = getparm("board");
    if (parent_name) {
        parent = getbcache(parent_name);
        parent_bid = getbnum(parent_name, &currentuser);
        if (parent == NULL || parent_bid <= 0 || !(parent->flag & BOARD_DIR_FLAG)) {
            parent = NULL;
            parent_bid = 0;
            parent_name = NULL;
        }
    }

	for(i=0; i<MAXBOARD; i++) {
		x=&(bcache[i]);
		if(x->filename[0]<=32 || x->filename[0]>'z') continue;
		if(!has_read_perm(&currentuser, x->filename)) continue;
		if ((x->flag & BOARD_CLUB_FLAG)&& (x->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, x->filename)&& !isclubmember(currentuser.userid, x->filename))
			continue;
		if (parent) { // directory listing
            if (x->group != parent_bid)
                continue;
        } else { // section listing
            if(!strchr(seccode[sec1], x->title[0])) continue;
        }
		memcpy(&data[total], x, sizeof(struct boardheader));
		total++;
	}
	qsort(data, total, sizeof(struct boardheader), cmpboard);
	
	sprintf(path, "%s/info/egroup%d/icon.jpg",BBSHOME,sec1);
	
	if(dashf(path))
	{
		printf("<img src=/info/egroup%d/icon.jpg align=absmiddle width=32 height=32>",sec1);
	}
	printf("<b>");

	sprintf(path,"%s/info/egroup%d/banner.jpg",BBSHOME, sec1);
	if (parent) {
        printf("<font style='font-size: 18pt'>%s</font> ·",
               parent->title + 10);
    }
    else if(dashf(path)) {
        printf("<img src=/info/egroup%d/banner.jpg "
               "align=absmiddle height=32>", sec1);
    } 
    else {
        printf("<font style='font-size: 18pt'>%s</font> ·",
            secname[sec1][0]);
    }
    
    if (parent)
        printf(" %s 版面目录 </b>",  BBSNAME);
    else
        printf(" %s 分类讨论区 </b>",  BBSNAME);

	sprintf(path,"%s/info/egroup%d/headline.txt",BBSHOME, sec1);
	if(!parent && dashf(path))
	{
		printpretable();
		printf("<b>HEADLINE</b><br>");
		printpremarquee("100%%", "48");
		showcontent(path);
		printpostmarquee();
		printposttable();
	}
	printpretable();
	printf("<table width=100%% bgcolor=#ffffff>\n");
	printf("<tr class=pt9h align=center><td nowrap><b>序号</b></td><td nowrap><b>未<td nowrap><b>讨论区名称</b></td><td nowrap><b>更新时间</b></td><td><b>类别</b></td><td nowrap><b>中文描述</b></td><td nowrap><b>版主</b></td><td nowrap><b>文章数\n");
	int cc=0;
	for(i=0; i<total; i++) {
		char buf[100];
		int isgroup = (data[i].flag & BOARD_DIR_FLAG)? 1 : 0;
		if(specialboard(data[i].filename))continue;
		sprintf(buf, "boards/%s/.DIR", data[i].filename);
		 /* print index */
		printf("<tr class=%s valign=top><td align=right nowrap>%d</td>",
                       ((cc++)%2)?"pt9dc":"pt9lc", i+1);	
		/* print brc flag */
        if (isgroup) 
            printf("<td nowrap>-</td>");
        else
            printf("<td nowrap>%s", board_read(data[i].filename) ? "◇" : "◆");
       
       if (isgroup)
            printf("<td nowrap><a href=%s?board=%s><b>[ %s ]</b></a>", 
                   "bbsboa", data[i].filename, data[i].filename);
        else
            printf("<td nowrap><a href=%s?board=%s><b>%s</b></a>", 
                   cgi, data[i].filename, data[i].filename);      	
		if (isgroup)
            printf("<td nowrap>-");
        else
            printf("<td nowrap>%12.12s", 4+Ctime(file_time(buf)));
		  /* print category */
        printf("<td nowrap>%6.6s", (isgroup?"[目录]":data[i].title+1));
		/* print display name */
        printf("</td><td width=100%%><a href=%s?board=%s><b>%s</b></a><br>", 
               (isgroup?"bbsboa":cgi), data[i].filename, data[i].title+10);
		ptr=strtok(data[i].BM, " ,;");
		if(ptr==0) ptr=(isgroup?"-":"诚征版主中");
		printf("</td><td nowrap align=center><a href=bbsqry?userid=%s><b>%s</b></a>", ptr, ptr);
		printf("</td><td nowrap align=right>%d\n", filenum(data[i].filename));
	}
    printf("</table>");

	printposttable();
	http_quit();
}
