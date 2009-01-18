#include "libweb.h"

int bbssec_main(void)
{
	int i;
	char path[256];
	printf("<style type=text/css>A{color: #000000}A.visited{color: #000000}</style>");
	printf("<img src=/images/secbanner.jpg>\n");
	printf("<table bgcolor=#ffffff cellpadding=0 cellspacing=1 width=100%% nowrap>\n");
	for(i=0; i<SECNUM; i++) {
		printf("<td nowrap valign=top width=100%%>");
		printpretable_lite();
		printf("<font class=pt9lc><b>%X</b> <a href=bbsboa?%d>", i, i);
		sprintf(path,"%s/info/egroup%d/banner_s.jpg",BBSHOME, i);
		if(dashf(path))
			printf("<img src=/info/egroup%d/banner_s.jpg align=absmiddle border=0>",i);
		else
			printf("%s", secname[i][0]);
		printf("</a> %s", secname[i][1]);
		printf("&nbsp;&nbsp;&nbsp;<a href=bbsboa?%d>¸ü¶à°æÃæ...</a><br>", i);
		sprintf(path,"%s/info/egroup%d/recommend", BBSHOME, i);
		if(dashf(path))
		{
			printf("<table class=pt9dc width=100%%><tr><td>");
			showbrdlist(path,0);
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
