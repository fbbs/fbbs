#include "libweb.h"

int main() {
	int i;
	int mode, destpid=0;
	char destid[20], msg[256], filename[80];
	struct userec *user;
		

	init_all();
	if(!loginok) 
	{
		printf("<b>发送消息 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("匆匆过客不能发消息, 请先登录！");
	}
/* added by roly 02.05.29 */
	if(!HAS_PERM(PERM_TALK)) 
	{
		printf("<b>发送消息 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("您是未注册用户,或者无权发送消息!");
	}
/* add end */
	strlcpy(destid, getparm("destid"), 13);
	strlcpy(msg, getparm("msg"), 51);
	destpid=atoi(getparm("destpid"));
	if(destid[0]==0 || msg[0]==0) {
		char buf3[256];
		strcpy(buf3, "<body onload='document.form0.msg.focus()'>");
		if(destid[0]==0) strcpy(buf3, "<body onload='document.form0.destid.focus()'>");
		printf("%s\n", buf3);
		printf("<b>发送消息 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		printf("<form name=form0 action=bbssendmsg method=post>\n");
		printf("<input type=hidden name=destpid value=%d>\n",destpid);
		printf("送消息给: <input name=destid maxlength=12 value='%s' size=12><br>\n",destid);
		printf("消息内容: <input name=msg maxlength=50 size=50 value='%s'><br>\n",msg);
		printf("<input type=submit value=确认 width=6></form>");
		printposttable_lite();
		http_quit();
	}
	if(getusernum(destid)<0) 
	{
		printf("<b>发送消息 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("查无此人");
	}
	printf("<body onload='document.form1.b1.focus()'>\n");
	getuser(destid);
	user = &lookupuser;
		
	sprintf(filename, "home/%c/%s/rejects", toupper(destid[0]), user->userid);
	if(file_has_word(filename, currentuser.userid))
	    http_fatal("对方不想收到您的消息");
		
	printf("<b>发送消息 · %s </b><br>\n",BBSNAME);
	printpretable_lite();

	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active)
			if(!strcasecmp(shm_utmp->uinfo[i].userid, destid)) 
			{
				/* added by roly 02.05.30 for fix the bug of in-casecensitive of destid */
				strcpy(destid,shm_utmp->uinfo[i].userid);
				/* add end */
				if(destpid!=0 && shm_utmp->uinfo[i].pid!=destpid) 
					continue;
				destpid=shm_utmp->uinfo[i].pid;
				if(!(shm_utmp->uinfo[i].pager & ALLMSG_PAGER)) 
					continue;
				if(shm_utmp->uinfo[i].invisible && !(currentuser.userlevel & PERM_SEECLOAK)) 
					continue;
				mode=shm_utmp->uinfo[i].mode;
				if(mode==BBSNET || mode==PAGE || mode== LOCKSCREEN) 
					continue;
				if(!strcasecmp(destid, currentuser.userid))
					printf("您不能给自己发讯息！");
				else 
				{
					if(send_msg(currentuser.userid, u_info->pid, destid, destpid, msg)==0) 
						printf("已经帮您送出消息");
					else
						printf("发送消息失败");
				}
				printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
				printf("<br><form name=form1><input name=b1 type=button onclick='history.go(-2)' value='[返回]'>");
				printf("</form>");
				http_quit();
			}
	printf("此人目前不在线或者无法接受消息");
    printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
	printf("<br><form name=form1><input name=b1 type=button onclick='history.go(-2)' value='[返回]'>");
	printf("</form>");
	http_quit();
}
