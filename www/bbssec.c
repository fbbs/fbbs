#include "libweb.h"

// Read board lists from file 'filename' and print line by line.
static int showbrdlist(const char *filename)
{
	struct boardheader *x;
	char board[STRLEN];
	FILE *fp;
	int showed = 0;

	fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	while (fgets(board, sizeof(board) - 1, fp)) {
		strtok(board,"\t\r\n ");
		if (board[0] == '\0')
			continue;
		x = getbcache(board);
		if (!x)
			return showed;
		if(showed > 0) {
			printf("&nbsp;");
		}
		if (x->flag & BOARD_DIR_FLAG)
			printf("<img src=/images/types/folder.gif align=absmiddle border=0> <a href=bbsboa?board=%s>%s</a>"
					, board, x->title + 10);
		else
			printf("<img src=/images/types/folder.gif align=absmiddle border=0> <a href=bbsdoc?board=%s>%s</a>"
					, board, x->title + 10);
		showed++;
	}
	fclose(fp);
	return showed;
}

int bbssec_main(void)
{
	int i;
	char path[HOMELEN];

	printf("<style type=text/css>A{color: #000000}A.visited{color: #000000}</style>");
	printf("<img src=/images/secbanner.jpg>\n");
	printf("<table bgcolor=#ffffff cellpadding=0 cellspacing=1 width=100%% nowrap>\n");
	for(i = 0; i < SECNUM; i++) {
		printf("<td nowrap valign=top width=100%%>");
		printpretable_lite();
		printf("<font class=pt9lc><b>%X</b> <a href=bbsboa?s=%d>", i, i);
		sprintf(path, "%s/info/egroup%d/banner_s.jpg", BBSHOME, i);
		if(dashf(path))
			printf("<img src=/info/egroup%d/banner_s.jpg align=absmiddle border=0>", i);
		else
			printf("%s", secname[i][0]);
		printf("</a> %s", secname[i][1]);
		printf("&nbsp;&nbsp;&nbsp;<a href=bbsboa?s=%d>¸ü¶à°æÃæ...</a><br>", i);
		sprintf(path, "%s/info/egroup%d/recommend", BBSHOME, i);
		if(dashf(path))
		{
			printf("<table class=pt9dc width=100%%><tr><td>");
			showbrdlist(path);
			printf("</td></tr></table>");
		}
		printposttable_lite();
		printf("</td>");
		printf("</tr>");
	}
	printf("</table>");
	HTTP_END;
	return 0;
}
