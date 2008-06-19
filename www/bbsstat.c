#include "BBSLIB.inc"

int main() {
	FILE *fp;
	struct userec x;
	int logins=0, posts=0, stays=0, lifes=0, total=0;
	init_all();
	if(!loginok) http_fatal("匆匆过客不加入排名");
	fp=fopen(".PASSWDS", "r");
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		if(x.userid[0]<'A') continue;
		if(x.userlevel==0) continue;
		if(x.numposts>=currentuser.numposts) posts++;
		if(x.numlogins>=currentuser.numlogins) logins++;
		if(x.stay>=currentuser.stay) stays++;
		if(x.firstlogin<=currentuser.firstlogin) lifes++;
		total++;
	}
	fclose(fp);
	printf("<center>\n");
	printf("<font style='font-size: 18pt'>%s</font> · %s 个人排名统计\n", currentuser.userid, BBSNAME);
	
	printf("<table border=0 width=400>\n");
	printf("	<tr height=6><td background=/images/b.gif width=100%%></td></tr>\n");
	printf("	<tr><td>\n");

	printf("<table width=100%%>\n");
	printf("<tr class=pt9h><td>项目<td>数值<td>全站排名<td>相对比例\n");
	printf("<tr class=pt9lc><td>本站网龄<td>%d天<td>%d<td>TOP %5.2f%%", 
		(time(0)-currentuser.firstlogin)/86400, lifes, (lifes*100.)/total);
	printf("<tr class=pt9dc><td>上站次数<td>%d次<td>%d<td>TOP %5.2f%%",
		currentuser.numlogins, logins, logins*100./total);
	printf("<tr class=pt9lc><td>发表文章<td>%d次<td>%d<td>TOP %5.2f%%",
		currentuser.numposts, posts, posts*100./total);
	printf("<tr class=pt9dc><td>在线时间<td>%d分<td>%d<td>TOP %5.2f%%",
		currentuser.stay/60, stays, stays*100./total);
	printf("</table>");
	printposttable_lite();
	printf("<br>总用户数: %d", total);
	printf("</center>\n");
	http_quit();
}
