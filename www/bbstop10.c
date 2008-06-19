#include "BBSLIB.inc"

int main() {
	FILE *fp;
	int n;
	char s1[256], s2[256], s3[256], s4[256],s5;
	char brd[256], id[256], title[256], num[100];
	init_all();
	printf("<b><font style='font-size: 18pt'>今日十大热门话题</font> · %s </b>\n\n", BBSNAME);
	fp=fopen("etc/posts/day", "r");
	if(fp==0) http_fatal("can't read data");
	fgets(s1, 255, fp);
	fgets(s1, 255, fp);
	
	printf("<center>\n");
	printpretable();
	printf("<table border=0 width=100%%>\n");
	printf("<tr class=pt9h bgcolor=#cccccc align=center><td nowrap><b>名次</b></td><td nowrap><b>讨论区</b></td><td nowrap><b>标题</b></td><td nowrap><b>作者</b></td><td nowrap><b>篇数</b></td></tr>\n");
	int cc=0;
	for(n=1; n<=10; n++) 
	{
		if(fgets(s1, 255, fp)<=0) 
			break;
		sscanf(s1+45, "%s", brd);
		sscanf(s1+122, "%s", id);
		sscanf(s1+105, "%s", num);
		if(fgets(s1, 255, fp)<=0) 
			break;
		strsncpy(title, s1+27, 60);
		printf("<tr class=%s><td nowrap>第 %2d 名</td><td nowrap><a href=bbsdoc?board=%s><b>%s</b></a></td><td width=100\%><a href='bbstfind?board=%s&title=%s'>%42.42s</a></td><td nowrap align=center><a href=bbsqry?userid=%s><b>%12s</b></a></td><td nowrap>%s</td></tr>\n",
			((cc++)%2)?"pt9dc":"pt9lc" ,n, brd, brd, brd, nohtml(title), nohtml(title), id, id, num);
	}
	printf("</table>");
	printposttable();
	printf("</center>\n");
	http_quit();
}
