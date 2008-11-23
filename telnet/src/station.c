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
 $Id: station.c 79 2005-10-16 13:52:50Z SpiritRain $
 */

#include "bbs.h"
#include "chat.h"
#include <sys/ioctl.h>
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef LINUX
#include <unistd.h>
#endif

#ifdef AIX
#include <sys/select.h>
#endif

#if !RELIABLE_SELECT_FOR_WRITE
#include <fcntl.h>
#endif

#if USES_SYS_SELECT_H
#include <sys/select.h>
#endif

#if NO_SETPGID
#define setpgid setpgrp
#endif

#ifndef L_XTND
#define L_XTND          2	/* relative to end of file */
#endif

#define RESTRICTED(u)   (users[(u)].flags == 0)	/* guest */
#define CLOAK(u)        (users[(u)].flags & PERM_CLOAK)
#define ROOMOP(u)       (users[(u)].flags & PERM_OCHAT)
#define SYSOP(u)       (users[(u)].flags & PERM_SYSOPS)
#define OUTSIDER(u)     !(users[(u)].flags & PERM_TALK)

#define ROOM_LOCKED     0x1
#define ROOM_SECRET     0x2

#define LOCKED(r)       (rooms[(r)].flags & ROOM_LOCKED)
#define SECRET(r)       (rooms[(r)].flags & ROOM_SECRET)

#define ROOM_ALL        (-2)

char *CHATROOM_TOPIC[2] = { "ÕâÀïÊÇ¹ú¼Ê»áÎîÖĞĞÄ", "½ñÌìÎÒÃÇÒªÌÖÂÛµÄÊÇ..." };

struct chatuser {
	int sockfd; /* socket to bbs server */
	int utent; /* utable entry for this user */
	int room; /* room: -1 means none, 0 means main */
	int flags;
	char cloak;
	char userid[IDLEN + 2]; /* real userid */
	char chatid[CHAT_IDLEN]; /* chat id */
	char ibuf[128]; /* buffer for sending/receiving */
	int ibufsize; /* current size of ibuf */
	char host[30];
} users[MAXACTIVE];

struct chatroom {
	char name[IDLEN]; /* name of room; room 0 is "main" */
	short occupants; /* number of users in room */
	short flags; /* ROOM_LOCKED, ROOM_SECRET */
	char invites[MAXACTIVE]; /* Keep track of invites to rooms */
	char topic[48]; /* Let the room op to define room topic */
} rooms[MAXROOM];

struct chatcmd {
	char *cmdstr;
	void (*cmdfunc)();
	int exact;
};

int chatroom, chatport;
int sock = -1; /* the socket for listening */
int nfds; /* number of sockets to select on */
int num_conns; /* current number of connections */
fd_set allfds; /* fd set for selecting */
struct timeval zerotv; /* timeval for selecting */
char chatbuf[256]; /* general purpose buffer */
char chatname[19];
/* name of the main room (always exists) */

char mainroom[] = "main";
char *maintopic = "½ñÌì£¬ÎÒÃÇÒªÌÖÂÛµÄÊÇ.....";

char *msg_not_op = "[1;37m¡ï[32mÄú²»ÊÇÕâÏá·¿µÄÀÏ´ó[37m ¡ï[m";
char *msg_no_such_id = "[1;37m¡ï[32m [[36m%s[32m] ²»ÔÚÕâ¼äÏá·¿Àï[37m ¡ï[m";
char *msg_not_here = "[1;37m¡ï [32m[[36m%s[32m] ²¢Ã»ÓĞÇ°À´±¾»áÒéÌü[37m ¡ï[m";

#define HAVE_REPORT

#ifdef  HAVE_REPORT
void report(char *s) {
	static int disable = NA;

	if (disable)
		return;

	file_append("trace.chatd", s);

	disable = YEA;
	return;
}
#else
#define	report(s)	;
#endif

is_valid_chatid(id)
char *id;
{
	int i;
	if (*id == '\0')
	return 0;

	for (i = 0; i < CHAT_IDLEN && *id; i++, id++) {
		if (strchr(BADCIDCHARS, *id))
		return 0;
	}
	return 1;
}
#ifdef SYSV
int FLOCK(int fd,int op)
{
	switch (op) {
		case LOCK_EX:
		return lockf(fd, F_LOCK, 0);
		case LOCK_UN:
		return lockf(fd, F_ULOCK, 0);
		default:
		return -1;
	}
}
#endif

int
Isspace(ch)
char ch;
{
	return (int) strchr(" \t\n\r", ch);
}

char *
nextword(str)
char **str;
{
	char *head, *tail;
	int ch;
	head = *str;
	for (;;) {
		ch = *head;
		if (!ch) {
			*str = head;
			return head;
		}
		if (!Isspace(ch))
		break;
		head++;
	}

	tail = head + 1;
	while (ch = *tail) {
		if (Isspace(ch)) {
			*tail++ = '\0';
			break;
		}
		tail++;
	}
	*str = tail;

	return head;
}

int
chatid_to_indx(unum, chatid)
int unum;
char *chatid;
{
	register int i;
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
		continue;
		if (!strcasecmp(chatid, users[i].chatid)) {
			if (users[i].cloak == 0 || !CLOAK(unum))
			return i;
		}
	}
	return -1;
}

int
fuzzy_chatid_to_indx(unum, chatid)
int unum;
char *chatid;
{
	register int i, indx = -1;
	unsigned int len = strlen(chatid);
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
		continue;
		if (!strncasecmp(chatid, users[i].chatid, len) ||
				!strncasecmp(chatid, users[i].userid, len)) {
			if (len == strlen(users[i].chatid) || len == strlen(users[i].userid)) {
				indx = i;
				break;
			}
			if (indx == -1)
			indx = i;
			else
			return -2;
		}
	}
	if (users[indx].cloak == 0 || CLOAK(unum))
	return indx;
	else
	return -1;
}

int
roomid_to_indx(roomid)
char *roomid;
{
	register int i;
	for (i = 0; i < MAXROOM; i++) {
		if (i && rooms[i].occupants == 0)
		continue;
		report(roomid);
		report(rooms[i].name);
		if (!strcasecmp(roomid, rooms[i].name))
		return i;
	}
	return -1;
}

void
do_send(writefds, str)
fd_set *writefds;
char *str;
{
	register int i;
	int len = strlen(str);
	if (select(nfds, NULL, writefds, NULL, &zerotv)> 0) {
		for (i = 0; i < nfds; i++)
		if (FD_ISSET(i, writefds))
		send(i, str, len + 1, 0);
	}
}

void
send_to_room(room, str)
int room;
char *str;
{
	int i;
	fd_set writefds;
	FD_ZERO(&writefds);
	for (i = 0; i < MAXROOM; i++) {
		if (users[i].sockfd == -1)
		continue;
		if (room == ROOM_ALL || room == users[i].room)
		/* write(users[i].sockfd, str, strlen(str) + 1); */
		FD_SET(users[i].sockfd, &writefds);
	}
	do_send(&writefds, str);
}

void
send_to_unum(unum, str)
int unum;
char *str;
{
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(users[unum].sockfd, &writefds);
	do_send(&writefds, str);
}

void
exit_room(unum, disp, msg)
int unum;
int disp;
char *msg;
{
	int oldrnum = users[unum].room;
	if (oldrnum != -1) {
		if (--rooms[oldrnum].occupants) {
			switch (disp) {
				case EXIT_LOGOUT:
				sprintf(chatbuf, "[1;37m¡ï [32m[[36m%s[32m] ÂıÂıÀë¿ªÁË [37m¡ï[37m", users[unum].chatid);
				if (msg && *msg) {
					strcat(chatbuf, ": ");
					strncat(chatbuf, msg, 80);
				}
				break;

				case EXIT_LOSTCONN:
				sprintf(chatbuf, "[1;37m¡ï [32m[[36m%s[32m] Ïñ¶ÏÁËÏßµÄ·çóİ ... [37m¡ï[37m", users[unum].chatid);
				break;

				case EXIT_KICK:
				sprintf(chatbuf, "[1;37m¡ï [32m[[36m%s[32m] ±»ÀÏ´ó¸Ï³öÈ¥ÁË [37m¡ï[37m", users[unum].chatid);
				break;
			}
			if (users[unum].cloak == 0)
			send_to_room(oldrnum, chatbuf);
		}
	}
	users[unum].flags &= ~PERM_OCHAT;
	users[unum].room = -1;
}

void
chat_rename(unum, msg)
int unum;
char *msg;
{
	int rnum;
	rnum = users[unum].room;

	if (!ROOMOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if (*msg == '\0') {
		send_to_unum(unum, "[1;31m¡ò [37mÇëÖ¸¶¨ĞÂµÄÁÄÌìÊÒÃû³Æ [31m¡ò[m");
		return;
	}
	strncpy(rooms[rnum].name, msg, IDLE);
	sprintf(chatbuf, "/r%.11s", msg);
	send_to_room(rnum, chatbuf);
	sprintf(chatbuf, "[1;37m¡ï [32m[[36m%s[32m] ½«ÁÄÌìÊÒÃû³Æ¸ÄÎª [1;33m%.11s [37m¡ï[37m", users[unum].chatid, msg);
	send_to_room(rnum, chatbuf);
}

void
chat_topic(unum, msg)
int unum;
char *msg;
{
	int rnum;
	rnum = users[unum].room;

	if (!ROOMOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if (*msg == '\0') {
		send_to_unum(unum, "[1;31m¡ò [37mÇëÖ¸¶¨»°Ìâ [31m¡ò[m");
		return;
	}
	strncpy(rooms[rnum].topic, msg, 43);
	rooms[rnum].topic[42] = '\0';
	sprintf(chatbuf, "/t%.42s", msg);
	send_to_room(rnum, chatbuf);
	sprintf(chatbuf, "[1;37m¡ï [32m[[36m%s[32m] ½«»°Ìâ¸ÄÎª [1;33m%42.42s [37m¡ï[37m", users[unum].chatid, msg);
	send_to_room(rnum, chatbuf);
}

enter_room(unum, room, msg)
int unum;
char *room;
char *msg;
{
	int rnum;
	int op = 0;
	register int i;
	rnum = roomid_to_indx(room);
	if (rnum == -1) {
		/* new room */
		if (OUTSIDER(unum)) {
			send_to_unum(unum, "[1;31m¡ò [37m±§Ç¸£¬Äú²»ÄÜ¿ªĞÂ°üÏá [31m¡ò[m");
			return 0;
		}
		for (i = 1; i < MAXROOM; i++) {
			if (rooms[i].occupants == 0) {
				report("new room");
				rnum = i;
				memset(rooms[rnum].invites, 0, MAXACTIVE);
				strcpy(rooms[rnum].topic, maintopic);
				strncpy(rooms[rnum].name, room, IDLEN - 1);
				rooms[rnum].name[IDLEN - 1] = '\0';
				rooms[rnum].flags = 0;
				op++;
				break;
			}
		}
		if (rnum == -1) {
			send_to_unum(unum, "[1;31m¡ò [37mÎÒÃÇµÄ·¿¼äÂúÁËà¸ [31m¡ò[m");
			return 0;
		}
	}
	if (!SYSOP(unum))
	if (LOCKED(rnum) && rooms[rnum].invites[unum] == 0) {
		send_to_unum(unum, "[1;31m¡ò [37m±¾·¿ÉÌÌÖ»úÃÜÖĞ£¬·ÇÇëÎğÈë [31m¡ò[m");
		return 0;
	}
	exit_room(unum, EXIT_LOGOUT, msg);
	users[unum].room = rnum;
	if (op)
	users[unum].flags |= PERM_OCHAT;
	rooms[rnum].occupants++;
	rooms[rnum].invites[unum] = 0;
	if (users[unum].cloak == 0) {
		sprintf(chatbuf, "[1;31m¡õ [37m[[36;1m%s[37m] ½øÈë [35m%s [37m°üÏá [31m¡õ[37m",
				users[unum].chatid, rooms[rnum].name);
		send_to_room(rnum, chatbuf);
	}
	sprintf(chatbuf, "/r%s", room);
	send_to_unum(unum, chatbuf);
	sprintf(chatbuf, "/t%s", rooms[rnum].topic);
	send_to_unum(unum, chatbuf);
	return 0;
}

void
logout_user(unum)
int unum;
{
	int i, sockfd = users[unum].sockfd;
	close(sockfd);
	FD_CLR(sockfd, &allfds);
	memset(&users[unum], 0, sizeof(users[unum]));
	users[unum].sockfd = users[unum].utent = users[unum].room = -1;
	for (i = 0; i < MAXROOM; i++) {
		if (rooms[i].invites != NULL)
		rooms[i].invites[unum] = 0;
	}
	num_conns--;
}
print_user_counts(unum)
int unum;
{
	int i, c, userc = 0, suserc = 0, roomc = 0;
	for (i = 0; i < MAXROOM; i++) {
		c = rooms[i].occupants;
		if (i> 0 && c> 0) {
			if (!SECRET(i) && !SYSOP(i))
			roomc++;
		}
		c = users[i].room;
		if (users[i].sockfd != -1 && c != -1 && users[i].cloak == 0) {
			if (SECRET(c) && !SYSOP(unum))
			suserc++;
			else
			userc++;
		}
	}
	sprintf(chatbuf, "[1;31m¡õ[37m »¶Ó­¹âÁÙ¡º[32m%s[37m¡»µÄ¡¾[36m%s[37m¡¿[31m¡õ[37m", MY_BBS_NAME, "ÑàÔ°Ò¹»°");
	send_to_unum(unum, chatbuf);
	sprintf(chatbuf,
			"[1;31m¡õ[37m Ä¿Ç°ÒÑ¾­ÓĞ [1;33m%d [37m¼ä»áÒéÊÒÓĞ¿ÍÈË [31m¡õ[37m",
			roomc + 1);
	send_to_unum(unum, chatbuf);
	sprintf(chatbuf, "[1;31m¡õ [37m±¾»áÒéÌüÄÚ¹²ÓĞ [36m%d[37m ÈË ", userc + 1);
	if (suserc)
	sprintf(chatbuf + strlen(chatbuf), "[[36m%d[37m ÈËÔÚ¸ß»úÃÜÌÖÂÛÊÒ]", suserc);
	sprintf(chatbuf + strlen(chatbuf), "[31m¡õ[37m");
	send_to_unum(unum, chatbuf);
	return 0;
}
login_user(unum, msg)
int unum;
char *msg;
{
	int i, utent; /* , fd = users[unum].sockfd; */
	char *utentstr;
	char *level;
	char *userid;
	char *chatid;
	char *cloak;
	utentstr = nextword(&msg);
	level = nextword(&msg);
	userid = nextword(&msg);
	chatid = nextword(&msg);
	cloak = nextword(&msg);

	utent = atoi(utentstr);
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd != -1 && users[i].utent == utent) {
			send_to_unum(unum, CHAT_LOGIN_BOGUS);
			return -1;
		}
	}
	if (!is_valid_chatid(chatid)) {
		send_to_unum(unum, CHAT_LOGIN_INVALID);
		return 0;
	}
	if (chatid_to_indx(0, chatid) != -1) {
		/* userid in use */
		send_to_unum(unum, CHAT_LOGIN_EXISTS);
		return 0;
	}
	if (!strncasecmp("localhost" /* MY_BBS_DOMAIN */, users[unum].host, 30)) {
		users[unum].flags = atoi(level) & PERM_TALK;
		users[unum].cloak = atoi(cloak);
	} else {
		if (chatport != CHATPORT1) { /* only CHAT1 allows remote
		 * user */
			send_to_unum(unum, CHAT_LOGIN_BOGUS);
			return -1;
		} else {
			if (!(atoi(level) & PERM_REGISTER) && !strncasecmp(chatid, "guest", 8)) {
				send_to_unum(unum, CHAT_LOGIN_INVALID);
				return 0;
			}
			users[unum].flags = 0;
			users[unum].cloak = 0;
		}
	}
	report(level);
	report(users[unum].host);

	users[unum].utent = utent;
	strcpy(users[unum].userid, userid);
	strncpy(users[unum].chatid, chatid, CHAT_IDLEN - 1);
	users[unum].chatid[CHAT_IDLEN] = '\0';
	send_to_unum(unum, CHAT_LOGIN_OK);
	print_user_counts(unum);
	enter_room(unum, mainroom, (char *) NULL);
	return 0;
}

void
chat_list_rooms(unum, msg)
int unum;
char *msg;
{
	int i, occupants;
	if (RESTRICTED(unum)) {
		send_to_unum(unum, "[1;31m¡ò [37m±§Ç¸£¡ÀÏ´ó²»ÈÃÄú¿´ÓĞÄÄĞ©·¿¼äÓĞ¿ÍÈË [31m¡ò[m");
		return;
	}
	send_to_unum(unum, "[1;33;44m Ì¸ÌìÊÒÃû³Æ  ©¦ÈËÊı©¦»°Ìâ        [m");
	for (i = 0; i < MAXROOM; i++) {
		occupants = rooms[i].occupants;
		if (occupants> 0) {
			if (!SYSOP(unum))
			if ((rooms[i].flags & ROOM_SECRET) && (users[unum].room != i))
			continue;
			sprintf(chatbuf, " [1;32m%-12s[37m©¦[36m%4d[37m©¦[33m%s[37m", rooms[i].name, occupants, rooms[i].topic);
			if (rooms[i].flags & ROOM_LOCKED)
			strcat(chatbuf, " [Ëø×¡]");
			if (rooms[i].flags & ROOM_SECRET)
			strcat(chatbuf, " [»úÃÜ]");
			send_to_unum(unum, chatbuf);
		}
	}
}

chat_do_user_list(unum, msg, whichroom)
int unum;
char *msg;
int whichroom;
{
	int start, stop, curr = 0;
	int i, rnum, myroom = users[unum].room;
	while (*msg && Isspace(*msg))
	msg++;
	start = atoi(msg);
	while (*msg && isdigit(*msg))
	msg++;
	while (*msg && !isdigit(*msg))
	msg++;
	stop = atoi(msg);
	send_to_unum(unum, "[1;33;44m ÁÄÌì´úºÅ©¦Ê¹ÓÃÕß´úºÅ  ©¦ÁÄ    Ìì    ÊÒ¡õOp¡õÀ´×Ô                          [m");
	for (i = 0; i < MAXROOM; i++) {
		rnum = users[i].room;
		if (users[i].sockfd != -1 && rnum != -1 && !(users[i].cloak == 1 && !CLOAK(unum))) {
			if (whichroom != -1 && whichroom != rnum)
			continue;
			if (myroom != rnum) {
				if (RESTRICTED(unum))
				continue;
				if ((rooms[rnum].flags & ROOM_SECRET) && !SYSOP(unum))
				continue;
			}
			curr++;
			if (curr < start)
			continue;
			else if (stop && (curr> stop))
			break;
			sprintf(chatbuf, "[1;5m%c[0;1;37m%-8s©¦[31m%s%-12s[37m©¦[32m%-14s[37m¡õ[34m%-2s[37m¡õ[35m%-30s[37m",
					(users[i].cloak == 1) ? 'C' : ' ', users[i].chatid, OUTSIDER(i) ? "[1;35m" : "[1;36m",
					users[i].userid, rooms[rnum].name, ROOMOP(i) ? "ÊÇ" : "·ñ", users[i].host);
			send_to_unum(unum, chatbuf);
		}
	}
	return 0;
}

void
chat_list_by_room(unum, msg)
int unum;
char *msg;
{
	int whichroom;
	char *roomstr;
	roomstr = nextword(&msg);
	if (*roomstr == '\0')
	whichroom = users[unum].room;
	else {
		if ((whichroom = roomid_to_indx(roomstr)) == -1) {
			sprintf(chatbuf, "[1;31m¡ò [37mÃ» %s Õâ¸ö·¿¼äà¸ [31m¡ò[37m", roomstr);
			send_to_unum(unum, chatbuf);
			return;
		}
		if ((rooms[whichroom].flags & ROOM_SECRET) && !SYSOP(unum)) {
			send_to_unum(unum, "[1;31m¡ò [37m±¾»áÒéÌüµÄ·¿¼ä½Ô¹«¿ªµÄ£¬Ã»ÓĞÃØÃÜ [31m¡ò[37m");
			return;
		}
	}
	chat_do_user_list(unum, msg, whichroom);
}

void
chat_list_users(unum, msg)
int unum;
char *msg;
{
	chat_do_user_list(unum, msg, -1);
}

void
chat_map_chatids(unum, whichroom)
int unum;
int whichroom;
{
	int i, c, myroom, rnum;
	myroom = users[unum].room;
	send_to_unum(unum,
			"[1;33;44m ÁÄÌì´úºÅ Ê¹ÓÃÕß´úºÅ  ©¦ ÁÄÌì´úºÅ Ê¹ÓÃÕß´úºÅ  ©¦ ÁÄÌì´úºÅ Ê¹ÓÃÕß´úºÅ [m");
	for (i = 0, c = 0; i < MAXROOM; i++) {
		rnum = users[i].room;
		if (users[i].sockfd != -1 && rnum != -1 && !(users[i].cloak == 1 && !CLOAK(unum))) {
			if (whichroom != -1 && whichroom != rnum)
			continue;
			if (myroom != rnum) {
				if (RESTRICTED(unum))
				continue;
				if ((rooms[rnum].flags & ROOM_SECRET) && !SYSOP(unum))
				continue;
			}
			sprintf(chatbuf + (c * 50), "[1;34;5m%c[m[1m%-8s%c%s%-12s%s[37m", (users[i].cloak == 1) ? 'C' : ' ',
					users[i].chatid, (ROOMOP(i)) ? '*' : ' ', OUTSIDER(i) ? "[1;35m" : "[1;36m", users[i].userid,
					(c < 2 ? "©¦" : "  "));
			if (++c == 3) {
				send_to_unum(unum, chatbuf);
				c = 0;
			}
		}
	}
	if (c> 0)
	send_to_unum(unum, chatbuf);
}

void
chat_map_chatids_thisroom(unum, msg)
int unum;
char *msg;
{
	chat_map_chatids(unum, users[unum].room);
}

void
chat_setroom(unum, msg)
int unum;
char *msg;
{
	char *modestr;
	int rnum = users[unum].room;
	int sign = 1;
	int flag;
	char *fstr;
	modestr = nextword(&msg);
	if (!ROOMOP(unum) && !SYSOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if (*modestr == '+')
	modestr++;
	else if (*modestr == '-') {
		modestr++;
		sign = 0;
	}
	if (*modestr == '\0') {
		send_to_unum(unum,
				"[1;31m¡Ñ [37mÇë¸æËß¹ñÌ¨ÄúÒªµÄ·¿¼äÊÇ: {[[32m+[37m(Éè¶¨)][[32m-[37m(È¡Ïû)]}{[[32ml[37m(Ëø×¡)][[32ms[37m(ÃØÃÜ)]}[m");
		return;
	}
	while (*modestr) {
		flag = 0;
		switch (*modestr) {
			case 'l':
			case 'L':
			flag = ROOM_LOCKED;
			fstr = "Ëø×¡";
			break;
			case 's':
			case 'S':
			flag = ROOM_SECRET;
			fstr = "»úÃÜ";
			break;
			default:
			sprintf(chatbuf, "[1;31m¡ò [37m±§Ç¸£¬¿´²»¶®ÄúµÄÒâË¼£º[[36m%c[37m] [31m¡ò[37m", *modestr);
			send_to_unum(unum, chatbuf);
			return;
		}
		if (flag && ((rooms[rnum].flags & flag) != sign * flag)) {
			rooms[rnum].flags ^= flag;
			sprintf(chatbuf, "[1;37m¡ï[32m Õâ·¿¼ä±» %s %s%sµÄĞÎÊ½ [37m¡ï[37m",
					users[unum].chatid, sign ? "Éè¶¨Îª" : "È¡Ïû", fstr);
			send_to_room(rnum, chatbuf);
		}
		modestr++;
	}
}

void
chat_nick(unum, msg)
int unum;
char *msg;
{
	char *chatid;
	int othernum;
	chatid = nextword(&msg);
	chatid[CHAT_IDLEN - 1] = '\0';
	if (!is_valid_chatid(chatid)) {
		send_to_unum(unum, "[1;31m¡ò [37mÄúµÄÃû×ÖÊÇ²»ÊÇĞ´´íÁË[31m ¡ò[m");
		return;
	}
	othernum = chatid_to_indx(0, chatid);
	if (othernum != -1 && othernum != unum) {
		send_to_unum (unum, "[1;31m¡ò [37m±§Ç¸£¡ÓĞÈË¸úÄúÍ¬Ãû£¬ËùÒÔÄú²»ÄÜ½øÀ´ [31m¡ò[m");
		return;
	}
	sprintf(chatbuf, "[1;31m¡ò [36m%s [0;37mÒÑ¾­¸ÄÃûÎª [1;33m%s [31m¡ò[37m",
			users[unum].chatid, chatid);
	send_to_room(users[unum].room, chatbuf);
	strcpy(users[unum].chatid, chatid);
	sprintf(chatbuf, "/n%s", users[unum].chatid);
	send_to_unum(unum, chatbuf);
}

void
chat_private(unum, msg)
int unum;
char *msg;
{
	char *recipient;
	int recunum;
	recipient = nextword(&msg);
	recunum = fuzzy_chatid_to_indx(unum, recipient);
	if (recunum < 0) {
		/* no such user, or ambiguous */
		if (recunum == -1)
		sprintf(chatbuf, msg_no_such_id, recipient);
		else
		sprintf(chatbuf, "[1;31m ¡ò[37m ÄÇÎ»²ÎÓëÕß½ĞÊ²÷áÃû×Ö [31m¡ò[37m");
		send_to_unum(unum, chatbuf);
		return;
	}
	if (*msg) {
		sprintf(chatbuf, "[1;32m ¡ù [36m%s [37m´«Ö½ÌõĞ¡ÃØÊéÀ´µ½[37m: ", users[unum].chatid);
		strncat(chatbuf, msg, 80);
		send_to_unum(recunum, chatbuf);
		sprintf(chatbuf, "[1;32m ¡ù [37mÖ½ÌõÒÑ¾­½»¸øÁË [36m%s[37m: ", users[recunum].chatid);
		strncat(chatbuf, msg, 80);
		send_to_unum(unum, chatbuf);
	} else {
		sprintf(chatbuf, "[1;31m ¡ò[37m ÄúÒª¸ú¶Ô·½ËµĞ©Ê²÷áÑ½£¿[31m¡ò[37m");
		send_to_unum(unum, chatbuf);
	}
}

put_chatid(unum, str)
int unum;
char *str;
{
	int i;
	char *chatid = users[unum].chatid;
	memset(str, ' ', 10);
	for (i = 0; chatid[i]; i++)
	str[i] = chatid[i];
	str[i] = ':';
	str[10] = '\0';
}

chat_allmsg(unum, msg)
int unum;
char *msg;
{
	if (*msg) {
		put_chatid(unum, chatbuf);
		strcat(chatbuf, msg);
		send_to_room(users[unum].room, chatbuf);
	}
	return 0;
}

void
chat_act(unum, msg)
int unum;
char *msg;
{
	if (*msg) {
		sprintf(chatbuf, "[1;36m%s [37m%s[37m", users[unum].chatid, msg);
		send_to_room(users[unum].room, chatbuf);
	}
}

void
chat_cloak(unum, msg)
int unum;
char *msg;
{
	if (CLOAK(unum)) {
		if (users[unum].cloak == 1)
		users[unum].cloak = 0;
		else
		users[unum].cloak = 1;
		sprintf(chatbuf, "[1;36m%s [37m%s ÒşÉí×´Ì¬...[37m", users[unum].chatid, (users[unum].cloak == 1) ? "½øÈë" : "Í£Ö¹");
		send_to_unum(unum, chatbuf);
	}
}

void
chat_join(unum, msg)
int unum;
char *msg;
{
	char *roomid;
	roomid = nextword(&msg);
	if (RESTRICTED(unum)) {
		send_to_unum(unum, "[1;31m¡ò [37mÄúÖ»ÄÜÔÚÕâÀïÁÄÌì [31m¡ò[m");
		return;
	}
	if (*roomid == '\0') {
		send_to_unum(unum, "[1;31m¡ò [37mÇëÎÊÄÄ¸ö·¿¼ä [31m¡ò[m");
		return;
	}
	if (!is_valid_chatid(roomid)) {
		send_to_unum(unum,"[1;31m¡ò[37m·¿¼äÃûÖĞ²»ÄÜÓĞ¡¾*:/%¡¿µÈ²»ºÏ·¨×Ö·û[31m¡ò[37m");
		return;
	}
	enter_room(unum, roomid, msg);
	return;
}

void
chat_kick(unum, msg)
int unum;
char *msg;
{
	char *twit;
	int rnum = users[unum].room;
	int recunum;
	twit = nextword(&msg);
	if (!ROOMOP(unum) && !SYSOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if ((recunum = chatid_to_indx(unum, twit)) == -1) {
		sprintf(chatbuf, msg_no_such_id, twit);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rnum != users[recunum].room) {
		sprintf(chatbuf, msg_not_here, users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	exit_room(recunum, EXIT_KICK, (char *) NULL);

	if (rnum == 0)
	logout_user(recunum);
	else
	enter_room(recunum, mainroom, (char *) NULL);
}

void
chat_makeop(unum, msg)
int unum;
char *msg;
{
	char *newop = nextword(&msg);
	int rnum = users[unum].room;
	int recunum;
	if (!ROOMOP(unum) && !SYSOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if ((recunum = chatid_to_indx(unum, newop)) == -1) {
		/* no such user */
		sprintf(chatbuf, msg_no_such_id, newop);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (unum == recunum) {
		sprintf(chatbuf, "[1;37m¡ï [32mÄúÍüÁËÄú±¾À´¾ÍÊÇÀÏ´óà¸ [37m¡ï[37m");
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rnum != users[recunum].room) {
		sprintf(chatbuf, msg_not_here, users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	users[unum].flags &= ~PERM_OCHAT;
	users[recunum].flags |= PERM_OCHAT;
	sprintf(chatbuf, "[1;37m¡ï [36m %s[32m¾ö¶¨ÈÃ [35m%s [32mµ±ÀÏ´ó [37m¡ï[37m", users[unum].chatid,
			users[recunum].chatid);
	send_to_room(rnum, chatbuf);
}

void
chat_invite(unum, msg)
int unum;
char *msg;
{
	char *invitee = nextword(&msg);
	int rnum = users[unum].room;
	int recunum;
	if (!ROOMOP(unum) && !SYSOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if ((recunum = chatid_to_indx(unum, invitee)) == -1) {
		sprintf(chatbuf, msg_not_here, invitee);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rooms[rnum].invites[recunum] == 1) {
		sprintf(chatbuf, "[1;37m¡ï [36m%s [32mµÈÒ»ÏÂ¾ÍÀ´ [37m¡ï[m", users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	rooms[rnum].invites[recunum] = 1;
	sprintf(chatbuf, "[1;37m¡ï [36m%s [32mÑûÇëÄúµ½ [[33m%s[32m] ·¿¼äÁÄÌì[37m ¡ï[m",
			users[unum].chatid, rooms[rnum].name);
	send_to_unum(recunum, chatbuf);
	sprintf(chatbuf, "[1;37m¡ï [36m%s [32mµÈÒ»ÏÂ¾ÍÀ´ [37m¡ï[m", users[recunum].chatid);
	send_to_unum(unum, chatbuf);
}

void
chat_broadcast(unum, msg)
int unum;
char *msg;
{
	if (!SYSOP(unum)) {
		send_to_unum(unum, "[1;31m¡ò [37mÄú²»¿ÉÒÔÔÚ»áÒéÌüÄÚ´óÉùĞú»© [31m¡ò[m");
		return;
	}
	if (*msg == '\0') {
		send_to_unum(unum, "[1;37m¡ï [32m¹ã²¥ÄÚÈİÊÇÊ²÷á [37m¡ï[m");
		return;
	}
	sprintf(chatbuf, "[1m Ding Dong!! ´«´ïÊÒ±¨¸æ£º [36m%s[37m ÓĞ»°¶Ô´ó¼ÒĞû²¼£º[m",

			users[unum].chatid);
	send_to_room(ROOM_ALL, chatbuf);
	sprintf(chatbuf, "[1;34m¡¾[33m%s[34m¡¿[m", msg);
	send_to_room(ROOM_ALL, chatbuf);
}

void
chat_goodbye(unum, msg)
int unum;
char *msg;
{
	exit_room(unum, EXIT_LOGOUT, msg);
}

/* -------------------------------------------- */
/* MUD-like social commands : action             */
/* -------------------------------------------- */

struct action {
	char *verb; /* ¶¯´Ê */
	char *part1_msg; /* ½é´Ê */
	char *part2_msg; /* ¶¯×÷ */
};

struct action party_data[] = {
		{ "admire", "¶Ô", "µÄ¾°ÑöÖ®ÇéÓÌÈçÌÏÌÏ½­Ë®Á¬Ãà²»¾ø" }, 
		{ "agree", "ÍêÈ«Í¬Òâ", "µÄ¿´·¨" },
		{ "bearhug", "ÈÈÇéµØÓµ±§", "" }, 
		{ "ben", "Ğ¦ºÇºÇµØ¶Ô", "Ëµ£º¡°±¿, Á¬Õâ¶¼²»ÖªµÀ... :P¡±" },
		{ "bless", "×£¸£", "ĞÄÏëÊÂ³É" }, 
		{ "bow", "±Ï¹§±Ï¾´µØÏò", "¾Ï¹ª" },
		{ "bye", "Ïò", "Ò»¹°ÊÖµÀ£º¡°ÇàÉ½ÒÀ¾É£¬ÅçË®³£À´£¬ÔÛÃÇÏÂ»ØÔÙ»á!¡±" },
		{ "bye1", "¿Ş¿ŞÌäÌäµØÀ­×Å",	"µÄÒÂ½Ç:¡°Éá²»µÃÄã×ß°¡...¡±" },
		{ "bye2", "ÆàÍñµØ¶Ô", "ËµµÀ:¡°ÊÀÉÏÃ»ÓĞ²»É¢µÄÑçÏ¯£¬ÎÒÏÈ×ßÒ»²½ÁË£¬´ó¼Ò¶à±£ÖØ.¡±" },
		{ "byebye", "Íû×Å",	"ÀëÈ¥µÄ±³Ó°½¥½¥ÏûÊ§£¬Á½µÎ¾§Ó¨µÄÀá»¨´ÓÈù±ß»¬Âä" },
		{ "bite", "ºİºİµØÒ§ÁË", "Ò»¿Ú£¬°ÑËûÍ´µÃÍÛÍÛ´ó½Ğ...ÕæË¬,¹ş¹ş£¡" },
		{ "blink", "¼Ù×°Ê²Ã´Ò²²»ÖªµÀ£¬¶Ô", "ÌìÕæµØÕ£ÁËÕ£ÑÛ¾¦" }, 
		{ "breath", ":¸Ï¿ì¸ø", "×öÈË¹¤ºôÎü!" },
		{ "brother", "¶Ô", "Ò»±§È­, µÀ:¡°ÄÑµÃÄãÎÒ¸Îµ¨ÏàÕÕ,²»ÈôÄãÎÒÒå½á½ğÀ¼,ÒâÏÂÈçºÎ?¡±" }, 
		{ "bigfoot", "Éì³ö´ó½Å£¬¶Ô×¼", "µÄÈı´ç½ğÁ«ºİºİµØ²ÈÏÂÈ¥" },
		{ "consider", "¿ªÊ¼ÈÏÕæ¿¼ÂÇÉ±ËÀ", "µÄ¿ÉÄÜĞÔºÍ¼¼ÊõÄÑ¶È" },
		{ "caress", "¸§Ãş", "" },
		{ "cringe", "Ïò", "±°¹ªÇüÏ¥£¬Ò¡Î²ÆòÁ¯" },
		{ "cry", "Ïò", "º¿ßû´ó¿Ş" }, 
		{ "cry1", "Ô½ÏëÔ½ÉËĞÄ£¬²»½ûÅ¿ÔÚ", "µÄ¼ç°òÉÏº¿ßû´ó¿ŞÆğÀ´" }, 
		{ "cry2", "ÆËÔÚ", "ÉíÉÏ£¬¿ŞµÃËÀÈ¥»îÀ´:¡°±ğ×ßÑ½!¸ÕÅİµÄ·½±ãÃæÄã»¹Ã»³ÔÍêÄØ!¡±" },
		{ "comfort", "ÎÂÑÔ°²Î¿", "" }, 
		{ "clap", "Ïò", "ÈÈÁÒ¹ÄÕÆ" }, 
		{ "dance", "À­ÁË", "µÄÊÖôæôæÆğÎè" },
		{ "die", "ºÜ¿áµØÌÍ³öÒ»°Ñ·ÀĞâË®Ç¹£¬¡°Åö!¡±Ò»Ç¹°Ñ", "¸ø±ĞÁË." },
		{ "dogleg",	"ÊÇÕı×Ú¹·ÍÈÍõ¡«ÍôÍô¡«, Ê¹¾¢ÅÄÅÄ", "µÄÂíÆ¨" }, 
		{ "dontno", "¶Ô", "Ò¡Ò¡Í·ËµµÀ: ¡°Õâ¸ö...ÎÒ²»ÖªµÀ...¡±" },
		{ "dontdo", "¶Ô", "Ëµ:¡°Ğ¡ÅóÓÑ£¬ ²»¿ÉÒÔÕâÑùÅ¶£¬ÕâÑù×öÊÇ²»ºÃµÄÓ´¡±" },
		{ "drivel", "¶ÔÖø", "Á÷¿ÚË®" },
		{ "dunno", "µÉ´óÑÛ¾¦£¬ÌìÕæµØÎÊ£º", "£¬ÄãËµÊ²Ã´ÎÒ²»¶®Ò®... :(" }, 
		{ "dlook", "´ô´ôµØÍû×Å", "£¬¿ÚË®»©À²À²µØÁ÷ÁËÒ»µØ" },
		{ "dove", "¸øÁË", "Ò»¿éDOVE£¬Ëµ:¡°ÄÅ£¬¸øÄãÒ»¿éDOVE£¬ÒªºÃºÃÌı»°Å¶¡±" },
		{ "emlook", "ÉÏÏÂ¶ËÏêÁË", "Á½ÑÛ¡°¾ÍÄãĞ¡×ÓÑ½,ÎÒµ±ÊÇË­ÄØ,Ò²¸ÒÔÚÕâÀïÈöÒ°!¡±" },
		{ "face", "ÍçÆ¤µÄ¶Ô", "×öÁË¸ö¹íÁ³." }, 
		{ "faceless", "¶Ô×Å", "´ó½ĞµÀ£º¡°ºÙºÙ, ÄãµÄÃæ×ÓÂô¶àÉÙÇ®Ò»½ï?¡±" },
		{ "farewell", "º¬Àá,ÒÀÒÀ²»ÉáµØÏò", "µÀ±ğ" },
		{ "fear", "¶Ô", "Â¶³öÅÂÅÂµÄ±íÇé" }, 
		{ "flook", "³Õ³ÕµØÍû×Å", ", ÄÇÉîÇéµÄÑÛÉñËµÃ÷ÁËÒ»ÇĞ." },
		{ "forgive", "´ó¶ÈµÄ¶Ô", "Ëµ£ºËãÁË£¬Ô­ÁÂÄãÁË" },
		{ "giggle", "¶ÔÖø", "ÉµÉµµÄ´ôĞ¦" },
		{ "grin", "¶Ô", "Â¶³öĞ°¶ñµÄĞ¦Èİ" },
		{ "growl", "¶Ô", "ÅØÏø²»ÒÑ" },
		{ "hammer", "¾ÙÆğ»İÏãµÄ50000000TÌú´¸Íù", "ÉÏÓÃÁ¦Ò»ÇÃ!, ïÏ!" },
		{ "hand", "¸ú", "ÎÕÊÖ" },
		{ "heng", "¿´¶¼²»¿´",	"Ò»ÑÛ£¬ ºßÁËÒ»Éù£¬¸ß¸ßµÄ°ÑÍ·ÑïÆğÀ´ÁË,²»Ğ¼Ò»¹ËµÄÑù×Ó..." },
		{ "hi", "¶Ô", "ºÜÓĞÀñÃ²µØËµÁËÒ»Éù£º¡°Hi! ÄãºÃ!¡±" },
		{ "hug", "ÇáÇáµØÓµ±§", "" }, 
		{ "kick", "°Ñ", "ÌßµÄËÀÈ¥»îÀ´" }, 
		{ "kiss", "ÇáÎÇ", "µÄÁ³¼Õ" }, 
		{ "laugh", "´óÉù³°Ğ¦", "" }, 
		{ "look", "ÔôÔôµØ¿´×Å", "£¬²»ÖªµÀÔÚ´òÊ²Ã´âÈÖ÷Òâ¡£" },
		{ "love", "ÉîÇéµØÍû×Å", "£¬·¢ÏÖ×Ô¼º°®ÉÏÁËta" },
		{ "lovelook", "Ò»Ë«Ë®ÍôÍôµÄ´óÑÛ¾¦º¬ÇéÂöÂöµØ¿´×Å", "!" },
		{ "missyou", "ÌğÌğÒ»Ğ¦£¬ÑÛÖĞÈ´Á÷ÏÂÑÛÀá:", "£¬ÕæµÄÊÇÄãÂğ£¬Õâ²»ÊÇ×öÃÎ£¿" }, 
		{ "meet", "¶Ô", "Ò»±§È­£¬ËµµÀ£º¡°¾ÃÎÅ´óÃû£¬½ñÈÕÒ»¼û£¬ÕæÊÇÈıÉúÓĞĞÒ£¡¡±" },
		{ "moon", "À­×Å", "µÄĞ¡ÊÖ£¬Ö¸×Å³õÉıµÄÔÂÁÁËµ£º¡°ÌìÉÏÃ÷ÔÂ£¬ÊÇÎÒÃÇµÄÖ¤ÈË¡±" }, 
		{ "nod", "Ïò", "µãÍ·³ÆÊÇ" },
		{ "nudge", "ÓÃÊÖÖâ¶¥", "µÄ·Ê¶Ç×Ó" }, 
		{ "oh", "¶Ô", "Ëµ£º¡°Å¶£¬½´×Ó°¡£¡¡±" },
		{ "pad", "ÇáÅÄ", "µÄ¼ç°ò" },
		{ "papa", "½ôÕÅµØ¶Ô", "Ëµ£º¡°ÎÒºÃÅÂÅÂÅ¶...¡±" },
		{ "papaya", "ÇÃÁËÇÃ", "µÄÄ¾¹ÏÄÔ´ü" },
		{ "praise", "¶Ô", "ËµµÀ: ¹ûÈ»¸ßÃ÷! Åå·şÅå·ş!" },
		{ "pinch", "ÓÃÁ¦µÄ°Ñ", "Å¡µÃºÚÇà" },
		{ "poor", "À­×Å", "µÄÊÖËµ£º¡°¿ÉÁ¯µÄº¢×Ó£¡¡±ÑÛÀáà§à§µØµôÁËÏÂÀ´....." },
		{ "punch", "ºİºİ×áÁË", "Ò»¶Ù" },
		{ "puke", "¶Ô×Å", "ÍÂ°¡ÍÂ°¡£¬¾İËµÍÂ¶à¼¸´Î¾ÍÏ°¹ßÁË" }, 
		{ "pure", "¶Ô", "Â¶³ö´¿ÕæµÄĞ¦Èİ" }, 
		{ "qmarry", "Ïò", "ÓÂ¸ÒµØ¹òÁËÏÂÀ´£ºÄãÔ¸Òâ¼Ş¸øÎÒÂğ---ÕæÊÇÓÂÆø¿É¼Î°¡!" },
		{ "report", "ÍµÍµµØ¶Ô", "Ëµ£º¡°±¨¸æÎÒºÃÂğ£¿¡±" },
		{ "rose", "Í»È»´ÓÉíºóÄÃ³öÒ»¶ä-`-,-<@ ÉîÇéµØÏ×¸ø", "£¡" },
		{ "rose999", "¶Ô", "³ªµÀ£º¡°ÎÒÒÑ¾­ÎªÄãÖÖÏÂ£¬¾Å°Ù¾ÅÊ®¾Å¶äÃµ¹å¡­¡­¡±" }, 
		{ "run", "Æø´­ÓõÓõµØ¶Ô", "Ëµ:ÎÒ»»ÁË°ËÆ¥¿ìÂíÈÕÒ¹¼æ³Ì¸ÏÀ´.ÄÜÔÙ¼ûµ½ÄãËÀÁËÒ²ĞÄ¸Ê" },
		{ "shrug", "ÎŞÄÎµØÏò", "ËÊÁËËÊ¼ç°ò" }, 
		{ "sigh", "¶Ô", "Ì¾ÁËÒ»¿ÚÆø,µÀ: Ôø¾­²×º£ÄÑÎªË®,³ıÈ´Î×É½²»ÊÇÔÆ..." },
		{ "slap", "Å¾Å¾µÄ°ÍÁË", "Ò»¶Ù¶ú¹â" },
		{ "smooch", "ÓµÎÇ×Å", "" }, 
		{ "snicker", "ºÙºÙºÙ..µÄ¶Ô", "ÇÔĞ¦" },
		{ "sniff", "¶Ô", "àÍÖ®ÒÔ±Ç" },
		{ "spank", "ÓÃ°ÍÕÆ´ò", "µÄÍÎ²¿" },
		{ "squeeze", "½ô½ôµØÓµ±§×Å", "" },
		{ "sorry", "¸Ğµ½¶Ô", "Ê®¶şÍò·ÖµÄÇ¸Òâ, ÓÚÊÇµÍÉùËµµÀ:ÎÒ¸Ğµ½·Ç³£µÄ±§Ç¸!" },
		{ "thank", "Ïò", "µÀĞ»" }, 
		{ "tickle", "¹¾ß´!¹¾ß´!É¦", "µÄÑ÷" }, 
		{ "wake", "Å¬Á¦µØÒ¡Ò¡", "£¬ÔÚÆä¶ú±ß´ó½Ğ£º¡°¿ìĞÑĞÑ£¬»á×ÅÁ¹µÄ£¡¡±" },
		{ "wakeup", "Ò¡×Å",  "£¬ÊÔ×Å°ÑËû½ĞĞÑ¡£´óÉùÔÚËû¶ú±ß´ó½Ğ£º¡¸ Öí! ÆğÀ´ÁË! ¡¹" },
		{ "wave", "¶Ô×Å", "Æ´ÃüµØÒ¡ÊÖ" },
		{ "welcome", "ÈÈÁÒ»¶Ó­", "µÄµ½À´" }, 
		{ "wink", "¶Ô", "ÉñÃØµØÕ£Õ£ÑÛ¾¦" },
		{ "zap", "¶Ô", "·è¿ñµØ¹¥»÷" },
		{ "xinku", "¸Ğ¶¯µÃÈÈÀáÓ¯¿ô£¬Ïò×Å", "Õñ±Û¸ßºô£º¡°Ê×³¤ĞÁ¿àÁË£¡¡±" },
		{ "bupa", "°®Á¯µØÃş×Å", "µÄÍ·,Ëµ:¡°Ğ¡ÃÃÃÃ,²»ÅÂ,ÊÜÁËÊ²Ã´Î¯Çü´ó¸ç¸çÌæÄã±¨³ğ!¡±" }, 
		{ "gril", "¸Ï½ô¸ø", "´·´·±³,ĞÄÏë:º¢×ÓĞ¡,±ğ²í¹ıÆøÈ¥." },
		{ ":)..", "¶Ô", "´¹ÏÑÈı³ß£¬²»ÖªµÀÏÂÒ»²½»áÓĞºÎ¾Ù¶¯" },
		{ "?", "ºÜÒÉ»óµÄ¿´×Å", "" }, 
		{ "@@", "Õö´óÁËÑÛ¾¦¾ªÆæµØ¶¢×Å", ":¡°Õâ...ÕâÒ²Ì«....¡±" }, 
		{ "@@!", "ºİºİµØµÉÁË", "Ò»ÑÛ, ËûÁ¢¿Ì±»¿´µÃËõĞ¡ÁËÒ»°ë" },
		{ NULL, NULL, NULL }
		};

int party_action(unum, cmd, party)
int unum;
char *cmd;
char *party;
{
	int i;
	for (i = 0; party_data[i].verb; i++) {
		if (!strcmp(cmd, party_data[i].verb)) {
			if (*party == '\0') {
				party = "´ó¼Ò";
			} else {
				int recunum = fuzzy_chatid_to_indx(unum, party);
				if (recunum < 0) {
					/* no such user, or ambiguous */
					if (recunum == -1)
					sprintf(chatbuf, msg_no_such_id, party);
					else
					sprintf(chatbuf, "[1;31m¡ò [37mÇëÎÊÄÄ¼ä·¿¼ä [31m¡ò[m");
					send_to_unum(unum, chatbuf);
					return 0;
				}
				party = users[recunum].chatid;
			}
			sprintf(chatbuf, "[1;36m%s [32m%s[33m %s [32m%s[37m",
					users[unum].chatid,
					party_data[i].part1_msg, party, party_data[i].part2_msg);
			send_to_room(users[unum].room, chatbuf);
			return 0;
		}
	}
	return 1;
}

/* -------------------------------------------- */
/* MUD-like social commands : speak              */
/* -------------------------------------------- */

struct action speak_data[] = { { "ask", "Ñ¯ÎÊ", NULL }, { "chant", "¸èËÌ",
		NULL }, { "cheer", "ºÈ²É", NULL }, { "chuckle", "ÇáĞ¦", NULL }, {
		"curse", "ÖäÂî", NULL }, { "demand", "ÒªÇó", NULL }, { "frown", "õ¾Ã¼",
		NULL }, { "groan", "ÉëÒ÷", NULL }, { "grumble", "·¢ÀÎÉ§", NULL }, {
		"hum", "à«à«×ÔÓï", NULL }, { "moan", "±¯Ì¾", NULL }, { "notice", "×¢Òâ",
		NULL }, { "order", "ÃüÁî", NULL }, { "ponder", "ÉòË¼", NULL }, {
		"pout", "àÙÖø×ìËµ", NULL }, { "pray", "Æíµ»", NULL }, { "request", "¿ÒÇó",
		NULL }, { "shout", "´ó½Ğ", NULL }, { "sing", "³ª¸è", NULL }, {
		"smile", "Î¢Ğ¦", NULL }, { "smirk", "¼ÙĞ¦", NULL }, { "swear", "·¢ÊÄ",
		NULL }, { "tease", "³°Ğ¦", NULL }, { "whimper", "ÎØÑÊµÄËµ", NULL }, {
		"yawn", "¹şÇ·Á¬Ìì", NULL }, { "yell", "´óº°", NULL },
		{ NULL, NULL, NULL } };

int
speak_action(unum, cmd, msg)
int unum;
char *cmd;
char *msg;
{
	int i;
	for (i = 0; speak_data[i].verb; i++) {
		if (!strcmp(cmd, speak_data[i].verb)) {
			sprintf(chatbuf, "[1;36m%s [32m%s£º[33m %s[37m",
					users[unum].chatid, speak_data[i].part1_msg, msg);
			send_to_room(users[unum].room, chatbuf);
			return 0;
		}
	}
	return 1;
}

/* -------------------------------------------- */
/* MUD-like social commands : condition          */
/* -------------------------------------------- */

struct action condition_data[] = { { "acid", "ËµµÀ£º¡°ºÃÈâÂïà¡~~~¡±", NULL }, {
		"addoil", "Íû¿Õ¸ßº°: ¼ÓÓÍ!!", NULL },
		{ "applaud", "Å¾Å¾Å¾Å¾Å¾Å¾Å¾....", NULL }, { "blush", "Á³¶¼ºìÁË", NULL }, {
				"boss", "·ÅÉù´ó½Ğ£ºÍÛÈû£¬²»µÃÁË£¬ÀÏ°åÀ´ÁË£¬ÎÒÒªÌÓÁË£¬ÔÙ¼ûÁË¸÷Î».", NULL }, { "bug",
				"´óÉùËµ¡°±¨¸æÕ¾³¤£¬ÎÒ×¥µ½Ò»Ö»³æ×Ó¡±¡£", NULL }, { "cool",
				"´ó½ĞÆğÀ´£ºÍÛÈû¡«¡«¡«ºÃcoolÅ¶¡«¡«", NULL }, { "cough", "¿ÈÁË¼¸Éù", NULL }, {
				"die", "¿ÚÍÂ°×Ä­£¬Ë«ÑÛÒ»·­£¬ÉíÌå´¤ÁË¼¸ÏÂ£¬²»¶¯ÁË£¬ÄãÒ»¿´ËÀÁË¡£", NULL }, { "goeat",
				"µÄ¶Ç×ÓÓÖ¹¾¹¾µÄ½ĞÁË,°¦,²»µÃ²»È¥³ÔÊ³ÌÃÄÇ#$%&µÄ·¹²ËÁË", NULL }, { "faint",
				"ßÛµ±Ò»Éù£¬ÔÎµ¹ÔÚµØ...", NULL },
		// {"faint","¿ÚÍÂ°×Ä­£¬»èµ¹ÔÚµØÉÏ¡£",NULL},
		{ "faint1", "¿ÚÖĞ¿ñÅçÏÊÑª£¬·­Éíµ¹ÔÚµØÉÏ¡£", NULL },
		{ "haha", "¹ş¹ş¹ş¹ş......", NULL }, { "handup",
				"Æ´ÃüµØÉì³¤×Ô¼ºµÄÊÖ±Û£¬¸ßÉù½ĞµÀ£º¡°ÎÒ£¬ÎÒ£¬ÎÒ£¡¡±", NULL }, { "happy",
				"r-o-O-m....ÌıÁËÕæË¬£¡", NULL }, { "heihei", "ÀäĞ¦Ò»Éù", NULL }, {
				"jump", "¸ßĞËµØÏñĞ¡º¢×ÓËÆµÄ£¬ÔÚÁÄÌìÊÒÀï±Ä±ÄÌøÌø¡£", NULL }, { "pavid",
				"Á³É«²Ô°×!ºÃÏñ¾åÅÂÊ²÷á!", NULL }, { "puke", "Õæ¶ñĞÄ£¬ÎÒÌıÁË¶¼ÏëÍÂ", NULL }, {
				"shake", "Ò¡ÁËÒ¡Í·", NULL }, { "sleep",
				"Zzzzzzzzzz£¬ÕæÎŞÁÄ£¬¶¼¿ìË¯ÖøÁË", NULL }, { "so", "¾Í½´×Ó!!", NULL }, {
				"strut", "´óÒ¡´ó°ÚµØ×ß", NULL }, { "suicide",
				"ÕæÏëÂò¿é¶¹¸¯À´Ò»Í·×²ËÀ, ÃşÁËÃşÉí±ßÓÖÃ»ÁãÇ®, Ö»ºÃÈÌÒ»ÊÖ", NULL }, { "toe",
				"¾õµÃÕæÊÇÎŞÁÄ, ÓÚÊÇ×¨ĞÄµØÍæÆğ×Ô¼ºµÄ½ÅÖºÍ·À´", NULL }, { "tongue", "ÍÂÁËÍÂÉàÍ·",
				NULL }, { "think", "ÍáÖøÍ·ÏëÁËÒ»ÏÂ", NULL }, { "wa", "ÍÛ£¡:-O",
				NULL }, { "wawl", "¾ªÌì¶¯µØµÄ¿Ş", NULL }, { "xixi", "ÍµĞ¦£ºÎûÎû¡«¡«",
				NULL },
		//  {"faint1","¿ÚÖĞ¿ñÅçÏÊÑª£¬·­Éíµ¹ÔÚµØÉÏ¡£",NULL},
		{ "ya", "µÃÒâµÄ×÷³öÊ¤ÀûµÄÊÖÊÆ! ¡¸ V ¡¹:¡° ¹ş¹ş¹ş...¡±", NULL }, { ":(",
				"³îÃ¼¿àÁ³µÄ,Ò»¸±µ¹Ã¹Ñù", NULL }, { ":)", "µÄÁ³ÉÏÂ¶³öÓä¿ìµÄ±íÇé.", NULL }, {
				":D", "ÀÖµÄºÏ²»Â£×ì", NULL }, { ":P", "ÍÂÁËÍÂÉàÍ·,²îµãÒ§µ½×Ô¼º", NULL }, {
				"dragon", "ÊÇÒ»¸öÉí²Ä¿ıÎäµÄĞ¡»ï×Ó. :P ", NULL }, { "@@",
				"ÒªËÀ²ËÁË %^#@%$#&^*&(&^$#%$#@(*&()*)_*&(#@%$^%&^.", NULL }, {
				NULL, NULL, NULL } };

int
condition_action(unum, cmd)
int unum;
char *cmd;
{
	int i;
	for (i = 0; condition_data[i].verb; i++) {
		if (!strcmp(cmd, condition_data[i].verb)) {
			sprintf(chatbuf, "[1;36m%s [33m%s[37m",
					users[unum].chatid, condition_data[i].part1_msg);
			send_to_room(users[unum].room, chatbuf);
			return 1;
		}
	}
	return 0;
}

/* -------------------------------------------- */
/* MUD-like social commands : help               */
/* -------------------------------------------- */

char *dscrb[] = {
		"[1m¡¾ Verb + Nick£º   ¶¯´Ê + ¶Ô·½Ãû×Ö ¡¿[36m   Àı£º//kick piggy[m",
		"[1m¡¾ Verb + Message£º¶¯´Ê + ÒªËµµÄ»° ¡¿[36m   Àı£º//sing ÌìÌìÌìÀ¶[m",
		"[1m¡¾ Verb£º¶¯´Ê ¡¿    ¡ü¡ı£º¾É»°ÖØÌá[m", NULL };
struct action *verbs[] = { party_data, speak_data, condition_data, NULL };

#define SCREEN_WIDTH    80
#define MAX_VERB_LEN    10
#define VERB_NO         8

void
view_action_verb(unum)
int unum;
{
	int i, j;
	char *p;
	send_to_unum(unum, "/c");
	for (i = 0; dscrb[i]; i++) {
		send_to_unum(unum, dscrb[i]);
		chatbuf[0] = '\0';
		j = 0;
		while (p = verbs[i][j++].verb) {
			strcat(chatbuf, p);
			if ((j % VERB_NO) == 0) {
				send_to_unum(unum, chatbuf);
				chatbuf[0] = '\0';
			} else {
				strncat(chatbuf, "        ", MAX_VERB_LEN - strlen(p));
			}
		}
		if (j % VERB_NO)
		send_to_unum(unum, chatbuf);
		send_to_unum(unum, " ");
	}
}

struct chatcmd chatcmdlist[] = { "act", chat_act, 0, "bye", chat_goodbye,
		0, "flags", chat_setroom, 0, "invite", chat_invite, 0, "join",
		chat_join, 0, "kick", chat_kick, 0, "msg", chat_private, 0,
		"nick", chat_nick, 0, "operator", chat_makeop, 0, "rooms",
				chat_list_rooms, 0, "whoin", chat_list_by_room, 1, "wall",
				chat_broadcast, 1, "cloak", chat_cloak, 1, "who",
				chat_map_chatids_thisroom, 0, "list", chat_list_users, 0,
				"topic", chat_topic, 0, "rname", chat_rename, 0, NULL,
				NULL, 0 };

int
command_execute(unum)
int unum;
{
	char *msg = users[unum].ibuf;
	char *cmd;
	struct chatcmd *cmdrec;
	int match = 0;
	/* Validation routine */
	if (users[unum].room == -1) {
		/* MUST give special /! command if not in the room yet */
		if (msg[0] != '/' || msg[1] != '!')
		return -1;
		else
		return (login_user(unum, msg + 2));
	}
	/* If not a /-command, it goes to the room. */
	if (msg[0] != '/') {
		chat_allmsg(unum, msg);
		return 0;
	}
	msg++;
	cmd = nextword(&msg);

	if (cmd[0] == '/') {

		if (!strcmp(cmd + 1, "help") || (cmd[1] == '\0')) {
			view_action_verb(unum);
			match = 1;
		} else if (party_action(unum, cmd + 1, msg) == 0)
		match = 1;
		else if (speak_action(unum, cmd + 1, msg) == 0)
		match = 1;
		else
		match = condition_action(unum, cmd + 1);
	} else {
		for (cmdrec = chatcmdlist; !match && cmdrec->cmdstr; cmdrec++) {
			if (cmdrec->exact)
			match = !strcasecmp(cmd, cmdrec->cmdstr);
			else
			match = !strncasecmp(cmd, cmdrec->cmdstr, strlen(cmd));
			if (match)
			cmdrec->cmdfunc(unum, msg);
		}
	}

	if (!match) {
		sprintf(chatbuf, "[1;31m ¡ò [37m±§Ç¸£¬¿´²»¶®ÄúµÄÒâË¼£º[36m/%s [31m¡ò[37m", cmd);
		send_to_unum(unum, chatbuf);
	}
	memset(users[unum].ibuf, 0, sizeof(users[unum].ibuf));
	return 0;
}

int
process_chat_command(unum)
int unum;
{
	register int i;
	int rc, ibufsize;
	if ((rc = recv(users[unum].sockfd, chatbuf, sizeof(chatbuf), 0)) <= 0) {
		/* disconnected */
		exit_room(unum, EXIT_LOSTCONN, (char *) NULL);
		return -1;
	}
	ibufsize = users[unum].ibufsize;
	for (i = 0; i < rc; i++) {
		/* if newline is two characters, throw out the first */
		if (chatbuf[i] == '\r')
		continue;

		/* carriage return signals end of line */
		else if (chatbuf[i] == '\n') {
			users[unum].ibuf[ibufsize] = '\0';
			if (command_execute(unum) == -1)
			return -1;
			ibufsize = 0;
		}
		/* add other chars to input buffer unless size limit exceeded */
		else {
			if (ibufsize < 127)
			users[unum].ibuf[ibufsize++] = chatbuf[i];
		}
	}
	users[unum].ibufsize = ibufsize;

	return 0;
}

int
main(argc, argv)
int argc;
char *argv[];
{
	struct sockaddr_in sin;
	register int i;
	int sr, newsock, sinsize;
	fd_set readfds;
	struct timeval tv;
	umask(007);
	/* ----------------------------- */
	/* init variable : rooms & users */
	/* ----------------------------- */

	if (argc <= 1) {
		//strcpy(chatname, CHATNAME1);
		chatroom = 1;
		//chatport = CHATPORT1;
	} else {
		chatroom = atoi(argv[1]);
	}
	if(chatroom == 2) {
		strcpy(chatname, CHATNAME2);
		chatport = CHATPORT2;
	} else {
		chatroom = 1;
		strcpy(chatname, CHATNAME1);
		chatport = CHATPORT1;
	}
	maintopic = CHATROOM_TOPIC[chatroom - 1];
	strcpy(rooms[0].name, mainroom);
	strcpy(rooms[0].topic, maintopic);

	if (chatport <= 1000) {
		strcpy(chatname, CHATNAME1);
		chatroom = 1;
	}
	for (i = 0; i < MAXACTIVE; i++) {
		users[i].chatid[0] = '\0';
		users[i].sockfd = users[i].utent = -1;
	}

	/* ------------------------------ */
	/* bind chat server to port       */
	/* ------------------------------ */

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(chatport);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *) & sin, sizeof(sin)) < 0) {
		return -1;
	}
	sinsize = sizeof(sin);
	if (getsockname(sock, (struct sockaddr *) & sin, &sinsize) == -1)
	exit(1);
	if (listen(sock, 5) == -1) exit(1);
	if (fork()) {
		return (0);
	}
	setpgid(0, 0);

	/* ------------------------------ */
	/* trap signals                   */
	/* ------------------------------ */

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	FD_ZERO(&allfds);
	FD_SET(sock, &allfds);
	nfds = sock + 1;

	while (1) {
		memcpy(&readfds, &allfds, sizeof(readfds));

		tv.tv_sec = 60 * 30;
		tv.tv_usec = 0;
		if ((sr = select(nfds, &readfds, NULL, NULL, &tv)) < 0) {
			if (errno == EINTR)
			sleep(50);
			continue;
		} else if (!sr)
		continue;

#if 0
		if (sr == 0) {
			exit(0);/* normal chat server shutdown */
		}
#endif

		if (FD_ISSET(sock, &readfds)) {
			sinsize = sizeof(sin);
			newsock = accept(sock, (struct sockaddr *) & sin, &sinsize);
			if (newsock == -1) {
				continue;
			}
			for (i = 0; i < MAXACTIVE; i++) {
				if (users[i].sockfd == -1) {
					struct hostent *hp;
					char *s = users[i].host;
					struct hostent *local;
					//struct in_addr in;
					int j;
					local = gethostbyname("localhost");

					if (local) {
						for (j = 0; j < local->h_length / sizeof(unsigned int); j++) {
							if (sin.sin_addr.s_addr == (unsigned int) local->h_addr_list[j])
							break;
						}
						if ((j < local->h_length) || (sin.sin_addr.s_addr == 0x100007F))
						strcpy(s, "localhost");
						else {
							hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr),
									sin.sin_family);
							strncpy(s, hp ? hp->h_name : (char *) inet_ntoa(sin.sin_addr), 30);
						}
					} else {
						hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr),
								sin.sin_family);
						strncpy(s, hp ? hp->h_name : (char *) inet_ntoa(sin.sin_addr), 30);
					}
					s[29] = 0;

					users[i].sockfd = newsock;
					users[i].room = -1;
					break;
				}
			}

			if (i >= MAXACTIVE) {
				/* full -- no more chat users */
				close(newsock);
			} else {

#if !RELIABLE_SELECT_FOR_WRITE
				int flags = fcntl(newsock, F_GETFL, 0);
				flags |= O_NDELAY;
				fcntl(newsock, F_SETFL, flags);
#endif

				FD_SET(newsock, &allfds);
				if (newsock >= nfds)
				nfds = newsock + 1;
				num_conns++;
			}
		}
		for (i = 0; i < MAXACTIVE; i++) {
			/* we are done with newsock, so re-use the variable */
			newsock = users[i].sockfd;
			if (newsock != -1 && FD_ISSET(newsock, &readfds)) {
				if (process_chat_command(i) == -1) {
					logout_user(i);
				}
			}
		}
#if 0
		if (num_conns <= 0) {
			/* one more pass at select, then we go bye-bye */
			tv = zerotv;
		}
#endif
	}
	/* NOTREACHED */
}
