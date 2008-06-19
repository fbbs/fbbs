#include "BBSLIB.inc"

int main() {
	FILE *fp;
	char buf[256], tmp[256], name[256], cname[256], ccount[256];
	int i, r;
	init_all();
	fp=fopen("0Announce/bbslist/board_day2", "r");
	if(fp==0) http_fatal("error 1");
	printf("<b><font style='font-size: 18pt'>今日热门讨论区</font> · %s </b>\n\n", BBSNAME);
	printf("<center>\n");
	printpretable();
	printf("<table width=100%% border=0>\n");
	printf("<tr class=pt9h bgcolor=#70a6ff><td><font color=white>名次<td><font color=white>版名<td><font color=white>中文版名<td><font color=white>人气\n");
	int cc=0;
	for(i=0; i<=15; i++) {
		if(fgets(buf, 150, fp)==0) break;
		if(i==0) continue;
		r=sscanf(buf, "%s %s %15s%s %s %s", tmp, tmp, name, tmp, cname, ccount);
		if(r==6) {
			printf("<tr class=%s><td>%d<td><a href=bbsdoc?board=%s>%s</a><td width=200><a href=bbsdoc?board=%s>%s</a><td>%s\n",
				((cc++)%2)?"pt9dc":"pt9lc" ,i, name, name, name, cname, ccount);
		}
	}
	printf("</table>\n");
	printposttable();
	printf("</center>\n");
	fclose(fp);
	http_quit();
}
