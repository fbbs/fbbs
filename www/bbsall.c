#include "libweb.h"

static int cmpboard(const void *b1, const void *b2)
{
	return strcasecmp((*(struct boardheader **)b1)->filename,
		(*(struct boardheader **)b2)->filename);
}

int bbsall_main(void)
{
	struct boardheader *data[MAXBOARD], *x;
	int i, total = 0;
	char *ptr;

	for(i = 0; i < MAXBOARD; i++) {
		x = &(bcache[i]);
		if (x->filename[0] <= 32 || x->filename[0] > 'z')
			continue;
		if (!hasreadperm(&currentuser, x))
			continue;
		data[total++] = x;
	}
	qsort(data, total, sizeof(struct boardheader *), cmpboard);

	printf("<img src=/info/all/banner.jpg align=absmiddle border=0><b> %s [讨论区数: %d]</b>", BBSNAME, total);
	printf("<center>\n");
	printpretable();	       
	printf("<table width=100%% bgcolor=#ffffff>\n");
	printf("<tr class=pt9h ><th nowrap>序号</th><th nowrap>讨论区名称</th><th nowrap>类别</th><th nowrap>中文描述</th><th nowrap>版主</th>\n");
	int cc = 0;
	for(i = 0; i < total; i++) {
		int isgroup = (data[i]->flag & BOARD_DIR_FLAG) ? 1 : 0;
		printf("<tr class=%s valign=top><td align=right>%d</td>",
				((cc++) % 2) ? "pt9dc" : "pt9lc", i + 1);
		// Print board name.
        if (isgroup)
            printf("<td nowrap><b><a href=bbsboa?board=%s>[ %s ]</a></b></td>", 
                   data[i]->filename, data[i]->filename);
        else
            printf("<td nowrap><b><a href=bbsdoc?board=%s>%s</a></b></td>", 
                   data[i]->filename, data[i]->filename);
		// Print category.
        if (isgroup)
			printf("<td  nowrap align=center><b>%6.6s</b></td>", "[目录]");
        else
			printf("<td  nowrap align=center>%6.6s</td>", data[i]->title + 1);
		// Print board title.
        printf("<td width=100%%><a href=%s?board=%s><b>%s</b></a><br>", 
              (isgroup?"bbsboa":"bbsdoc"), data[i]->filename, data[i]->title + 10);
		printf("</td>");
		// Print first BM.
		char buf[BM_LEN - 4];
		memcpy(buf, data[i]->BM, sizeof(buf));
        ptr = strtok(buf, " ,;");
        if (ptr == NULL)
			ptr = (isgroup)? "-" : "诚征版主中";
		printf("</td><td nowrap><b><a href=bbsqry?userid=%s>%s</a></b></td>", ptr, ptr);
	}
	printf("</table>");
	
	printposttable();
	printf("</center>\n</html>\n");

	return 0;
}
