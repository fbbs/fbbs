#include "libweb.h"

// TODO: telnet->web
static int send_msg(const char *myuserid, int mypid, const char *touserid, int topid, char *msg)
{
	char msgbuf[256];
	for (int i = 0; i < strlen(msg); i++) {
		if (msg[i] <= 27)
			msg[i] = ' ';
	}
	if (mypid <= 0)
		return -1;
	snprintf(msgbuf, sizeof(msgbuf), "\033[0;1;44;36m%-12.12s\033[33m(\033[36m"
			"%-5.5s\033[33m):\033[37m%-54.54s\033[31m(^Z回)\033[m\033[%05dm\n",
			myuserid, getdatestring(time(NULL), DATE_SHORT) + 6, msg, mypid);
	char fname[HOMELEN];
	sethomefile(fname, touserid, "msgfile");
	file_append(fname, msgbuf);
	sethomefile(fname, touserid, "msgfile.me");
	file_append(fname, msgbuf);
	sethomefile(fname, myuserid, "msgfile.me");
	snprintf(msgbuf, sizeof(msgbuf), "\033[1;32;40mTo \033[1;33;40m%-12.12s"
			"\033[m (%-5.5s):%-55.55s\n", touserid,
			getdatestring(time(NULL), DATE_SHORT) + 6, msg);
	file_append(fname, msgbuf); 
	if (topid <= 0)
		return -1;
	kill(topid, SIGTTOU);
	kill(topid, SIGUSR2);
	return 0;
}

int bbssendmsg_main(void)
{
	if (!loginok)
		http_fatal("匆匆过客不能发消息, 请先登录！");
	if (!HAS_PERM(PERM_TALK))
		http_fatal("您是未注册用户,或者无权发送消息！");

	parse_post_data();
	char *destid = getparm("id");
	char *msg = getparm("msg");
	int destpid = strtol(getparm("pid"), NULL, 10);
	if (*destid == '\0' || *msg == '\0') {
		xml_header("bbssendmsg");
		puts("<bbssendmsg>null</bbssendmsg>");
		return 0;
	}
	if (!strcasecmp(destid, currentuser.userid))
		http_fatal("您不能给自己发讯息！");
	int destuid = searchuser(destid);
	if (!destuid)
		http_fatal("查无此人");
	// TODO: check blacklist
	xml_header("bbssendmsg");
	struct user_info *user = utmpshm->uinfo;
	for (int i = 0; i < MAXACTIVE; ++i, ++user) {
		if (user->active && user->uid == destuid) {
			if (destpid != 0 && user->pid != destpid)
				continue;
			destpid = user->pid;
			if (!(user->pager & ALLMSG_PAGER))
				continue;
			if (user->invisible && !HAS_PERM(PERM_SEECLOAK))
				continue;
			int mode = user->mode;
			if (mode == BBSNET || mode == PAGE || mode == LOCKSCREEN) {
				continue;
			} else {
				if (send_msg(currentuser.userid, u_info->pid, destid, destpid, msg) == 0)
					puts("<bbssendmsg>success</bbssendmsg>");
				else
					puts("<bbssendmsg>fail</bbssendmsg>");
				break;
			}
		}			
	}
	return 0;
}
