/*
 Pirate Bulletin Board System
 Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
 Eagles Bulletin Board System
 Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
 Guy Vega, gtvega@seabass.st.usm.edu
 Dominic Tynes, dbtynes@seabass.st.usm.edu
 Firebird Bulletin Board System
 Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
 Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */
/*
 $Id: sendmsg.c 339 2006-11-26 12:02:02Z danielfree $
 */

#include "bbs.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern int RMSG;
extern int msg_num;
char buf2[STRLEN];
struct user_info *t_search();
void count_msg();
void r_msg();

int get_msg(char *uid, char * msg, int line) {
	char genbuf[3];
	move(line, 0);
	clrtoeol();
	prints("ËÍÒôĞÅ¸ø£º%s", uid);
	memset(msg, 0, sizeof(msg));
	while (1) {
		getdata(line + 1, 0, "ÒôĞÅ : ", msg, 55, DOECHO, NA);
		if (msg[0] == '\0')
			return NA;
		getdata(line + 2, 0, "È·¶¨ÒªËÍ³öÂğ(Y)ÊÇµÄ (N)²»Òª (E)ÔÙ±à¼­? [Y]: ", genbuf, 2,
				DOECHO, YEA);
		if (genbuf[0] == 'e' || genbuf[0] == 'E')
			continue;
		if (genbuf[0] == 'n' || genbuf[0] == 'N')
			return NA;
		else
			return YEA;
	}//while
}

char msgchar(struct user_info *uin) {
	//added by Ashinmarch: uin->mode == LOCKSCREEN to show the P M status on 07.11.5
	if (uin->mode==FIVE||uin->mode==BBSNET || uin->mode == LOCKSCREEN)
		return '@';
	if (isreject(uin))
		return '*';
	if ((uin->pager & ALLMSG_PAGER))
		return ' ';
	if (hisfriend(uin)) {
		if ((uin->pager & FRIENDMSG_PAGER))
			return 'O';
		else
			return '#';
	}
	return '*';
}

int canmsg(struct user_info *uin) {
	if (isreject(uin))
		return NA;
	if ((uin->pager & ALLMSG_PAGER) || HAS_PERM(PERM_OCHAT))
		return YEA;
	if ((uin->pager & FRIENDMSG_PAGER) && hisfriend(uin))
		return YEA;
	return NA;
}
void s_msg() {
	do_sendmsg(NULL, NULL, 0, 0);
}

int send_msg(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct user_info* uin;
	if(!strcmp(currentuser.userid,"guest"))
	return DONOTHING;
	uin = t_search(fileinfo->owner, NA);
	if (uin == NULL ||(uin->invisible && !HAS_PERM(PERM_SEECLOAK))) {
		move(2, 0);
		prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ...\n");
		pressreturn();
	} else {
		do_sendmsg(uin, NULL, 0, uin->pid);
	}
	return FULLUPDATE;
}

int show_allmsgs() {
	char fname[STRLEN];
	if (!strcmp(currentuser.userid, "guest"))
		return;
#ifdef LOG_MY_MESG
	setuserfile(fname, "msgfile.me");
#else
	setuserfile(fname, "msgfile");
#endif
	clear();
	modify_user_mode(LOOKMSGS);
	if (dashf(fname)) {
		mesgmore(fname, YEA, 0, 9999);
		clear();
	} else {
		move(5, 30);
		prints("Ã»ÓĞÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡");
		pressanykey();
		clear();
	}//if dashf(fname)
}

int do_sendmsg(struct user_info *uentp, char msgstr[256], int mode,
		int userpid) {
	char uident[STRLEN], ret_str[20];
	time_t now;
	struct user_info *uin;
	char buf[80], *msgbuf, *timestr;
#ifdef LOG_MY_MESG
	char *mymsg, buf2[80];
	int ishelo = 0; /* ÊÇ²»ÊÇºÃÓÑÉÏÕ¾Í¨ÖªÑ¶Ï¢ */
	mymsg = (char *) malloc(256);
#endif
	msgbuf = (char *) malloc(256);

	if (mode == 0) {
		move(2, 0);
		clrtobot();
		/*
		 if (uinfo.invisible && !HAS_PERM(PERM_SYSOP)) {
		 move(2, 0);
		 prints("±§Ç¸, ´Ë¹¦ÄÜÔÚÒşÉí×´Ì¬ÏÂ²»ÄÜÖ´ĞĞ...\n");
		 pressreturn();
		 return 0;
		 }
		 */
		modify_user_mode(MSG);
	}
	if (uentp == NULL) {
		prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n");
		move(1, 0);
		clrtoeol();
		prints("ËÍÑ¶Ï¢¸ø: ");
		creat_list();
		namecomplete(NULL, uident);
		if (uident[0] == '\0') {
			clear();
			return 0;
		}
		if (!strcasecmp(uident, "guest")) {
			clear();
			return 0;
		}
		uin = t_search(uident, NA);
		if (uin == NULL) {
			move(2, 0);
			prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ...\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
		if (uin->mode == BBSNET ||uin->mode	== LOCKSCREEN
			||uin->mode == GAME || uin->mode == PAGE
			||uin->mode == FIVE || !canmsg(uin)) {
			move(2, 0);
			prints("Ä¿Ç°ÎŞ·¨´«ËÍÑ¶Ï¢¸ø¶Ô·½.\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
	} else {
		if (uentp->uid == usernum)
			return 0;
		uin = uentp;
		if (uin->mode == BBSNET || uin->mode == GAME
			|| uin->mode == PAGE || uin->mode == LOCKSCREEN
			|| uin->mode == FIVE || (mode != 2 && !canmsg(uin)))
			return 0;
		strcpy(uident, uin->userid);
	}
	if (msgstr == NULL) {
		if (!get_msg(uident, buf, 1)) {
			move(1, 0);
			clrtoeol();
			move(2, 0);
			clrtoeol();
			return 0;
		}
	}
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 8) = '\0';
	strcpy(ret_str, "^Z»Ø");
	if (msgstr == NULL || mode == 2) {
		sprintf(
				msgbuf,
				"[0;1;44;36m%-12.12s[33m([36m%-5.5s[33m):[37m%-54.54s[31m(%s)[m[%05dm\n",
				currentuser.userid, timestr, (msgstr == NULL) ? buf
						: msgstr, ret_str, uinfo.pid);
#ifdef LOG_MY_MESG
		//sprintf(mymsg, "[1;32;40mTo [1;33;40m%-12.12s[m (%-5.5s):%-58.58s\n",
		//modified by iamfat 02.05.14
		sprintf(mymsg, "[1;32;40mTo [1;33;40m%-12.12s[m(%-5.5s):%-57.57s\n",
				uin->userid, timestr, (msgstr == NULL) ? buf : msgstr);

		sprintf(buf2, "ÄãµÄºÃÅóÓÑ %s ÒÑ¾­ÉÏÕ¾ÂŞ£¡", currentuser.userid);

		if (msgstr != NULL)
		if (strcmp(msgstr, buf2) == 0)
		ishelo = 1;
		else if (strcmp(buf, buf2) == 0)
		ishelo = 1;
#endif
	} else if (mode == 0) {
		sprintf(
				msgbuf,
				"[0;1;5;44;33mÕ¾³¤ ì¶[36m %8.8s [33m¹ã²¥£º[m[1;37;44m%-57.57s[m[%05dm\n",
				timestr, msgstr, uinfo.pid);
	} else if (mode == 1) {
		sprintf(
				msgbuf,
				"[0;1;44;36m%-12.12s[37m([36m%-5.5s[37m) ÑûÇëÄã[37m%-48.48s[31m(%s)[m[%05dm\n",
				currentuser.userid, timestr, msgstr, ret_str, uinfo.pid);
	} else if (mode == 3) {
		sprintf(
				msgbuf,
				"[0;1;45;36m%-12.12s[33m([36m%-5.5s[33m):[37m%-54.54s[31m(%s)[m[%05dm\n",
				currentuser.userid, timestr, (msgstr == NULL) ? buf
						: msgstr, ret_str, uinfo.pid);
	}
	/* added by roly 02.06.02 for logoff msg */
	else if (mode == 4) {
		sprintf(
				msgbuf,
				"[0;1;45;36m%-12.12s[36mÏòÄú¸æ±ğ([1;36;45m%8.8s[36m)£º[m[1;36;45m%-48.48s[m[%05dm\n",
				currentuser.userid, timestr, msgstr, 0);
	}
	/* add end */
	if (userpid) {
		if (userpid != uin->pid) {
			saveline(0, 0); /* Save line */
			move(0, 0);
			clrtoeol();
			prints("[1m¶Ô·½ÒÑ¾­ÀëÏß...[m\n");
			sleep(1);
			saveline(0, 1); /* restore line */
			return -1;
		}
	}
	if (!uin->active || bbskill(uin->pid, 0) == -1) {
		if (msgstr == NULL) {
			prints("\n¶Ô·½ÒÑ¾­ÀëÏß...\n");
			pressreturn();
			clear();
		}
		return -1;
	}
	sethomefile(buf, uident, "msgfile");
	file_append(buf, msgbuf);

#ifdef LOG_MY_MESG
	/*
	 * 990610.edwardc ³ıÁËÎÒÖ±½ÓËÍÑ¶Ï¢¸øÄ³ÈËÍâ, ÆäËûÈç¹ã²¦¸øÕ¾ÉÏ
	 * ²¦ºÃÓÑÒ»ÂÉ²»¼ÍÂ¼ .. :)
	 */

	if (mode == 2 || (mode == 0 && msgstr == NULL)) {
		if (ishelo == 0) {
			sethomefile(buf, currentuser.userid, "msgfile.me");
			file_append(buf, mymsg);
		}
	}
	sethomefile(buf, uident, "msgfile.me");
	file_append(buf, msgbuf);
	free(mymsg);

#endif
	free(msgbuf);
	if (uin->pid) {
		bbskill(uin->pid, SIGUSR2);
	}
	if (msgstr == NULL) {
		prints("\nÒÑËÍ³öÑ¶Ï¢...\n");
		pressreturn();
		clear();
	}
	return 1;
}

int dowall(struct user_info *uin) {
	if (!uin->active || !uin->pid)
		return -1;
	move(1, 0);
	clrtoeol();
	prints("[1;32mÕı¶Ô %s ¹ã²¥.... Ctrl-D Í£Ö¹¶Ô´ËÎ» User ¹ã²¥¡£[m", uin->userid);
	refresh();
	do_sendmsg(uin, buf2, 0, uin->pid);
}

int
myfriend_wall(uin)
struct user_info *uin;
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))
	return -1;
	if (myfriend(uin->uid)) {
		move(1, 0);
		clrtoeol();
		prints("[1;32mÕıÔÚËÍÑ¶Ï¢¸ø %s...  [m", uin->userid);
		refresh();
		do_sendmsg(uin, buf2, 3, uin->pid);
	}
}

/* added by roly 02.06.02 for logoff msg */
int
hisfriend_wall_logout(uin)
struct user_info *uin;
{
	/* Modified by Amigo 2002.04.03. Add logoff msg reject. */
	/*	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))*/
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin) || !(uin->pager & LOGOFFMSG_PAGER))
	return -1;
	if (hisfriend(uin)) {
		refresh();
		do_sendmsg(uin, buf2, 4, uin->pid);
	}
}

/* add end */

int
hisfriend_wall(uin)
struct user_info *uin;
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))
	return -1;
	if (hisfriend(uin)) {
		/*
		 move(1, 0);
		 clrtoeol();
		 prints("[1;32mÕıÔÚËÍÑ¶Ï¢¸ø %s...  [m", uin->userid);
		 *///commented by roly 02.03.29
		refresh();
		do_sendmsg(uin, buf2, 3, uin->pid);
	}
}

int friend_wall() {
	char buf[3];
	char msgbuf[256], filename[80];
	time_t now;
	char *timestr;
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 8) = '\0';

	/* 
	 if (uinfo.invisible) {
	 move(2, 0);
	 prints("±§Ç¸, ´Ë¹¦ÄÜÔÚÒşÉí×´Ì¬ÏÂ²»ÄÜÖ´ĞĞ...\n");
	 pressreturn();
	 return 0;
	 }
	 */
	modify_user_mode(MSG);
	move(2, 0);
	clrtobot();
	getdata(1, 0, "ËÍÑ¶Ï¢¸ø [1] ÎÒµÄºÃÅóÓÑ£¬[2] ÓëÎÒÎªÓÑÕß: ", buf, 2, DOECHO, YEA);
	switch (buf[0]) {
		case '1':
			if (!get_msg("ÎÒµÄºÃÅóÓÑ", buf2, 1))
				return 0;
			if (apply_ulist(myfriend_wall) == -1) {
				move(2, 0);
				prints("ÏßÉÏ¿ÕÎŞÒ»ÈË\n");
				pressanykey();
			} else {
				/* ¼ÇÂ¼ËÍÑ¶Ï¢¸øºÃÓÑ */
				sprintf(
						msgbuf,
						"[0;1;45;36mËÍÑ¶Ï¢¸øºÃÓÑ[33m([36m%-5.5s[33m):[37m%-54.54s[31m(^Z»Ø)[m[%05dm\n",
						timestr, buf2, uinfo.pid);
				setuserfile(filename, "msgfile.me");
				file_append(filename, msgbuf);
			}
			break;
		case '2':
			if (!get_msg("ÓëÎÒÎªÓÑÕß", buf2, 1))
				return 0;
			if (apply_ulist(hisfriend_wall) == -1) {
				move(2, 0);
				prints("ÏßÉÏ¿ÕÎŞÒ»ÈË\n");
				pressanykey();
			} else {
				/* ¼ÇÂ¼ËÍÑ¶Ï¢¸øÓëÎÒÎªÓÑÕß */
				sprintf(
						msgbuf,
						"[0;1;45;36mËÍ¸øÓëÎÒÎªÓÑ[33m([36m%-5.5s[33m):[37m%-54.54s[31m(^Z»Ø)[m[%05dm\n",
						timestr, buf2, uinfo.pid);
				setuserfile(filename, "msgfile.me");
				file_append(filename, msgbuf);

			}
			break;
		default:
			return 0;
	}
	move(6, 0);
	prints("Ñ¶Ï¢´«ËÍÍê±Ï...");
	pressanykey();
	return 1;
}

void r_msg2() {
	FILE *fp;
	char buf[256];
	char msg[256];
	char fname[STRLEN];
	int line, tmpansi;
	int y, x, ch, i, j;
	int MsgNum;
	struct sigaction act;

	if (!strcmp(currentuser.userid, "guest"))
		return;
	getyx(&y, &x);
	if (uinfo.mode == TALK)
		line = t_lines / 2 - 1;
	else
		line = 0;
	setuserfile(fname, "msgfile");
	i = get_num_records(fname, 129);
	if (i == 0)
		return;
	signal(SIGUSR2, count_msg);
	tmpansi = showansi;
	showansi = 1;
	oflush();
	if (RMSG == NA) {
		saveline(line, 0);
		saveline(line + 1, 2);
	}
	MsgNum = 0;
	RMSG = YEA;
	while (1) {
		MsgNum = (MsgNum % i);
		if ((fp = fopen(fname, "r")) == NULL) {
			RMSG = NA;
			if (msg_num)
				r_msg();
			else {
				sigemptyset(&act.sa_mask);
				act.sa_flags = SA_NODEFER;
				act.sa_handler = r_msg;
				sigaction(SIGUSR2, &act, NULL);
			}
			return;
		}
		for (j = 0; j < (i - MsgNum); j++) {
			if (fgets(buf, 256, fp) == NULL)
				break;
			else
				strcpy(msg, buf);
		}
		fclose(fp);
		move(line, 0);
		clrtoeol();
		prints("%s", msg);
		refresh();
		{
			struct user_info *uin;
			char msgbuf[STRLEN];
			int good_id, send_pid;
			char *ptr, usid[STRLEN];
			ptr = strrchr(msg, '[');
			send_pid = atoi(ptr + 1);

			ptr = strtok(msg + 12, " [");
			if (ptr == NULL)
				good_id = NA;
			else if (!strcmp(ptr, currentuser.userid))
				good_id = NA;
			else {
				/*	
				 strcpy(usid, ptr);
				 uin = t_search(usid, send_pid);
				 if (uin == NULL)
				 good_id = NA;
				 else
				 good_id = YEA;
				 }
				 if (good_id == YEA && canmsg(uin)) {
				 *///modified by roly 02.05.19 for reply ones msg with PAGE OFF
				strcpy(usid, ptr);
				uin = t_search(usid, send_pid);
				/* added by roly 02.06.02 for disable reply logout msg*/
				if (send_pid==0)
					send_pid=-1;
				/* add end */
				good_id = NA; // change 2 lines by quickmouse 2002-5-15 for reply who's msg off
				if (uin != NULL && (uin->pid == send_pid || canmsg(uin) ))
					good_id = YEA;
			}
			if (good_id == YEA /*&& canmsg(uin)*/) { // comment by quickmouse 2002-5-15 for reply who's msg off 
				//modified end
				int userpid;
				userpid = uin->pid;
				move(line + 1, 0);
				clrtoeol();
				sprintf(msgbuf, "»ØÑ¶Ï¢¸ø %s: ", usid);
				getdata(line + 1, 0, msgbuf, buf, 55, DOECHO, YEA);
				if (buf[0] == Ctrl('Z')) {
					MsgNum++;
					continue;
				} else if (buf[0] == Ctrl('A')) {
					MsgNum--;
					if (MsgNum < 0)
						MsgNum = i - 1;
					continue;
				}
				if (buf[0] != '\0') {
					if (do_sendmsg(uin, buf, 2, userpid) == 1)
						sprintf(msgbuf, "[1;32m°ïÄúËÍ³öÑ¶Ï¢¸ø %s ÁË![m", usid);
					else
						sprintf(msgbuf, "[1;32mÑ¶Ï¢ÎŞ·¨ËÍ³ö.[m");
				} else
					sprintf(msgbuf, "[1;33m¿ÕÑ¶Ï¢, ËùÒÔ²»ËÍ³ö.[m");
				move(line + 1, 0);
				clrtoeol();
				refresh();
				prints("%s", msgbuf);
				refresh();
				if (!strstr(msgbuf, "°ïÄú"))
					sleep(1);
			} else {
				sprintf(
						msgbuf,
						"[1;32mÕÒ²»µ½·¢Ñ¶Ï¢µÄ %s! Çë°´ÉÏ:[^Z ¡ü] »òÏÂ:[^A ¡ı] »òÆäËû¼üÀë¿ª.[m",
						usid);
				move(line + 1, 0);
				clrtoeol();
				refresh();
				prints("%s", msgbuf);
				refresh();
				if ((ch = igetkey()) == Ctrl('Z') || ch == KEY_UP) {
					MsgNum++;
					continue;
				}
				if (ch == Ctrl('A') || ch == KEY_DOWN) {
					MsgNum--;
					if (MsgNum < 0)
						MsgNum = i - 1;
					continue;
				}
			}
		}
		break;
	}
	saveline(line, 1);
	saveline(line + 1, 3);
	showansi = tmpansi;
	move(y, x);
	refresh();
	RMSG = NA;
	if (msg_num)
		r_msg();
	else {
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_NODEFER;
		act.sa_handler = r_msg;
		sigaction(SIGUSR2, &act, NULL);
	}
	return;
}

void count_msg() {
	signal(SIGUSR2, count_msg);
	msg_num++;
}

void r_msg() {
	FILE *fp;
	char buf[256];
	char mustbak_title[80];
	char msg[256];
	char fname[STRLEN];
	int line, tmpansi;
	int y, x, i, j, premsg;
	char ch;
	struct sigaction act;

	signal(SIGUSR2, count_msg);
	msg_num++;
	getyx(&y, &x);
	tmpansi = showansi;
	showansi = 1;
	if (uinfo.mode == TALK)
		line = t_lines / 2 - 1;
	else
		line = 0;
	if (DEFINE(DEF_MSGGETKEY)) {
		oflush();
		saveline(line, 0);
		premsg = RMSG;
	}
	while (msg_num) {
		if (DEFINE(DEF_SOUNDMSG)) {
			bell();
		}
		setuserfile(fname, "msgfile");
		i = get_num_records(fname, 129);
		if ((fp = fopen(fname, "r")) == NULL) {
			sigemptyset(&act.sa_mask);
			act.sa_flags = SA_NODEFER;
			act.sa_handler = r_msg;
			sigaction(SIGUSR2, &act, NULL);
			return;
		}
		for (j = 0; j <= (i - msg_num); j++) {
			if (fgets(buf, 256, fp) == NULL)
				break;
			else
				strcpy(msg, buf);
		}
		fclose(fp);
		move(line, 0);
		clrtoeol();
		prints("%s", msg);
		refresh();
		msg_num--;
		if (DEFINE(DEF_MSGGETKEY)) {
			RMSG = YEA;
			ch = 0;
#ifdef MSG_CANCLE_BY_CTRL_C
			while (ch != Ctrl('C'))
#else
			while (ch != '\r' && ch != '\n')
#endif
			{
				ch = igetkey();
#ifdef MSG_CANCLE_BY_CTRL_C
				if (ch == Ctrl('C'))
				break;
#else
				if (ch == '\r' || ch == '\n')
					break;
#endif
				else if (ch == Ctrl('R') || ch == 'R' || ch == 'r' || ch
						== Ctrl('Z')) {
					struct user_info *uin;
					char msgbuf[STRLEN];
					int good_id, send_pid;
					char *ptr, usid[STRLEN];
					ptr = strrchr(msg, '[');
					send_pid = atoi(ptr + 1);
					/* added by roly 02.06.02 for disable reply logout msg*/
					if (send_pid==0)
						send_pid=-1;
					/* add end */
					ptr = strtok(msg + 12, " [");
					if (ptr == NULL)
						good_id = NA;
					else if (!strcmp(ptr, currentuser.userid))
						good_id = NA;
					else {
						strcpy(usid, ptr);
						uin = t_search(usid, send_pid);
						if (uin == NULL)
							good_id = NA;
						else
							good_id = YEA;
					}
					oflush();
					saveline(line + 1, 2);
					if (good_id == YEA) {
						int userpid;
						userpid = uin->pid;
						move(line + 1, 0);
						clrtoeol();
						sprintf(msgbuf, "Á¢¼´»ØÑ¶Ï¢¸ø %s: ", usid);
						getdata(line + 1, 0, msgbuf, buf, 55, DOECHO, YEA);
						if (buf[0] != '\0' && buf[0] != Ctrl('Z')
								&& buf[0] != Ctrl('A')) {
							if (do_sendmsg(uin, buf, 2, userpid))
								sprintf(msgbuf, "[1;32m°ïÄúËÍ³öÑ¶Ï¢¸ø %s ÁË![m",
										usid);
							else
								sprintf(msgbuf, "[1;32mÑ¶Ï¢ÎŞ·¨ËÍ³ö.[m");
						} else
							sprintf(msgbuf, "[1;33m¿ÕÑ¶Ï¢, ËùÒÔ²»ËÍ³ö. [m");
					} else {
						sprintf(msgbuf, "[1;32mÕÒ²»µ½·¢Ñ¶Ï¢µÄ %s.[m", usid);
					}
					move(line + 1, 0);
					clrtoeol();
					refresh();
					prints("%s", msgbuf);
					refresh();
					if (!strstr(msgbuf, "°ïÄú"))
						sleep(1);
					saveline(line + 1, 3);
					refresh();
					break;
				} /* if */
			} /* while */
		} /* if */
	} /* while */

	setuserfile(fname, "msgfile.me");
	i = get_num_records(fname, 129);
	if (i>500) {
		char bak_title[STRLEN];
		getdatestring(time(0), NA);
		sprintf(mustbak_title, "[%s] Ç¿ÖÆÑ¶Ï¢±¸·İ%dÌõ", datestring, i);
		strlcpy(bak_title, save_title, STRLEN-1);
		bak_title[STRLEN-1]=0;
		mail_file(fname, currentuser.userid, mustbak_title);
		strcpy(save_title, bak_title);
		unlink(fname);
	}

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = r_msg;
	sigaction(SIGUSR2, &act, NULL);

	if (DEFINE(DEF_MSGGETKEY)) {
		RMSG = premsg;
		saveline(line, 1);
	}
	showansi = tmpansi;
	move(y, x);
	refresh();
	return;
}

int
friend_login_wall(pageinfo)
struct user_info *pageinfo;
{
	char msg[STRLEN];
	int x, y;
	if (!pageinfo->active || !pageinfo->pid || isreject(pageinfo))
	return 0;
	if (hisfriend(pageinfo)) {
		if (getuser(pageinfo->userid) <= 0)
		return 0;
		if (!(lookupuser.userdefine & DEF_LOGINFROM))
		return 0;
		if (pageinfo->uid ==usernum)
		return 0;
		/* edwardc.990427 ºÃÓÑÒşÉí¾Í²»ÏÔÊ¾ËÍ³öÉÏÕ¾Í¨Öª */
		if (pageinfo->invisible)
		return 0;
		getyx(&y, &x);
		if (y> 22) {
			pressanykey();
			move(7, 0);
			clrtobot();
		}
		prints("ËÍ³öºÃÓÑÉÏÕ¾Í¨Öª¸ø %s\n", pageinfo->userid);
		sprintf(msg, "ÄãµÄºÃÅóÓÑ %s ÒÑ¾­ÉÏÕ¾ÂŞ£¡", currentuser.userid);
		do_sendmsg(pageinfo, msg, 2, pageinfo->pid);
	}
	return 0;
}
