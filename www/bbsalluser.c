#include "libweb.h"

int main() {
	FILE *fp;
	char buf[32];
	struct userec x;
	int pos[MAXUSERS], total=0;
	int p, i, start;
	init_all();
	for(i=0; i<MAXUSERS; i++) {
		if(shm_ucache->userid[i][0]>32) {
			pos[total]=i;
			total++;
		}
	}
	printf("<center>%s -- 所有使用者列表 [用户总数: %d]<hr color=green>\n", BBSNAME, total);
	start=atoi(getparm("start"));
	if(start>total || start<0) start=0;
	printf("<table width=610 border=1>\n");
	printf("<tr><td>编号<td>好友<td>使用者代号<td>昵称<td>上站次数<td>文章数<td>最近光临时间\n");
	fp=fopen(".PASSWDS", "r");
	for(i=0; i<20; i++) {
		if(start+i>total-1) break;
		p=pos[start+i];
		fseek(fp, sizeof(struct userec)*p, SEEK_SET);
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		printf("<tr><td>%d<td>%s<td><a href=bbsqry?userid=%s>%s</a><td>%s<td>%d<td>%d<td>%s\n",
			start+i+1, isfriend(x.userid) ? "<font color=green>√</font>" : " ", x.userid, 
			x.userid, nohtml(x.username), x.numlogins, x.numposts, Ctime(x.lastlogin)+4);
	}
	printf("</table><hr color=green>\n");
	if(start>0) printf("[<a href=bbsalluser?start=0>第一页</a>] ");
	if(start>0) printf("[<a href=bbsalluser?start=%d>上一页</a>] ", start-20<0 ? 0 : start-20);
	if(start<total-19) printf("[<a href=bbsalluser?start=%d>下一页</a>]", start+20);
	if(start<total-19) printf("[<a href=bbsalluser?start=%d>最后一页</a>]\n", total-19);
	printf("<form action=bbsalluser>\n");
	printf("<input type=submit value=跳转到>");
	printf("第<input name=start type=text size=5>个使用者\n");
	printf("</form>\n");
}
