#include "BBSLIB.inc"

int main() {
	FILE *fp;
	struct fileheader x;
	int filetime, i, total=0, total2=0, type;
	char *ptr, buf[512], path[80], dir[80];
   	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */

	if(!loginok) 
	{
		printf("<b>新信件列表 · %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("您尚未登录, 请先登录");
	}
   	printf("<b>新信件列表%s --  [使用者: %s]</b>\n", BBSNAME, currentuser.userid);
   	sprintf(dir, "mail/%c/%s/.DIR", toupper(currentuser.userid[0]), currentuser.userid);
	fp=fopen(dir, "r");
	if(fp==0) 
	{
		printpretable_lite();
		http_fatal("目前您的信箱没有任何信件");
	}
	printf("<center>\n");
   	printpretable();
    printf("<table width=100%% border=0  bgcolor=#ffffff>\n");
   	printf("<tr class=pt9h ><td><font color=white>序号<td><font color=white>状态<td><font color=white>发信人<td><font color=white>日期<td><font color=white>信件标题\n");
   	int cc=0;
	while(1) 
	{
		if(fread(&x, sizeof(x), 1, fp)<=0) 
			break;
		//added by iamfat 2002.08.10
		//check_anonymous(x.owner);
		//added end.
		total++;
		if(x.accessed[0] & FILE_READ) 
			continue;
		printf("<tr class=%s><td>%d<td>N",((cc++)%2)?"pt9dc":"pt9lc" , total);
 		ptr=strtok(x.owner, " (");
		if(ptr==0) 
			ptr=" ";
		ptr=nohtml(ptr);
		printf("<td><a href=bbsqry?userid=%s>%13.13s</a>", ptr, ptr);
		printf("<td>%6.6s", Ctime(atoi(x.filename+2))+4);
		printf("<td><a href=bbsmailcon?file=%s&num=%d>", x.filename, total-1);
	 	if(strncmp("Re: ", x.title, 4)) 
			printf("★ ");
        hprintf("%42.42s", x.title);
	 	printf(" </a>\n");
		total2++;
	}
	printf("</table>\n");
	printposttable();
	printf("</center>");
	printf("您的信箱共有%d封信件, 其中新信%d封.", total, total2);
	http_quit();
}
