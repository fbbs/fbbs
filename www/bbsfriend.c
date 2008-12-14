#include "libweb.h"
struct user_info user[MAXACTIVE];

int cmpuser(a, b)
struct user_info *a, *b;
{
	char id1[80], id2[80];
	sprintf(id1, "%d%s", !isfriend(a->userid), a->userid);
	sprintf(id2, "%d%s", !isfriend(b->userid), b->userid);
	return strcasecmp(id1, id2);
}

int main() {
	int i, total=0, fh, shmkey, shmid; 
	struct user_info *x;
	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */

	printf("<b><font style='font-size: 18pt'>在线好友列表</font> · %s [使用者: %s]</b>\n", BBSNAME, currentuser.userid);
	for(i=0; i<MAXACTIVE; i++) {
		x=&(shm_utmp->uinfo[i]);
		if(x->active==0) continue;
		if(x->invisible && !HAS_PERM(PERM_SEECLOAK)) continue;
		if(!isfriend(x->userid)) continue;
		memcpy(&user[total], x, sizeof(struct user_info));
		total++;
	} 
	printf("<center>\n");
	printpretable();
	printf("<table border=0 width=100%% bgcolor=#ffffff>\n");
	printf("<tr class=pt9h ><td><font color=white>序号<td><font color=white>友<td><font color=white>使用者代号<td><font color=white>使用者昵称<td><font color=white>来自<td><font color=white>动态<td><font color=white>发呆\n");
	qsort(user, total, sizeof(struct user_info), cmpuser);
	int cc=0;
	for(i=0; i<total; i++) {
		int dt=(time(0)-user[i].idle_time)/60;
		//if(!isfriend(user[i].userid)) continue; move this sentence to the former for... loop for efficiency consideration by jacobson 2006.4.18
		printf("<tr class=%s><td>%d",((cc++)%2)?"pt9dc":"pt9lc" , i+1);
		printf("<td>%s", "√");
		printf("%s", user[i].invisible ? "<font color=green>C</font>" : " ");
		printf("<td><a href=bbsqry?userid=%s>%s</a>", user[i].userid, user[i].userid);
		printf("<td><a href=bbsqry?userid=%s>%24.24s </a>", user[i].userid, nohtml(user[i].username));
		printf("<td>%20.20s ", user[i].from);
		printf("<td>%s", user[i].invisible ? "隐身中..." : ModeType(user[i].mode));
		if(dt==0) {
			printf("<td> \n");
		} else {
			printf("<td>%d\n", dt);
		}
	}
	printf("</table>\n");
	if(total==0) printf("目前没有好友在线");
	printposttable();
	printf("</center>\n");
	printf("[<a href=bbsfall>全部好友名册</a>]");
	http_quit();
}
