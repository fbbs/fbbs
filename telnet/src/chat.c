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
 $Id: chat.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include "bbs.h"

#ifdef lint
#include <sys/uio.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "chat.h"

char chatname[IDLEN]; /* Chat-Room Name */
int chatline; /* Where to display message now */
int stop_line; /* next line of bottom of message window area */
FILE* rec;
int recflag = 0;
char buftopic[STRLEN];
char chat_station[19];

extern char BoardName[];
extern char page_requestor[];
extern int talkrequest;
extern char *cexpstr();
extern char pagerchar();
extern void t_pager();
void set_rec();
struct user_info * t_search();
#define b_lines t_lines-1
#define cuser currentuser
char *msg_seperator = "¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ªCtrl+C ÍË³ö  /h ²é¿´°ïÖú¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª";
char
		*msg_shortulist =
				"[1;33;44m\
                         Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬ [m";

struct chat_command { //ÃüÁîÃû¼°Æä¶ÔÓ¦º¯Êý
	char *cmdname; /* Char-room command length */
	void (*cmdfunc)(); /* Pointer to function */
};
/* After This *//* KCN alias emote , 1999.11.25 */
struct chatalias {
	char cmd[9];
	char action[81];
};

struct chatalias * chat_aliases;
int chat_alias_count;

// µ¼ÈëÁÄÌìÊ± Ò»Ð©ÊôÐÔ(¿ì½Ý)
void chat_load_alias() {
	char buf[256];
	int i;

	chat_alias_count=0;
	chat_aliases = (struct chatalias *)malloc(sizeof(struct chatalias)
			*MAXDEFINEALIAS);
	for (i=0; i<MAXDEFINEALIAS; i++) {
		chat_aliases[i].cmd[0]=0;
	}
	setuserfile(buf, "chatalias");
	chat_alias_count = get_num_records(buf, sizeof(struct chatalias));
	if (chat_alias_count>MAXDEFINEALIAS) {
		chat_alias_count = MAXDEFINEALIAS;
	}
	if (chat_alias_count!=0) {
		get_records(buf, chat_aliases, sizeof(struct chatalias), 1,
				chat_alias_count);
	}
	for (i=0; i<chat_alias_count; i++) {
		if (chat_aliases[i].cmd[0]==0) {
			chat_alias_count=i;
			break;
		}
		chat_aliases[i].cmd[8]=0;
		chat_aliases[i].action[80]=0;
	}
}
/* Above All *//* KCN alias emote , 1999.11.25 */
void transPERstr(char *str, char *tmpstr) {
	char* p;
	int i;

	p = str;
	i = 0;
	memset(tmpstr, 0, strlen(tmpstr));
	while ( *p != 0) {
		if (*p=='%') {
			if ( *(p+1) == 0) {
				tmpstr[i] = '%';
				i++;
				p++;
			} else if ( *(p+1) == '%') {
				tmpstr[i] = '%';
				i++;
				p++;
				p++;
			} else if ( *(p+1)>'0' && *(p+1)<='7') {
				tmpstr[i++] = 27;
				tmpstr[i++] = '[';
				tmpstr[i++] = '3';
				tmpstr[i++] = *(p+1);
				tmpstr[i++] = 'm';
				p++;
				p++;
			} else if ( *(p+1) == '0') {
				tmpstr[i++] = 27;
				tmpstr[i++] = '[';
				tmpstr[i++] = '0';
				tmpstr[i++] = 'm';
				p++;
				p++;
			} else {
				tmpstr[i] = '%';
				i++;
				p++;
				tmpstr[i] = *p;
				i++;
				p++;
			}
		} else {
			tmpstr[i] = *p;
			i++;
			p++;
		}
	}
	tmpstr[i++] = 27;
	tmpstr[i++] = '[';
	tmpstr[i++] = '3';
	tmpstr[i++] = '7';
	tmpstr[i++] = 'm';
	tmpstr[i] = 0;
	return;
}

void printchatline(char *str) {
	char tmpstr[256];
	transPERstr(str, tmpstr);
	move(chatline, 0);
	clrtoeol();
	outs(tmpstr);
	outc('\n');
	if (recflag == 1) {
		fprintf(rec, "%s\n", str);
	}
	if (++chatline == stop_line) {
		chatline = 2;
	}
	move(chatline, 0);
	clrtoeol();
	outs("¡ú");
}

void chat_clear() {
	for (chatline = 2; chatline < stop_line; chatline++) {
		move(chatline, 0);
		clrtoeol();
	}
	chatline = stop_line - 1;
	printchatline("");
}

void print_chatid(char * chatid) {
	move(b_lines, 0);
	outs(chatid);
	outc(':');
}

int chat_send(int fd, char * buf) {
	int len;
	sprintf(genbuf, "%s\n", buf);
	len = strlen(genbuf);
	return (send(fd, genbuf, len, 0) == len);
}

int chat_recv(int fd, char * chatid) {
	static char buf[512];
	static int bufstart = 0;
	int c, len;
	char *bptr, tmptopic[256];
	len = sizeof(buf) - bufstart - 1;
	if ((c = recv(fd, buf + bufstart, len, 0)) <= 0) {
		return -1;
	}
	c += bufstart;
	bptr = buf;
	while (c > 0) {
		len = strlen(bptr) + 1;
		if (len > c && len < (sizeof(buf) / 2)) {
			break;
		}

		if (*bptr == '/') {
			switch (bptr[1]) {
				case 'c':
					chat_clear();
					break;
				case 'n':
					strncpy(chatid, bptr + 2, 8);
					print_chatid(chatid);
					clrtoeol();
					break;
				case 'r':
				case 't':
					if (bptr[1] == 'r') {
						strncpy(chatname, bptr + 2, IDLEN - 1);
						chatname[IDLEN-1] = '\0';
					} else {
						strncpy(buftopic, bptr+2, 43);
						buftopic[42] = '\0';
					}
					transPERstr(buftopic, tmptopic);
					sprintf(genbuf,
							"[1;44;33m ·¿¼ä£º [32m%-12.12s  [5;31;44m%6.6s"
								"[0;1;44;33m»°Ìâ£º[36m%-49.49s[m",
							chatname, (recflag == 1) ? "[Â¼Òô]" : "      ",
							tmptopic);
					move(0, 0);
					clrtoeol();
					prints("%s", genbuf);
			} //switch(bptr[1])
		} else {
			printchatline(bptr);
		}

		c -= len;
		bptr += len;
	}

	if (c > 0) {
		strcpy(genbuf, bptr);
		strncpy(buf, genbuf, sizeof(buf));
		bufstart = len - 1;
	} else {
		bufstart = 0;
	}
	return 0;
}

void fixchatid(char *chatid) {
	chatid[CHAT_IDLEN] = '\0';
	while (*chatid != '\0' && *chatid != '\n') {
		if (strchr(BADCIDCHARS, *chatid)) {
			*chatid = '_';
		}
		chatid++;
	}
}

int ent_chat(char *chatbuf) {
	char inbuf[80], chatid[CHAT_IDLEN], lastcmd[MAXLASTCMD][80], *ptr;
	struct sockaddr_in sin;
	struct hostent *h;
	int cfd, cmdpos, ch;
	int chatroom, chatport;
	int currchar;
	int newmail;
	extern int talkidletime, enabledbchar;
	int page_pending = NA;
	int chatting = YEA;
	int i, j;
	char runchatbuf[STRLEN];
	char stnname[12][20];
	char stnaddr[12][30];
	char temp[10];
	int portnum[12];
	FILE *stationrec;
	chatroom = atoi(chatbuf);
	/* added by roly 02.03.24*/
	if (chatroom==1) {
		chatroom=2;
	}
	/* add end */
	if (chatroom == 2) {
		strcpy(chat_station, CHATNAME2);
		modify_user_mode(CHAT2);
		chatport = CHATPORT2;
	} else {
		chatroom = 1;
		strcpy(chat_station, CHATNAME1);
		modify_user_mode(CHAT1);
		chatport = CHATPORT1;
	}

	if ((chatroom == 1) && (stationrec = fopen("etc/chatstation", "r"))
			!= NULL) {
		i = 0;
		while (fgets(inbuf, STRLEN, stationrec) != NULL) {
			if (inbuf[0]=='#') {
				continue;
			}
			ptr = strtok(inbuf, " \n\r\t");
			if (ptr) {
				strncpy(stnname[i], ptr, 19);
				ptr = strtok(NULL, " \n\r\t");
				if (ptr) {
					strncpy(stnaddr[i], ptr, 29);
					ptr = strtok(NULL, " \n\r\t");
					if (ptr) {
						strncpy(temp, ptr, 9);
						portnum[i] = atoi(temp);
					} else {
						portnum[i] = 7201;
					}
				} else {
					strcpy(stnaddr[i], "127.0.0.1");
				}
				i ++;
			} // if
		}// while
		fclose(stationrec);
		move(1, 0);
		clrtobot();
		prints("\n\nÐò Á¬ÏßÕ¾Ãû³Æ             Á¬ÏßÕ¾Î»Ö·\n");
		prints("== =====================  ==============================\n");
		for (j = 0; j <= i - 1; j++) {
			move(5 + j, 0);
			prints("%2d %-22s %-32s", j+1, stnname[j], stnaddr[j]);
		}
		getdata(23, 0, "ÇëÊäÈëÕ¾Ì¨ÐòºÅ(ÍË³ö£º0)", temp, 3, DOECHO, YEA);
		i = 0;
		i = atoi(temp);
		if (i == 0)
			return 0;
		i --;
		if ((i > j - 1) || (i < 0))
			i = 0;
		clear();
		prints("Á¬Íù¡º%s¡»£¬ÇëÉÔºò...\n", stnname[i]);
		if (!(h = gethostbyname(stnaddr[i])))
			return -1;
		memset(&sin, 0, sizeof sin);
		sin.sin_family = PF_INET;
		memcpy(&sin.sin_addr, h->h_addr, h->h_length);
		sin.sin_port = htons(portnum[i]);
		cfd = socket(sin.sin_family, SOCK_STREAM, 0);

		if (connect(cfd, (struct sockaddr *) &sin, sizeof(sin))) {
			gethostname(inbuf, STRLEN);
			if (!(h = gethostbyname(inbuf)))
				return -1;
			memset(&sin, 0, sizeof sin);
			sin.sin_family = PF_INET;
			memcpy(&sin.sin_addr, h->h_addr, h->h_length);
			chatport = CHATPORT1;
			sin.sin_port = htons(chatport);
			close(cfd);
			move(1, 0);
			clrtoeol();
			prints("¶Ô·½ÁÄÌìÊÒÃ»¿ª£¬Á¬½ø±¾Õ¾µÄ¹ú¼Ê»áÒéÌü...");
			sprintf(runchatbuf, "bin/chatd %d", chatroom);
			system(runchatbuf);
			cfd = socket(sin.sin_family, SOCK_STREAM, 0);
			if ((connect(cfd, (struct sockaddr *) &sin, sizeof(sin))))
				return -1;
		}
	} else {
		gethostname(inbuf, STRLEN);
		if (!(h = gethostbyname(inbuf)))
			return -1;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = PF_INET;
		memcpy(&sin.sin_addr, h->h_addr, h->h_length);
		sin.sin_port = htons(chatport);
		cfd = socket(sin.sin_family, SOCK_STREAM, 0);

		if (connect(cfd, (struct sockaddr *) &sin, sizeof(sin))) {
			close(cfd);
			move(1, 0);
			clrtoeol();
			prints("¿ªÆôÁÄÌìÊÒ...");
			sprintf(runchatbuf, "bin/chatd %d", chatroom);
			system(runchatbuf);
			sleep(1);
			cfd = socket(sin.sin_family, SOCK_STREAM, 0);
			if ((connect(cfd, (struct sockaddr *) &sin, sizeof(sin))))
				return -1;
		}
	}
	while (1) {
		getdata(2, 0, "ÇëÊäÈëÁÄÌì´úºÅ(ÍË³ö: *)£º", inbuf, CHAT_IDLEN, DOECHO, YEA);
		if (inbuf[0] == '*')
			return;
		sprintf(chatid, "%.8s",
				((inbuf[0] != '\0' && inbuf[0] != '\n') ? inbuf
						: cuser.userid));
		fixchatid(chatid);
		sprintf(inbuf, "/! %d %d %s %s %d", uinfo.uid, cuser.userlevel,
				cuser.userid, chatid,
				HAS_PERM(PERM_CLOAK) ? uinfo.invisible : 0);
		chat_send(cfd, inbuf);
		if (recv(cfd, inbuf, 3, 0) != 3) {
			return 0;
		}
		move(3, 0);
		if (!strcmp(inbuf, CHAT_LOGIN_OK)) {
			break;
		} else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS)) {
			prints("Õâ¸ö´úºÅÒÑ¾­ÓÐÈËÓÃÁË");
		} else if (!strcmp(inbuf, CHAT_LOGIN_INVALID)) {
			prints("Õâ¸ö´úºÅÊÇ´íÎóµÄ");
		} else {
			prints("ÄúÒÑ¾­ÓÐÁíÒ»¸öÊÓ´°½øÈë´ËÁÄÌìÊÒ¡£");
		}
		clrtoeol();
		refresh();
		sleep(1);
		bell();
	}

	endmsg();
	add_io(cfd, 0);

	newmail = cmdpos = currchar = 0;
	memset(lastcmd, 0, MAXLASTCMD * 80);

	uinfo.in_chat = YEA;
	strcpy(uinfo.chatid, chatid);
	update_utmp();

	clear();
	chatline = 2;
	strcpy(inbuf, chatid);
	stop_line = t_lines - 2;
	move(stop_line, 0);
	outs(msg_seperator);
	move(1, 0);
	outs(msg_seperator);
	print_chatid(chatid);
	memset(inbuf, 0, 80);

	chat_load_alias(); /* KCN alias emote , 1999.11.25 */

	while (chatting) {
		move(b_lines, currchar + 10);
		ch = igetkey();
		talkidletime = 0;
		if (talkrequest)
			page_pending = YEA;
		if (page_pending)
			page_pending = servicepage(0, NULL);
		switch (ch) {
			case KEY_UP: //ÕâÀïÓÐÎÊÌâ?
				cmdpos += MAXLASTCMD - 2;
			case KEY_DOWN:
				cmdpos++;
				cmdpos %= MAXLASTCMD;
				strcpy(inbuf, lastcmd[cmdpos]);
				move(b_lines, 10);
				clrtoeol();
				outs(inbuf);
				currchar = strlen(inbuf);
				continue;
			case KEY_LEFT:
				if (currchar) {
					--currchar;
				}
				continue;
			case KEY_RIGHT:
				if (inbuf[currchar]) {
					++currchar;
				}
				continue;
		} //switch
		if (!newmail && chkmail(0)) {
			newmail = 1;
			printchatline("[1;32m¡ô [31mµ±£¡ÓÊ²îËÍÐÅÀ´ÁË...[m");
		}
		if (ch == I_OTHERDATA) { /* incoming */
			if (chat_recv(cfd, chatid) == -1)
				break;
			continue;
		}
#ifdef BIT8
		if (isprint2(ch)) {
#else
		if (isprint(ch)) {
#endif
			if (currchar < 68) {
				if (inbuf[currchar]) { /* insert */
					int i;
					for (i = currchar; inbuf[i] && i < 68; i++)
						;
					inbuf[i + 1] = '\0';
					for (; i > currchar; i--)
						inbuf[i] = inbuf[i - 1];
				} else { /* append */
					inbuf[currchar + 1] = '\0';
				}
				inbuf[currchar] = ch;
				move(b_lines, currchar + 10);
				outs(&inbuf[currchar++]);
			}
			continue;
		}
		if (ch == '\n' || ch == '\r') {
			if (currchar) {
				chatting = chat_cmd(inbuf, cfd);
				if (chatting == 0)
					chatting = chat_send(cfd, inbuf);
				if (!strncmp(inbuf, "/b", 2))
					break;
				for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
					strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);

				strcpy(lastcmd[0], inbuf);
				inbuf[0] = '\0';
				currchar = cmdpos = 0;
				move(b_lines, 10);
				clrtoeol();
			}
			continue;
		}
		if (ch == Ctrl('R') ) {
			enabledbchar = !enabledbchar;
			continue;
		}
		if (ch == Ctrl('H') || ch == '\177') {
			if (currchar) {
				register int i, patch=0;
				if (enabledbchar) {
					for (i=currchar-1; i>=0&&inbuf[i]&0x80; i--)
						patch ++;
					if (patch && patch%2==0) {
						patch =1;
					} else if (patch%2==1 && inbuf[currchar]&0x80) {
						currchar ++;
						patch = 1;
					} else {
						patch = 0;
					}
				} // µ½´Ë  1009

				currchar--;
				if (currchar && patch)
					currchar --;
				else
					patch = 0;

				inbuf[69] = '\0';
				memcpy(&inbuf[currchar], &inbuf[currchar + 1 + patch], 69
						- currchar);
				move(b_lines, currchar + 10);
				clrtoeol();
				outs(&inbuf[currchar]);
			}
			continue;
		}
		if (ch == Ctrl('Q')) {
			inbuf[0] = '\0';
			currchar = 0;
			move(b_lines, 10);
			clrtoeol();
			continue;
		}
		if (ch == Ctrl('C') || ch == Ctrl('D')) {
			chat_send(cfd, "/b");
			if (recflag == 1) {
				set_rec();
			}
			break;
		}
	}
	close(cfd);
	add_io(0, 0);
	uinfo.in_chat = NA;
	uinfo.chatid[0] = '\0';
	update_utmp();
	clear();
	refresh();
	free(chat_aliases); /* KCN alias emote , 1999.11.25 */
	return 0;
}

int printuserent(struct user_info *uentp) {
	static char uline[256];
	static int cnt;
	char pline[50];
	if (!uentp) {
		if (cnt)
			printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if ((uentp->invisible && !(HAS_PERM(PERM_SEECLOAK) || usernum
			==uentp->uid)) || isreject(uentp))
		return 0;

#if 0

	if (kill(uentp->pid, 0) == -1)
	return 0;
#endif

	sprintf(pline, " %s%-13s[m%c%-10.10s",
			myfriend(uentp->userid) ? "[1;32m" : "", uentp->userid,
			uentp->invisible ? '#' : ' ', ModeType(uentp->mode) );
	//modestring(uentp->mode, uentp->destuid, 0, NULL));
	if (cnt < 2)
		strcat(pline, "©¦");
	strcat(uline, pline);
	if (++cnt == 3) {
		printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
	}
	return 0;
}

void chat_help(char * arg) {
	char * ptr;
	char buf[256];
	FILE * fp;
	if (strstr(arg, " op")) {
		if ((fp = fopen("help/chatophelp", "r")) == NULL)
			return;
		while (fgets(buf, 255, fp) != NULL) {
			ptr = strstr(buf, "\n");
			*ptr = '\0';
			printchatline(buf);
		}
		fclose(fp);
	} else {
		if ((fp = fopen("help/chathelp", "r")) == NULL)
			return;
		while (fgets(buf, 255, fp) != NULL) {
			ptr = strstr(buf, "\n");
			*ptr = '\0';
			printchatline(buf);
		}
		fclose(fp);
	}
}
/* youzi 1997.7.25 */
void query_user(char *arg) {
	char *userid, msg[STRLEN * 2];
	char qry_mail_dir[STRLEN], buf[50];
	int tuid, exp, perf, clr = 0, online = 0;
	time_t now;
	struct user_info uin;
	extern int t_cmpuids();

	userid = strrchr(arg, ' ');
	if (userid == NULL) {
		printchatline("[1;37m¡ï [33mÇëÊäÈëÄúÒª²éÑ°µÄ ID [37m¡ï[m");
		return;
	}
	userid++;
	tuid = getuser(userid);
	if (!tuid) {
		printchatline("[1;31m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m");
		return;
	}
	online = t_search_ulist(&uin, t_cmpuids, tuid, NA, NA);
	sprintf(qry_mail_dir, "mail/%c/%s/%s", toupper(lookupuser.userid[0]),
			lookupuser.userid, DOT_DIR);
	exp = countexp(&lookupuser);
	perf = countperf(&lookupuser);
	sprintf(msg, "[1;37m%s[m ([1;33m%s[m) ¹²ÉÏÕ¾ [1;32m%d[m ´Î, ·¢±í"
		"¹ý [1;32m%d[m ÆªÎÄÕÂ", lookupuser.userid, lookupuser.username,
			lookupuser.numlogins, lookupuser.numposts);
	printchatline(msg);
	if (HAS_DEFINE(lookupuser.userdefine, DEF_COLOREDSEX) )
		clr = (lookupuser.gender == 'F') ? 5 : 6;
	else
		clr = 2; /* °²ÄÜ±æÎÒÊÇÐÛ´Æ ?! :D */
	if (strcasecmp(lookupuser.userid, "guest") != 0) {
		sprintf(buf, "[[1;3%dm%s[m] ", clr, horoscope(
				lookupuser.birthmonth, lookupuser.birthday) );
	} else {
		sprintf(buf, "");
	}
	getdatestring(lookupuser.lastlogin, NA);
	sprintf(msg, "%sÉÏ´ÎÔÚ [[1;32m%s[m] ÓÉ [[1;32m%s[m] µ½±¾Õ¾Ò»ÓÎ",
			(HAS_DEFINE(lookupuser.userdefine, DEF_S_HOROSCOPE) ) ? buf
					: "", datestring,
			(lookupuser.lasthost[0]=='\0' ? "(²»Ïê)" : lookupuser.lasthost));
	printchatline(msg);
	/*
	 sprintf(msg, 
	 "ÐÅÏä£º[[1;5;32m%2s[m]£¬¾­ÑéÖµ£º[[1;32m%d[m]([1;36m%s[m)"
	 " ±íÏÖÖµ£º[[1;32m%d[m]([1;36m%s[m) ÉúÃüÁ¦£º[[1;32m%d[m]",
	 (check_query_mail(qry_mail_dir) == 1) ? "ÐÅ" : "  ", exp, cexp(exp),
	 perf, cperf(perf), compute_user_value(&lookupuser));
	 *///modified by roly 02.03.24
	/*
	 sprintf(msg, 
	 "ÐÅÏä£º[[1;5;32m%2s[m]£¬¾­ÑéÖµ£º([1;36m%s[m)"
	 " ±íÏÖÖµ£º[[1;32m%d[m]([1;36m%s[m) ÉúÃüÁ¦£º[[1;32m%d[m]",
	 (check_query_mail(qry_mail_dir) == 1) ? "ÐÅ" : "  ", cexp(exp),
	 perf, cperf(perf), compute_user_value(&lookupuser));
	 */
	sprintf(msg, 
	"ÐÅÏä:[[1;5;32m%2s[m] ¾­ÑéÖµ:"
#ifdef SHOWEXP
			"%d([1;36m%-10s[m)"
#else
			"[[1;36m%-10s[m]"
#endif
			" ±íÏÖÖµ:"
#ifdef SHOWPERF
			"%d([1;36m%s[m)"
#else
			"[[1;36m%s[m]"
#endif
			" ÉúÃüÁ¦:[[1;32m%d[m]",
			(check_query_mail(qry_mail_dir) == 1) ? "ÐÅ" : "  "
#ifdef SHOWEXP
			,exp
#endif
			, cexpstr(exp)
#ifdef SHOWPERF
			,perf
#endif
			, cperf(perf), compute_user_value(&lookupuser));

	printchatline(msg);
	if (online) {
		sprintf(msg, "[1;37mÄ¿Ç°ÔÚÕ¾ÉÏ[m");
	} else {
		if (lookupuser.lastlogout < lookupuser.lastlogin) {
			now = ((time(0)-lookupuser.lastlogin)/120)%47+1
					+lookupuser.lastlogin;
		} else {
			now = lookupuser.lastlogout;
		}
		getdatestring(now, NA);
		sprintf(msg, "[1;37mÀëÕ¾Ê±¼ä£º[[1;32m%s[1;37m][m", datestring);
	}
	printchatline(msg);
}

void call_user(char *arg) {
	char *userid, msg[STRLEN * 2];
	struct user_info *uin;
	int good_id;
	userid = strrchr(arg, ' ');
	if (userid == NULL) {
		printchatline("[1;37m¡ï [32mÇëÊäÈëÄúÒªÑûÇëµÄ ID[37m ¡ï[m");
		return;
	} else {
		userid += 1;
	}
	if (!strcasecmp(userid, currentuser.userid)) {
		good_id = NA;
	} else {
		uin = t_search(userid, NA);
		if (uin == NULL ||uin->invisible&&(!HAS_PERM(PERM_SEECLOAK))) {
			good_id = NA;
		} else {
			good_id = YEA;
		}
	}
	if (good_id == YEA && canmsg(uin)) {
		sprintf(msg, "µ½ %s µÄ %s °üÏáÁÄÁÄÌì", chat_station, chatname);
		do_sendmsg(uin, msg, 1, uin->pid);
		sprintf(msg, "[1;37mÒÑ¾­°ïÄúÑûÇë [32m%s[37m ÁË[m", uin->userid);
	} else {
		sprintf(msg, "[1;32m%s[37m %s[m", userid, uin
				&& !uin->invisible ? "ÎÞ·¨ºô½Ð" : "²¢Ã»ÓÐÉÏÕ¾");
	}
	printchatline(msg);
}

void chat_date() {
	time_t thetime;
	time(&thetime);
	sprintf(genbuf, "[1m %s±ê×¼Ê±¼ä: [32m%s[m", BoardName, Cdate(&thetime));
	printchatline(genbuf);
}

void chat_users() {
	printchatline("");
	sprintf(genbuf, "[1m¡¾ [36m%s [37mµÄ·Ã¿ÍÁÐ±í ¡¿[m", BoardName);
	printchatline(genbuf);
	printchatline(msg_shortulist);

	if (apply_ulist(printuserent) == -1) {
		printchatline("[1m¿ÕÎÞÒ»ÈË[m");
	}
	printuserent(NULL);
}

int print_friend_ent(struct user_info * uentp)//print one user & status if he is a friend
{
	static char uline[256];
	static int cnt;
	char pline[50];

	if (!uentp) {
		if (cnt) {
			printchatline(uline);
		}
		memset(uline, 0, 256);
		cnt = 0;
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (!(HAS_PERM(PERM_SEECLOAK)||usernum==uentp->uid)
			&& uentp->invisible) {
		return 0;
	}

#if 0
	if (kill(uentp->pid, 0) == -1)
	return 0;
#endif

	/*if (!myfriend(uentp->userid)) Leeward 99.02.01 */
	if (!myfriend(uentp->uid))
		return 0;

	sprintf(pline, " %-13s%c%-10s", uentp->userid, uentp->invisible ? '#'
			: ' ', ModeType(uentp->mode) );
	//modestring(uentp->mode, uentp->destuid, 0, NULL));
	if (cnt < 2)
		strcat(pline, "©¦");
	strcat(uline, pline);
	if (++cnt == 3) {
		printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
	}
	return 0;
}

void chat_friends() {
	printchatline("");
	sprintf(genbuf, "[1m¡¾ µ±Ç°ÏßÉÏµÄºÃÓÑÁÐ±í ¡¿[m");
	printchatline(genbuf);
	printchatline(msg_shortulist);

	if (apply_ulist(print_friend_ent) == -1) {
		printchatline("[1mÃ»ÓÐÅóÓÑÔÚÏßÉÏ[m");
	}
	print_friend_ent(NULL);
}

void set_rec() {
	char fname[STRLEN], tmptopic[256];
	time_t now;
	now = time(0);

	sprintf(fname, "tmp/chat.%s", currentuser.userid);
	if (recflag == 0) {
		if ((rec = fopen(fname, "w")) == NULL)
			return;

		printchatline("[1;5;32m¿ªÊ¼Â¼Òô...[m");
		recflag = 1;
		move(0, 0);
		clrtoeol();
		transPERstr(buftopic, tmptopic);
		sprintf(genbuf,
				"[1;44;33m ·¿¼ä£º [32m%-12.12s  [5;31;44m%6.6s[0;1;44;33m"
					"»°Ìâ£º[36m%-49.49s[m", chatname,
				(recflag == 1) ? "[Â¼Òô]" : "      ", tmptopic);
		prints("%s", genbuf);
		fprintf(rec, "±¾¶ÎÓÉ %s", currentuser.userid);
		getdatestring(now, NA);
		fprintf(rec, "ËùÂ¼ÏÂ£¬Ê±¼ä£º %s\n", datestring);
	} else {
		recflag = 0;
		move(0, 0);
		clrtoeol();
		transPERstr(buftopic, tmptopic);
		sprintf(genbuf,
				"[1;44;33m ·¿¼ä£º [32m%-12.12s  [5;31;44m%6.6s[0;1;44;33m"
					"»°Ìâ£º[36m%-49.49s[m", chatname,
				(recflag == 1) ? "[Â¼Òô]" : "      ", tmptopic);
		prints("%s", genbuf);
		printchatline("[1;5;32mÂ¼Òô½áÊø...[m");
		getdatestring(now, NA);
		fprintf(rec, "½áÊøÊ±¼ä£º%s\n\n", datestring);
		fclose(rec);
		mail_file(fname, currentuser.userid, "Â¼Òô½á¹û");
		/*  Postfile(fname,"syssecurity","Â¼Òô½á¹û",2); */
		unlink(fname);
	}
}

//ÉèÖÃºô½ÐÆ÷ÊôÐÔ
void setpager() {
	char buf[STRLEN];
	t_pager();
	sprintf(buf, "[1;32m¡ô [31mºô½ÐÆ÷ %s ÁË[m",
			(uinfo.pager & ALL_PAGER) ? "´ò¿ª" : "¹Ø±Õ");
	printchatline(buf);

}

/* After This *//* KCN alias emote , 1999.11.25 */
void chat_sendmsg(char *arg) {
	struct user_info * uentp;
	char * uident, * msgstr, showstr[80];

	uident=strchr(arg, ' ');
	if (uident != NULL) {
		for (; *uident!=0 && *uident == ' '; uident++)
			;
	}
	if (uident==NULL || *uident == 0) {
		printchatline("[1;32mÇëÊäÈëÄúÒª·¢ÏûÏ¢µÄ ID[m");
		return;
	}

	msgstr=strchr(uident, ' ');
	if (msgstr!=NULL) {
		*msgstr=0;
		msgstr++;
		for (; *msgstr!=0 && *msgstr == ' '; msgstr++)
			;
	}
	if (msgstr==NULL || *msgstr == 0) {
		printchatline("[1;32mÇëÊäÈëÄúÒª·¢µÄÏûÏ¢[m");
		return;
	}

	uentp=t_search(uident, NA);
	if (uentp==NULL || uentp->invisible&&(!HAS_PERM(PERM_SEECLOAK))) {
		printchatline("[1mÏßÉÏÃ»ÓÐÕâ¸öID[m");
		return;
	}
	/*	02.11.05 added by stephen to fix the char mode can send message to
	 the person who close msger*/
	if (!canmsg(uentp)) {
		printchatline("[1m¶Ô·½²»Ô¸½ÓÊÜÄãµÄÑ¶Ï¢[m");
		return;
	}
	/*	02.11.05 add end*/
	if (do_sendmsg(uentp, msgstr, 2, 0)!=1) {
		sprintf(showstr, "[1mÎÞ·¨·¢ÏûÏ¢¸ø %s [m", uentp->userid);
		printchatline(showstr);
		return;
	}
	sprintf(showstr, "[1mÒÑ¾­¸ø %s ·¢³öÏûÏ¢[m", uentp->userid);
	printchatline(showstr);
}

char * maildoent(int num, struct fileheader *ent);

void chat_showmail() {
	struct fileheader *ents;
	int ssize=sizeof(struct fileheader);
	extern char currmaildir[STRLEN ];
	char tmpstr[512];
	int i;
	int nums;
	int lines = t_lines-7;
	int base;

	ents =(struct fileheader *)malloc(ssize*lines);

	nums = get_num_records(currmaildir, ssize);
	base = nums-lines+1;
	if (base<=0)
		base=1;

	lines = get_records(currmaildir, ents, ssize, base, lines);
	printchatline("[1;32mµ±Ç°ÐÅ¼þÁÐ±í[0m");
	printchatline("[1;36m----------------------------------------------[0m");

	for (i=0; i<lines; i++) {
		strcpy(tmpstr, maildoent(i+base, &ents[i]));
		printchatline(tmpstr);
	}
	free(ents);
}

void define_alias(char *arg) {
	int i;
	int del=0;
	char buf[256];
	char* action;

	for (i=0; (i<255)&&(arg[i]!=0)&&!isspace(arg[i]); i++)
		;

	if (arg[i]==0) {
		if (chat_alias_count!=0) {
			printchatline("ÒÑ¶¨ÒåµÄalias:\n");
			for (i=0; i<chat_alias_count; i++) {
				if (chat_aliases[i].cmd[0]!=0) {
					sprintf(buf, "%-9s %s\n", chat_aliases[i].cmd,
							chat_aliases[i].action);
					printchatline(buf);
				}
			}
			return;
		} else {
			printchatline("Î´¶¨Òåalias\n");
		}
	}

	arg=&arg[i+1];
	for (i=0; (i<9)&&(arg[i]!=0)&&!isspace(arg[i]); i++)
		;
	if (i>=9) {
		printchatline("aliasÌ«³¤!\n");
		return;
	}

	if (arg[i]==0)
		del=1;
	else
		action=&arg[i+1];

	arg[i]=0;

	if (!del) {
		if (chat_alias_count==MAXDEFINEALIAS) {
			printchatline("×Ô¶¨ÒåaliasÒÑ¾­ÂúÁË\n");
			return;
		}
	}

	for (i=0; i<chat_alias_count; i++)
		if (!strncasecmp(chat_aliases[i].cmd, arg, 8)) {
			if (del) {
				chat_alias_count--;
				setuserfile(buf, "chatalias");
				if (chat_alias_count!=0) {
					memcpy( &chat_aliases[i],
							&chat_aliases[chat_alias_count],
							sizeof(struct chatalias));
					chat_aliases[chat_alias_count].cmd[0]=0;
					substitute_record(buf,
							&chat_aliases[chat_alias_count],
							sizeof(chat_aliases[chat_alias_count]),
							chat_alias_count+1);
				} else {
					chat_aliases[i].cmd[0]=0;
				}
				substitute_record(buf, &chat_aliases[i],
						sizeof(chat_aliases[i]), i+1);
				sprintf(buf, "×Ô¶¨ÒåaliasÒÑ¾­É¾³ý\n");
				printchatline(buf);
				return;
			} else {
				sprintf(buf, "×Ô¶¨Òåalias-%sÒÑ¾­´æÔÚ\n", chat_aliases[i].cmd);
				printchatline(buf);
				return;
			}
		}
	if (!del) {
		strncpy(chat_aliases[chat_alias_count].cmd, arg, 8);
		chat_aliases[chat_alias_count].cmd[8]=0;
		strncpy(chat_aliases[chat_alias_count].action, action, 80);
		chat_aliases[chat_alias_count].action[81]=0;
		sprintf(buf, "×Ô¶¨Òåalias-%sÒÑ¾­´´½¨\n", arg);
		printchatline(buf);
		i=chat_alias_count;
		chat_alias_count++;
	} else {
		printchatline("Ã»ÕÒµ½×Ô¶¨Òåalias\n");
		return;
	}
	setuserfile(buf, "chatalias");
	substitute_record(buf, &chat_aliases[i], sizeof(chat_aliases[i]), i+1);
}

int use_alias(char *arg, int cfd) {
	char buf[256];
	char buf1[256];
	char* args[10];
	int arg_count;
	int i;
	int slen;
	register char* fmt;

	strncpy(buf, arg, 255);
	arg_count=0;
	args[0]=buf;
	args[1]=buf;
	while (*args[arg_count+1]!=0)
		if (isspace(*args[arg_count+1])) {
			arg_count++;
			if (arg_count==9)
				break;
			*args[arg_count]=0;
			args[arg_count]++;
			args[arg_count+1]=args[arg_count];
		} else
			args[arg_count+1]++;
	for (i=arg_count+1; i<10; i++)
		args[i]="´ó¼Ò";

	for (i=0; i<chat_alias_count; i++) {
		if (!strncasecmp(chat_aliases[i].cmd, args[0], 8)) {
			strcpy(buf1, "/a ");
			slen=3;
			fmt=chat_aliases[i].action;
			while (*fmt!=0) {
				if (*fmt=='$') {
					fmt++;
					if (isdigit(*fmt)) {
						int index=*fmt-'0';
						if (slen+strlen(args[index])>255-8)
							break;
						buf1[slen]=0;
						strcat(buf1, "[1m");
						strcat(buf1, args[index]);
						strcat(buf1, "[m");
						slen+=strlen(args[index])+7;
					} else if (*fmt=='s') {
						if (slen+strlen(args[1])>255-8)
							break;
						buf1[slen]=0;
						strcat(buf1, "[1m");
						strcat(buf1, args[1]);
						strcat(buf1, "[m");
						slen+=strlen(args[1])+7;
					} else if (slen<253) {
						buf1[slen++]='$';
						buf1[slen++]=*fmt;
					} else
						break;
				} else
					buf1[slen++]=*fmt;
				fmt++;
			}
			buf1[slen]=0;
			chat_send(cfd, buf1);
			return 1;
		}
	}
	return 0;
}
/* Above All *//* KCN alias emote , 1999.11.25 */

/* Function:
 *    show the msg of the current user.
 * Writer: Seaman      Date: 2002/1/6
 */
void chat_showmsg(char *_no_used) {
	char str[256];
	char msgme_filename[256];
	FILE *fp_msgme;

	sprintf ( msgme_filename,
			BBSHOME"/home/%c/%s/msgfile.me",
			toupper(currentuser.userid[0]),
			currentuser.userid
	);
	/* printchatline(msgme_filename); */
	fp_msgme = fopen(msgme_filename, "rt");
	if (NULL != fp_msgme) {
		while ( !feof(fp_msgme) ) {
			fgets(str, 200, fp_msgme);
			if (feof(fp_msgme) )
				break;
			printchatline(str);
		}
	} else
		return;
	fclose(fp_msgme);
}
/* end Seaman */

struct chat_command chat_cmdtbl[] = { { "alias", define_alias },
/* KCN alias emote , 1999.11.25 */
{ "pager", setpager }, { "help", chat_help }, { "?", chat_help }, {
		"clear", chat_clear }, { "date", chat_date }, { "users",
		chat_users }, { "set", set_rec }, { "g", chat_friends }, { "call",
		call_user }, { "query", query_user }, /* 1997.7.25 youzi */
{ "mail", chat_showmail }, { "v", chat_showmsg }, /* added by Seaman */
{ NULL, NULL } };

int chat_cmd_match(char *buf, char *str) {
	while (*str && *buf && !isspace(*buf)) {
		if (tolower(*buf++) != *str++)
			return 0;
	}
	return 1;
}
/* After This *//* KCN alias emote , 1999.11.25 */
int chat_cmd(char *buf, int cfd) {
	int i;

	if (*buf++ != '/')
		return 0;

	if (*buf=='/')
		if (!use_alias(buf+1, cfd))
			return 0;
		else
			return 1;
	if ((tolower(*buf)=='m')&&isspace(*(buf+1)))
		return 0;
	if ((tolower(*buf)=='a')&&isspace(*(buf+1)))
		return 0;
	if ((tolower(*buf)=='s')&&isspace(*(buf+1))) {
		chat_sendmsg(buf);
		return 1;
	} else
		for (i = 0; chat_cmdtbl[i].cmdname; i++) {
			if (*buf!='\0'&&chat_cmd_match(buf, chat_cmdtbl[i].cmdname)) {
				chat_cmdtbl[i].cmdfunc(buf);
				return 1;
			}
		}
	return 0;
}
/* Above All *//* KCN alias emote , 1999.11.25 */
/*
 int
 chat_cmd(buf)
 char   *buf;
 {
 int     i;
 if (*buf++ != '/')
 return 0;
 
 for (i = 0; chat_cmdtbl[i].cmdname; i++) {
 if (*buf != '\0' && chat_cmd_match(buf, chat_cmdtbl[i].cmdname)) {
 chat_cmdtbl[i].cmdfunc(buf);
 return 1;
 }
 }
 return 0;
 } */
