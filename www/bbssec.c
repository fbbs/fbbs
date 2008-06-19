#include "BBSLIB.inc"

int main() {
   	int i,j;
	char path[256];
   	init_all();
	printf("<style type=text/css>A{color: #000000}A.visited{color: #000000}</style>");
  	printf("<img src=/images/secbanner.jpg>\n");
	
	//printpretable();
	/*
	printf("<table align=center border=0 cellpadding=0 cellspacing=0 width=350>\n");
	printf("	<tr height=6>\n");
	printf("		<td width=6><img border=0 src='/images/lt.gif'></td>\n");
	printf("		<td background='/images/t.gif' width=100%%></td>\n");
	printf("		<td width=6><img border=0 src='/images/rt.gif'></td>\n");
	printf("	</tr>\n");
	printf("	<tr  height=100%%>\n");
	printf("		<td width=6 background='/images/l.gif'>\n");
	printf("		<td width=100%%>\n");
	*/
   	printf("<table bgcolor=#ffffff cellpadding=0 cellspacing=1 width=100%% nowrap>\n");
   	//printf("<tr class=pt9h align=center ><td><b>区号</b></td><td><b>类别</b></td><td><b>描述</b></td></tr>\n");
	//printf("<tr><td colspan=3 width=100%%><hr noshade color=#000000 width=100%% height=1></td></tr>\n");
   	//int cc=0;
	//j=0;
	for(i=0; i<SECNUM; i++) {
		//if(j==0)printf("<tr>");
		printf("<td nowrap valign=top width=100%%>");
		printpretable_lite();
		printf("<font class=pt9lc><b>%X</b> <a href=bbsboa?%d>", i, i);
		sprintf(path,"%s/info/egroup%d/banner_s.jpg",BBSHOME, i);
		if(dashf(path))
			printf("<img src=/info/egroup%d/banner_s.jpg align=absmiddle border=0>",i);
		else
      			printf("%s", secname[i][0]);
		printf("</a> %s", secname[i][1]);
		printf("&nbsp;&nbsp;&nbsp;<a href=bbsboa?%d>更多版面...</a><br>", i);
		sprintf(path,"%s/info/egroup%d/recommend", BBSHOME, i);
		if(dashf(path))
		{
			printf("<table class=pt9dc width=100%%><tr><td>");
			showbrdlist(path,0);
			printf("</td></tr></table>");
		}
		
		printposttable_lite();
		printf("</td>");
		//if(j==1)
		//{
			printf("</tr>");
		//}
		//j=1-j;
   	}
   	printf("</table>");

	/*
	printf("		</td>\n");
	printf("		<td width=6 background='/images/r.gif'></td>\n");
	printf("	</tr>\n");
	printf("	<tr height=6>\n");
	printf("		<td width=6><img border=0 src='/images/lb.gif'></td>\n");
	printf("		<td background='/images/b.gif' width=100%%></td>\n");
	printf("		<td width=6><img border=0 src='/images/rb.gif'></td>\n");
	printf("	</tr>\n");	
   	printf("</table>\n");
	*/
	//printposttable();
	
	http_quit();
}
