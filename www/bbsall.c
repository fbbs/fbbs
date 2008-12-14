#include "libweb.h"

int cmpboard(b1, b2)
struct boardheader *b1, *b2;
{
	return strcasecmp(b1->filename, b2->filename);
}

int main() {
	struct boardheader data[MAXBOARD], *x;
	int i, total=0;
	char *ptr;
	init_all();
//	printf("<style type=text/css>A {color: #000000}</style>");
	for(i=0; i<MAXBOARD; i++) {
		x=&(bcache[i]);
		if(x->filename[0]<=32 || x->filename[0]>'z') continue;
		if(!has_read_perm(&currentuser, x->filename)) continue;
		if ((x->flag & BOARD_CLUB_FLAG)&& (x->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, x->filename)&& !isclubmember(currentuser.userid, x->filename))
		            continue;
		memcpy(&data[total], x, sizeof(struct boardheader));
		total++;
	}
	qsort(data, total, sizeof(struct boardheader), cmpboard);

	printf("<img src=/info/all/banner.jpg align=absmiddle border=0><b> %s [讨论区数: %d]</b>", BBSNAME, total);

	printf("<center>\n");
	printpretable();	       
	printf("<table width=100% bgcolor=#ffffff>\n");
	printf("<tr class=pt9h ><th nowrap>序号</th><th nowrap>讨论区名称</th><th nowrap>类别</th><th nowrap>中文描述</th><th nowrap>版主</th>\n");
	int cc=0;
	for(i=0; i<total; i++) {
		int isgroup = (data[i].flag & BOARD_DIR_FLAG)? 1 : 0; //add for dir 06.3.5 Danielfree
		if(specialboard(data[i].filename))continue;
		//special thx to fancitron 
		printf("<tr class=%s valign=top><td align=right>%d</td>", ((cc++)%2)?"pt9dc":"pt9lc" , i+1);
		  /* print board name */
        if (isgroup)
            printf("<td nowrap><b><a href=bbsboa?board=%s>[ %s ]</a></b></td>", 
                   data[i].filename, data[i].filename);
        else
            printf("<td nowrap><b><a href=bbsdoc?board=%s>%s</a></b></td>", 
                   data[i].filename, data[i].filename);

        /* print board category */
        if (isgroup)
            printf("<td  nowrap align=center><b>%6.6s</b></td>", "[目录]");
        else
            printf("<td  nowrap align=center>%6.6s</td>", data[i].title+1);

	/* print board display name */
        printf("<td width=100%%><a href=%s?board=%s><b>%s</b></a><br>", 
               (isgroup?"bbsboa":"bbsdoc"), data[i].filename, data[i].title+10);
		//printf("<td nowrap><b><a href=bbsdoc?board=%s>%s</a></b></td>", data[i].filename, data[i].filename);
		//printf("<td  nowrap align=center>%6.6s</td>", data[i].title+1);
		//printf("<td width=100%><a href=bbsdoc?board=%s><b>%s</b></a><br>", data[i].filename, data[i].title+10);
		showrecommend(data[i].filename, 3, 0);
		printf("</td>");
		/* print first BM */
        ptr=strtok(data[i].BM, " ,;");
        if(ptr==0) ptr=(isgroup)? "-":"诚征版主中";    
		//if(ptr==0) ptr="诚征版主中";
		printf("</td><td nowrap><b><a href=bbsqry?userid=%s>%s</a></b></td>", ptr, ptr);
	}
	printf("</table>");
	
	printposttable();
	printf("</center>\n");
	http_quit();
}
