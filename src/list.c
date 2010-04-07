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
 $Id: list.c 288 2006-08-07 12:02:32Z SpiritRain $
 */

#include "bbs.h"
#include "list.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15
#define BBS_PAGESIZE    (t_lines - 4)
#define refreshtime     (30)
extern time_t login_start_time;
extern char BoardName[];
time_t update_time = 0;
int freshmode = 0;
int SHOWONEBRD=0;

extern int cmpfnames();
extern int searchuser();
int showexplain = 0;
char fexp[30]; //added by kit to make W work

int toggle1 = 0, toggle2 = 0;
int mailmode, numf;
int friendmode = 0;
int usercounter, real_user_names = 0;
int range, page, readplan;
//struct user_info *user_record[MAXACTIVE];
struct user_info *user_record[USHM_SIZE];
struct userec *user_data;
// add by Flier - 2000.5.12 - Begin
enum sort_type {stUserID, stUserName, stIP, stState} st = stUserID;
// add by Flier - 2000.5.12 - End

char* get_explain(char* userid) {
	struct override tmp;
	char buf[STRLEN];
	memset(&tmp, 0, sizeof(tmp));
	fexp[0]=0;
	setuserfile(buf, "friends");
	if (search_record(buf, &tmp, sizeof(tmp), cmpfnames, userid))
		strcpy(fexp, tmp.exp);
	return fexp;
}

int
friend_search(uid, uentp, tblsize)
int uid;
struct user_info *uentp;
int tblsize;
{
	int hi, low, mid;
	int cmp;

	if (uid == 0) {
		return NA;
	}
	hi = tblsize - 1;
	low = 0;
	while (low <= hi) {
		mid = (low + hi) / 2;
		cmp = uentp->friend[mid] - uid;
		if (cmp == 0) {
			return YEA;
		}
		if (cmp> 0)
		hi = mid - 1;
		else
		low = mid + 1;
	}
	return NA;
}

int
myfriend(uid)
int uid;
{
	return friend_search(uid, &uinfo, uinfo.fnum);
}

int
hisfriend(uentp)
struct user_info *uentp;
{
	return friend_search(uinfo.uid, uentp, uentp->fnum);
}

//	”√ªßuentp.uid”Î”√ªßuinfo.uid «∑Ò±ª∂‘∑Ω…ËΩ¯¡À∫⁄√˚µ•
int isreject(struct user_info *uentp) {
	int i;

	if (uentp->uid != uinfo.uid) {
		for (i = 0; i<MAXREJECTS&&(uentp->reject[i]||uinfo.reject[i]); i++) {
			if (uentp->reject[i]==uinfo.uid||uentp->uid==uinfo.reject[i])
				return YEA; /* ±ª…ËŒ™∫⁄√˚µ• */
		}
	}
	return NA;
}
void print_title() {

	docmdtitle(
			(friendmode) ? "[∫√≈Û”—¡–±Ì]" : "[ π”√’ﬂ¡–±Ì]",
			" ¡ƒÃÏ[[1;32mt[m] ºƒ–≈[[1;32mm[m] ÀÕ—∂œ¢[[1;32ms[m] º”,ºı≈Û”—[[1;32mo[m,[1;32md[m] ø¥Àµ√˜µµ[[1;32m°˙[m,[1;32mRtn[m] «–ªªƒ£ Ω [[1;32mf[m] «Ûæ»[[1;32mh[m]");
	update_endline();
}
print_title2() {

	docmdtitle(
			(friendmode) ? "[∫√≈Û”—¡–±Ì]" : "[ π”√’ﬂ¡–±Ì]",
			"        ºƒ–≈[[1;32mm[m] º”,ºı≈Û”—[[1;32mo[m,[1;32md[m] ø¥Àµ√˜µµ[[1;32m°˙[m,[1;32mRtn[m] —°‘Ò[[1;32m°¸[m,[1;32m°˝[m] «Ûæ»[[1;32mh[m]");
	update_endline();
}

void update_data() {
	if (readplan == YEA)
		return;
	if (time(0) >= update_time + refreshtime - 1) {
		freshmode = 1;
	}
	signal(SIGALRM, update_data);
	alarm(refreshtime);
	return;
}

int print_user_info_title() {
	char title_str[512];
	char *field_2;

	move(2, 0);
	clrtoeol();
	field_2 = " π”√’ﬂÍ«≥∆";
	if (real_user_names)
		field_2 = "’Ê µ–’√˚  ";
	if (showexplain)
		field_2 = "∫√”—Àµ√˜  ";
	//modified by IAMFAT 2002-05-26
	//   sprintf(title_str,"[1;44m±‡∫≈ %s π”√’ﬂ¥˙∫≈%s %s%s%s%8.8s %s…œ’æµƒŒª÷√%s      P M %c%sƒø«∞∂ØÃ¨%s %5s[m\n",
	//   sprintf(title_str,"[1;44m±‡∫≈ %s π”√’ﬂ¥˙∫≈%s %s%s%s%8.8s %s…œ’æµƒŒª÷√%s      P M %c %sƒø«∞∂ØÃ¨%s %5s [m\n",
	//Modified by IAMFAT 2002-05-29
	sprintf(
			title_str,
			"[1;44m ±‡∫≈ %s π”√’ﬂ¥˙∫≈%s %s%s%s%8.8s %s…œ’æµƒŒª÷√%s     P M %c%sƒø«∞∂ØÃ¨%s  %5s[m\n",
			(st==stUserID) ? "[32m{" : " ", (st==stUserID) ? "}[37m"
					: " ", (st==stUserName) ? "[32m{" : " ", field_2, (st
					==stUserName) ? "}[37m" : " ", " ",
			(st==stIP) ? "[32m{" : " ", (st==stIP) ? "}[37m" : " ",
			((HAS_PERM(PERM_CLOAK)) ? 'C' : ' '), (st==stState) ? "[32m{"
					: " ", (st==stState) ? "}[37m" : " ",
#ifdef SHOW_IDLE_TIME 
			" ±:∑÷");
#else
			"");
#endif
	prints("%s", title_str);
	return 0;
}

show_message(msg)
char msg[];
{

	move(BBS_PAGESIZE + 2, 0);
	clrtoeol();
	if (msg != NULL)
	prints("[1m%s[m", msg);
	refresh();
}

void
swap_user_record(a, b)
int a, b;
{
	struct user_info *c;
	c = user_record[a];
	user_record[a] = user_record[b];
	user_record[b] = c;
}
// Add by Flier - 2000.5.12 - Begin
int compare_user_record(left, right)
struct user_info *left, *right;
{
	int retCode;

	switch(st) {
		case stUserID:
		retCode = strcasecmp(left->userid, right->userid);
		break;
		case stUserName:
		retCode = strcasecmp(left->username, right->username);
		break;
		case stIP:
		retCode = strncmp(left->from, right->from,20);
		break;
		case stState:
		retCode = left->mode - right->mode;
		break;
	}
	return retCode;
}
// Add by Filer - 2000.5.12 - End

void sort_user_record(int left, int right) {
	int i, last;

	if (left >= right)
		return;
	swap_user_record(left, (left + right) / 2);
	last = left;
	for (i = left + 1; i <= right; i++) {
		// Modified by Flier - 2000.5.12
		if (compare_user_record(user_record[i], user_record[left])<0) {
			swap_user_record(++last, i);
		}
	}
	swap_user_record(left, last);
	sort_user_record(left, last - 1);
	sort_user_record(last + 1, right);
}

int fill_userlist() {
	register int i, n, totalusernum;
	int friendno[MAXACTIVE];

	resolve_utmp();
	totalusernum = 0;
	numf = 0;
	for (i = 0; i < USHM_SIZE; i++) {
		if ( !utmpshm->uinfo[i].active ||!utmpshm->uinfo[i].pid
				||isreject(&utmpshm->uinfo[i]))
			continue;
		if (SHOWONEBRD && utmpshm->uinfo[i].currbrdnum!=uinfo.currbrdnum)
			continue;
		if ( (utmpshm->uinfo[i].invisible) &&(usernum
				!= utmpshm->uinfo[i].uid) &&(!HAS_PERM(PERM_SEECLOAK)))
			continue;
		if (myfriend(utmpshm->uinfo[i].uid)) {
			friendno[numf++] = totalusernum;
		} else if (friendmode)
			continue;
		user_record[totalusernum++] = &utmpshm->uinfo[i];
	}
	if (!friendmode) {
		for (i=0, n=0; i < totalusernum; i++) {
			if (n >= numf)
				break;
			if (friendno[n] == i) {
				if (i != n)
					swap_user_record(n, i);
				n ++;
			}
		}
		if (numf > 2) {
			sort_user_record(0, numf - 1);
		} else if (numf == 2) {
			/* The following line is modified by Amigo 2002.04.02. Fix bug of wrong sort. */
			/*         if(compare_user_record(user_record[0], user_record[1])<0)*/
			if (compare_user_record(user_record[0], user_record[1])>0)
				swap_user_record(0, 1);
		}
		sort_user_record(numf, totalusernum - 1);
	} else {
		if (totalusernum > 2) {
			sort_user_record(0, totalusernum - 1);
		} else if (totalusernum == 2) {
			/* The following line is modified by Amigo 2002.04.02. Fix bug of wrong sort. */
			/*         if(compare_user_record(user_record[0], user_record[1])<0)*/
			if (compare_user_record(user_record[0], user_record[1])>0)
				swap_user_record(0, 1);
		}
	}
	range = totalusernum;
	return totalusernum == 0 ? -1 : 1;
}

extern const char *idle_str(struct user_info *uent);

int do_userlist() {
	int i, j, override;
	char user_info_str[STRLEN * 2];
	struct user_info *uentp;

	move(3, 0);
	print_user_info_title();
	for (i = 0, j = 0; j < BBS_PAGESIZE && i + page < range; i++) {
		uentp = user_record[i + page];
		override = (i + page < numf) || friendmode;
		if (readplan == YEA)
			return 0;
		if (uentp == NULL || !uentp->active || !uentp->pid) { //by sunner
			continue; /* ƒ≥»À’˝«…¿Îø™ */
		}
		if (uentp != NULL) { // by wujian ‘ˆº” «∑ÒŒ™∂‘∑Ω∫√”—µƒœ‘ æ
			//Added IAMFAT 2002-05-27
			char userid[STRLEN];
			strcpy(userid,
					(override&&showexplain) ? get_explain(uentp->userid)
							: uentp->username);
			ellipsis(userid, 20);
			//End IAMFAT
			char *host;
			if (HAS_PERM2(PERM_OCHAT, &currentuser)) {
				host = uentp->from;
			} else {
				if (uentp->from[22] == 'H')
					host = "......";
				else
					host = mask_host(uentp->from);
			}

			char pager;
			if (uentp->mode == FIVE || uentp->mode == BBSNET
					|| uentp->mode == LOCKSCREEN)
				pager = '@';
			else
				pager = pagerchar(hisfriend(uentp), uentp->pager);

			char *color;
			if (uentp->invisible)
				color = "\033[1;30m";
			else if (is_web_user(uentp->mode))
				color = "\033[36m";
			else if (uentp->mode == POSTING || uentp->mode == MARKET)
				color = "\033[32m";
			else if (uentp->mode == FIVE || uentp->mode == BBSNET)
				color = "\033[33m";
			else
				color = "";

			snprintf(user_info_str, sizeof(user_info_str), " \033[m%4d%s"
					"%-12.12s\033[37m %-20.20s\033[m %-15.15s %c %c %c %s"
					"%-10.10s\033[37m %5.5s\033[m\n", i + 1 + page,
					(override) ? "\033[32m°Ã" : "  ", uentp->userid, userid,
					host, pager, msgchar(uentp),
					(uentp->invisible) ? '@' : ' ', color,
					mode_type(uentp->mode),
#ifdef SHOW_IDLE_TIME
					idle_str(uentp));
#else
					"");
#endif
			clrtoeol();
			prints("%s", user_info_str);
			j++;
		}
	}
	return 0;
}

int show_userlist() {
	if (update_time + refreshtime < time(0)) {
		fill_userlist();
		update_time = time(0);
	}
	if (range == 0) {
		move(2, 0);
		prints("√ª”– π”√’ﬂ£®≈Û”—£©‘⁄¡–±Ì÷–...\n");
		clrtobot();
		if (friendmode) {
			move(BBS_PAGESIZE + 3, 0);
			if (askyn(" «∑Ò◊™ªª≥… π”√’ﬂƒ£ Ω", YEA, NA) == YEA) {
				range = num_visible_users();
				page = -1;
				friendmode = NA;
				return 1;
			}
		} else
			pressanykey();
		return -1;
	}
	do_userlist();
	clrtobot();
	return 1;
}

int deal_key(char ch, int allnum, int pagenum) //ª∑πÀÀƒ∑Ω¥¶¿Ì∞¥º¸
{
	char buf[STRLEN], desc[5];
	static int msgflag;
	extern int friendflag;

	if (msgflag == YEA) {
		show_message(NULL);
		msgflag = NA;
	}
	switch (ch) {
		case 'Y':
			if (HAS_PERM(PERM_CLOAK))
				x_cloak();
			break;
		case 'P':
			t_pager();
			break;
		case 'C':
		case 'c':
			//added by iamfat 2002.10.29 avoid guest to change nick
			if ( !strcmp(currentuser.userid, "guest") )
				return 1;
			if (ch == 'C') {
				strcpy(genbuf, "±‰ªªÍ«≥∆(≤ª «¡Ÿ ±±‰ªª)Œ™: ");
			} else {
				strcpy(genbuf, "‘› ±±‰ªªÍ«≥∆(◊Ó∂‡10∏ˆ∫∫◊÷): ");
			}
			strcpy(buf, "");
			getdata(BBS_PAGESIZE+3,0,genbuf,buf,(ch=='C')?NAMELEN:21,DOECHO,NA);
			if (buf[0] != '\0') {
				strcpy(uinfo.username, buf);
				if (ch == 'C') {
					set_safe_record();
					strcpy(currentuser.username, buf);
					substitut_record(PASSFILE, &currentuser,
							sizeof(currentuser), usernum);
				}
			}
			break;
		case 'k':
		case 'K':
			if (!HAS_PERM(PERM_USER)
					&& (usernum!=user_record[allnum]->uid))
				return 1;
			if ( !strcmp(currentuser.userid, "guest") )
				return 1;
			if (user_record[allnum]->pid == uinfo.pid)
				strcpy(buf, "ƒ˙◊‘º∫“™∞—°æ◊‘º∫°øÃﬂ≥ˆ»•¬");
			else
				sprintf(buf, "ƒ„“™∞— %s Ãﬂ≥ˆ’æÕ‚¬", user_record[allnum]->userid);
			move(BBS_PAGESIZE + 2, 0);
			if (askyn(buf, NA, NA) == NA)
				break;
			char tmp[EXT_IDLEN];
			strlcpy(tmp, user_record[allnum]->userid, sizeof(tmp));
			if (do_kick_user(user_record[allnum]) == 0) {
				sprintf(buf, "%s “—±ªÃﬂ≥ˆ’æÕ‚", tmp);
			} else {
				sprintf(buf, "%s Œﬁ∑®Ãﬂ≥ˆ’æÕ‚", tmp);
			}
			msgflag = YEA;
			break;
		case 'h':
		case 'H':
			show_help("help/userlisthelp");
			break;
		case 't':
		case 'T':
			/* Following line modified by Amigo 2002.06.08. To combine perm_chat and perm_page right. */
			//         if (!HAS_PERM(PERM_PAGE)) return 1;
			if (!HAS_PERM(PERM_TALK))
				return 1;
			if (user_record[allnum]->uid!=usernum)
				ttt_talk(user_record[allnum]);
			else
				return 1;
			break;
		case 'v':
		case 'V':
			if ( !HAS_PERM(PERM_USER))
				return 1;
			real_user_names = !real_user_names;
			break;
		case 'm':
		case 'M':
			/* Following line modified by Amigo 2002.06.08. To add mail right. */
			/*         if (!HAS_PERM(PERM_POST)) return 1;*/
			if (!HAS_PERM(PERM_MAIL))
				return 1;
			m_send(user_record[allnum]->userid);
			break;
			/*
			 case 'g':
			 case 'G':
			 if(!HAS_PERM(PERM_POST)) return 1;
			 sendGoodWish(user_record[allnum]->userid);
			 break;
			 *///commented by roly 02.03.24
		case 'f':
		case 'F':
			if (friendmode)
				friendmode = NA;
			else
				friendmode = YEA;
			update_time = 0;
			break;
			/*      case 'x':
			 case 'X':
			 if(!Personal(user_record[allnum]->userid)){
			 sprintf(buf,"%s ªπ√ª”–…Í«Î∏ˆ»ÀŒƒºØ", user_record[allnum]->userid);
			 msgflag = YEA;
			 }
			 break;*///Commented by Amigo 2002.06.07
		case 's':
		case 'S':
			if (!strcmp("guest", currentuser.userid))
				return 0;
			if (!HAS_PERM(PERM_TALK))
				return 1;
			if (!canmsg(user_record[allnum])) {
				sprintf(buf, "%s “—æ≠πÿ±’—∂œ¢∫ÙΩ–∆˜", user_record[allnum]->userid);
				msgflag = YEA;
				break;
			}
			do_sendmsg(user_record[allnum], NULL, 0,
					user_record[allnum]->pid);
			break;
		case 'o':
		case 'O':
		case 'r':
		case 'R':
			if (!strcmp("guest", currentuser.userid))
				return 0;
			if (ch == 'o' || ch == 'O') {
				friendflag = YEA;
				strcpy(desc, "∫√”—");
			} else {
				friendflag = NA;
				strcpy(desc, "ªµ»À");
			}
			sprintf(buf, "»∑∂®“™∞— %s º”»Î%s√˚µ•¬", user_record[allnum]->userid,
					desc);
			move(BBS_PAGESIZE + 2, 0);
			if (askyn(buf, NA, NA) == NA)
				break;
			if (addtooverride(user_record[allnum]->userid) == -1) {
				sprintf(buf, "%s “—‘⁄%s√˚µ•", user_record[allnum]->userid,
						desc);
			} else {
				sprintf(buf, "%s ¡–»Î%s√˚µ•", user_record[allnum]->userid,
						desc);
			}
			msgflag = YEA;
			break;
		case 'd':
		case 'D':
			if (!strcmp("guest", currentuser.userid))
				return 0;
			sprintf(buf, "»∑∂®“™∞— %s ¥”∫√”—√˚µ•…æ≥˝¬", user_record[allnum]->userid);
			move(BBS_PAGESIZE + 2, 0);
			if (askyn(buf, NA, NA) == NA)
				break;
			if (deleteoverride(user_record[allnum]->userid, "friends")
					== -1) {
				sprintf(buf, "%s ±æ¿¥æÕ≤ª‘⁄≈Û”—√˚µ•÷–", user_record[allnum]->userid);
			} else {
				sprintf(buf, "%s “—¥”≈Û”—√˚µ•“∆≥˝", user_record[allnum]->userid);
			}
			msgflag = YEA;
			break;
		case 'W':
		case 'w':
			if (showexplain==1)
				showexplain=0;
			else
				showexplain=1;
			break;
		default:
			return 0;
	}
	if (friendmode)
		modify_user_mode(FRIEND);
	else
		modify_user_mode(LUSERS);
	if (readplan == NA) {
		print_title();
		clrtobot();
		if (show_userlist() == -1)
			return -1;
		if (msgflag)
			show_message(buf);
		update_endline();
	}
	return 1;
}

int deal_key2(char ch, int allnum, int pagenum) //ÃΩ ”Õ¯”—¥¶¿Ì∞¥º¸
{
	char buf[STRLEN];
	static int msgflag;

	if (msgflag == YEA) {
		show_message(NULL);
		msgflag = NA;
	}
	switch (ch) {
		case 'h':
		case 'H':
			show_help("help/usershelp");
			break;
		case 'm':
		case 'M':
			/* Following line modified by Amigo 2002.06.08. To add mail right. */
			/*         if (!HAS_PERM(PERM_POST)) return 1;*/
			if (!HAS_PERM(PERM_MAIL))
				return 1;
			m_send(user_data[allnum - pagenum].userid);
			break;
		case 'o':
		case 'O':
			if (!strcmp("guest", currentuser.userid))
				return 0;
			sprintf(buf, "»∑∂®“™∞— %s º”»Î∫√”—√˚µ•¬",
					user_data[allnum - pagenum].userid);
			move(BBS_PAGESIZE + 2, 0);
			if (askyn(buf, NA, NA) == NA)
				break;
			if (addtooverride(user_data[allnum - pagenum].userid) == -1) {
				sprintf(buf, "%s “—‘⁄≈Û”—√˚µ•", user_data[allnum-pagenum].userid);
				show_message(buf);
			} else {
				sprintf(buf, "%s ¡–»Î≈Û”—√˚µ•",
						user_data[allnum - pagenum].userid);
				show_message(buf);
			}
			msgflag = YEA;
			if (!friendmode)
				return 1;
			break;
		case 'f':
		case 'F':
			toggle1++;
			if (toggle1 >= 3)
				toggle1 = 0;
			break;
		case 'W':
		case 'w':
			if (showexplain==1)
				showexplain=0;
			else
				showexplain=1;
			break;
		case 't':
		case 'T':
			toggle2++;
#ifdef ALLOWGAME
			if (toggle2 >= 3) toggle2 = 0;
#else
			if (toggle2 >= 2)
				toggle2 = 0;
#endif
			break;
		case 'd':
		case 'D':
			if (!strcmp("guest", currentuser.userid))
				return 0;
			sprintf(buf, "»∑∂®“™∞— %s ¥”∫√”—√˚µ•…æ≥˝¬",
					user_data[allnum - pagenum].userid);
			move(BBS_PAGESIZE + 2, 0);
			if (askyn(buf, NA, NA) == NA)
				break;
			if (deleteoverride(user_data[allnum-pagenum].userid, "friends")
					==-1) {
				sprintf(buf, "%s ±æ¿¥æÕ≤ª‘⁄≈Û”—√˚µ•÷–",
						user_data[allnum - pagenum].userid);
				show_message(buf);
			} else {
				sprintf(buf, "%s “—¥”≈Û”—√˚µ•“∆≥˝",
						user_data[allnum - pagenum].userid);
				show_message(buf);
			}
			msgflag = YEA;
			if (!friendmode)
				return 1;
			break;
		default:
			return 0;
	}
	modify_user_mode(LAUSERS);
	if (readplan == NA) {
		print_title2();
		move(3, 0);
		clrtobot();
		if (Show_Users() == -1)
			return -1;
		update_endline();
	}
	redoscr();
	return 1;
}

int
countusers(uentp)
struct userec *uentp;
{
	static int totalusers;
	char permstr[11];
	if (uentp == NULL) {
		int c = totalusers;
		totalusers = 0;
		return c;
	}
	if (uentp->numlogins != 0 && uleveltochar(permstr, uentp->userlevel) != 0) {
		totalusers++;
		return 1;
	}
	return 0;
}

printuent(uentp)
struct userec *uentp;
{
	static int i;
	char permstr[11];
	char msgstr[18];
	int override;
	if (uentp == NULL) {
		printutitle();
		i = 0;
		return 0;
	}
	if (uentp->numlogins == 0 ||
			uleveltochar(permstr, uentp->userlevel) == 0)
	return 0;
	if (i < page || i >= page + BBS_PAGESIZE || i >= range) {
		i++;
		if (i >= page + BBS_PAGESIZE || i >= range)
		return QUIT;
		else
		return 0;
	}
	uleveltochar(&permstr, uentp->userlevel);
	switch (toggle1) {
		case 0:
			sprintf(msgstr, "%-.16s", getdatestring(uentp->lastlogin, DATE_ZH) + 6);
			break;
		case 1:
			sprintf(msgstr, "%-.16s", uentp->lasthost);
			break;
		case 2:
		default:
			sprintf(msgstr, "%-.14s", getdatestring(uentp->firstlogin, DATE_ZH));
			break;
	}
	user_data[i - page] = *uentp;
	override = myfriend(searchuser(uentp->userid));
	prints(" %5d%2s%s%-12s%s %-17s %6d %4d %10s %-16s\n", i + 1,
			(override) ? "°Ã" : "",
			(override) ? "[1;32m" : "", uentp->userid, (override) ? "[m" : "",
			uentp->username,
#ifdef ALLOWGAME
			(toggle2 == 0) ? (uentp->numlogins) : (toggle2 == 1) ? (uentp->numposts) : (uentp->money),
			(toggle2 == 0) ? uentp->stay / 3600 : (toggle2 == 1) ? (uentp->nummedals) : uentp->nummails,
#else
			(toggle2 == 0) ? (uentp->numlogins) : (uentp->numposts),
			(toggle2 == 0) ? uentp->stay / 3600 : uentp->nummails,
#endif
			HAS_PERM(PERM_SEEULEVELS) ? permstr : "", msgstr);
	i++;
	usercounter++;
	return 0;
}

/*******************Modify following two functions to support Type 2 mailall by Ashinmarch 2008.3.30*******************/
/*******************œÍœ∏Àµ√˜º˚mail.cµƒmailtoall∫Ø ˝********************************************************************/
static int mailto(void *uentpv, int index, void *args) {
	char filename[STRLEN];
	sprintf(filename, "tmp/mailall.%s", currentuser.userid);

	struct userec *uentp = (struct userec *)uentpv;
	if ((!(uentp->userlevel & PERM_BINDMAIL) && mailmode == 1) ||
			(uentp->userlevel & PERM_BOARDS && mailmode == 3)
			|| (uentp->userlevel & PERM_SPECIAL0 && mailmode == 4)
			|| (uentp->userlevel & PERM_SPECIAL9 && mailmode == 5)) {
		mail_file(filename, uentp->userid, save_title);
		//added by iamfat 2003.11.03 to avoid offline for timeout
		uinfo.idle_time = time(0);
		update_ulist(&uinfo, utmpent);
		//added end.
	}
	/***************∞—type2∂¿¡¢≥ˆ¿¥◊ˆ≈–∂œ£¨µ˜”√sharedmail_file∫Ø ˝************************/
	else if (uentp->userlevel & PERM_POST && mailmode == 2) {
		sharedmail_file(args, uentp->userid, save_title);
		uinfo.idle_time = time(0);
		update_ulist(&uinfo, utmpent);
	}
	/******end*******/
	return 1;
}

int mailtoall(int mode, char *fname)
{
	/******** π”√apply_record∫Ø ˝÷–µƒvoid *args≤Œ ˝¥´µ›π≤œÌŒƒº˛µƒŒƒº˛√˚*********/
	mailmode = mode;
	if (apply_record(PASSFILE, mailto, sizeof(struct userec),
			(char*)fname , 0, 0, false) == -1) {
		prints("No Users Exist");
		pressreturn();
		return 0;
	}
	return 1;
}
Show_Users() {

	usercounter = 0;
	modify_user_mode(LAUSERS);
	printuent((struct userec *) NULL);
	if (apply_record(PASSFILE, printuent, sizeof(struct userec), 0, 0, 0, false)
			== -1) {
		prints("No Users Exist");
		pressreturn();
		return 0;
	}
	clrtobot();
	return 0;
}
setlistrange(i)
int i;
{
	range = i;
}

do_query(star, curr)
int star, curr;
{
	if (user_record[curr]->userid != NULL) {
		clear();
		t_query(user_record[curr]->userid);
		move(t_lines - 1, 0);
		prints("[0;1;37;44m¡ƒÃÏ[[1;32mt[37m] ºƒ–≈[[1;32mm[37m] ÀÕ—∂œ¢[[1;32ms[37m] º”,ºı≈Û”—[[1;32mo[37m,[1;32md[37m] —°‘Ò π”√’ﬂ[[1;32m°¸[37m,[1;32m°˝[37m] «–ªªƒ£ Ω [[1;32mf[37m] «Ûæ»[[1;32mh[37m][m");
	}
}
do_query2(star, curr)
int star, curr;
{
	if (user_data[curr - star].userid != NULL) {
		t_query(user_data[curr - star].userid);
		move(t_lines - 1, 0);
		prints("[0;1;37;44m          ºƒ–≈[[1;32mm[37m] º”,ºı≈Û”—[[1;32mo[37m,[1;32md[37m] ø¥Àµ√˜µµ[[1;32m°˙[37m,[1;32mRtn[37m] —°‘Ò[[1;32m°¸[37m,[1;32m°˝[37m] «Ûæ»[[1;32mh[37m]          [m");
	}
}

Users() {
	range = allusers();
	modify_user_mode(LAUSERS);
	clear();
	user_data = (struct userec *) calloc(sizeof(struct userec),
			BBS_PAGESIZE);
	choose(NA, 0, print_title2, deal_key2, Show_Users, do_query2);
	clear();
	free(user_data);
	return;
}

int t_friends() {
	char buf[STRLEN];

	modify_user_mode(FRIEND);
	real_user_names = 0;
	friendmode = YEA;
	setuserfile(buf, "friends");
	if (!dashf(buf)) {
		move(1, 0);
		clrtobot();
		prints("ƒ˙…–Œ¥¿˚”√ Info -> Override …Ë∂®∫√”—√˚µ•£¨À˘“‘...\n");
		range = 0;
	} else {
		num_alcounter();
		range = count_friends;
	}
	if (range == 0) {
		move(2, 0);
		clrtobot();
		prints("ƒø«∞Œﬁ∫√”—…œœﬂ\n");
		move(BBS_PAGESIZE + 3, 0);
		if (askyn(" «∑Ò◊™ªª≥… π”√’ﬂƒ£ Ω", YEA, NA) == YEA) {
			range = num_visible_users();
			if (range>0) {
				freshmode = 1;
				page = -1;
				friendmode = NA;
				update_time = 0;
				choose(YEA, 0, print_title, deal_key, show_userlist,
						do_query);
				clear();
				return;
			}
		}
	} else {
		update_time = 0;
		choose(YEA, 0, print_title, deal_key, show_userlist, do_query);
	}
	clear();
	friendmode = NA;
	return;
}

int t_users() {
	friendmode = NA;
	modify_user_mode(LUSERS);
	real_user_names = 0;
	range = num_visible_users();
	if (range == 0) {
		move(3, 0);
		clrtobot();
		prints("ƒø«∞Œﬁ π”√’ﬂ…œœﬂ\n");
	}
	update_time = 0;
	choose(YEA, 0, print_title, deal_key, show_userlist, do_query);
	clear();
	return 0;
}

int
choose(update, defaultn, title_show, key_deal, list_show, read)
int update;
int defaultn;
int (*title_show)();
int (*key_deal)();
int (*list_show)();
int (*read)();
{
	int num = 0;
	int ch, number, deal;
	readplan = NA;
	(*title_show) ();
	signal(SIGALRM, SIG_IGN);
	if (update == 1)
		update_data();
	page = -1;
	number = 0;
	num = defaultn;
	while (1) {
		if (num <= 0)
		num = 0;
		if (num >= range)
		num = range - 1;
		if (page < 0 || freshmode == 1) {
			freshmode = 0;
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			move(3, 0);
			clrtobot();
			if ((*list_show) () == -1)
			return -1;
			update_endline();
		}
		if (num < page || num >= page + BBS_PAGESIZE) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			if ((*list_show) () == -1)
			return -1;
			update_endline();
			continue;
		}
		if (readplan == YEA) {
			if ((*read) (page, num) == -1)
			return num;
		} else {
			move(3 + num - page, 0);
			prints(">", number);
		}
		ch = egetch();
		if (readplan == NA)
		move(3 + num - page, 0);
		prints(" ");
		if (ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF) {
			if (readplan == YEA) {
				readplan = NA;
				move(1, 0);
				clrtobot();
				if ((*list_show) () == -1)
				return -1;
				(*title_show) ();
				continue;
			}
			break;
		}
		deal = (*key_deal) (ch, num, page);
		if (range == 0)
		break;
		if (deal == 1)
		continue;
		else if (deal == -1)
		break;
		switch (ch) {
			case 'b':
			case Ctrl('B'):
			case KEY_PGUP:
			if (num == 0)
			num = range - 1;
			else
			num -= BBS_PAGESIZE;
			break;
			case ' ':
			if (readplan == YEA) {
				if (++num >= range)
				num = 0;
				break;
			}
			case 'N':
			case Ctrl('F'):
			case KEY_PGDN:
			if (num == range - 1)
			num = 0;
			else
			num += BBS_PAGESIZE;
			break;
			case 'p':
			case 'l':
			case KEY_UP:
			if (num-- <= 0)
			num = range - 1;
			break;
			case 'n':
			case 'j':
			case KEY_DOWN:
			if (++num >= range)
			num = 0;
			break;
			case KEY_TAB:
			if (HAS_PERM(PERM_OCHAT)) {
				if(st!=stState)st++;
				else st=stUserID;
				fill_userlist();
				freshmode=1;
			}
			break;
			case '$':
			case KEY_END:
			num = range - 1;
			break;
			case KEY_HOME:
			num = 0;
			break;
			case '\n':
			case '\r':
			if (number> 0) {
				num = number - 1;
				break;
			}
			/* fall through */
			case KEY_RIGHT:
			{
				if (readplan == YEA) {
					if (++num >= range)
					num = 0;
				} else
				readplan = YEA;
				break;
			}
			default:
			;
		}
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
	}
	signal(SIGALRM, SIG_IGN);
	return -1;
}

enum {
	LIST_START = 3,
};

int choose2(choose_t *cp)
{
	int ch, ret, number = 0;
	bool end = false;

	cp->cur = 0;
	cp->start = -1;

	clear();
	(*cp->title)(cp);

	while (!end) {
		// Rolling.
		if (cp->cur < 0)
			cp->cur = cp->all - 1;
		if (cp->cur >= cp->all)
			cp->cur = 0;

		if (cp->start < 0 || cp->update == PARTUPDATE) {
			cp->update = DONOTHING;
			cp->start = (cp->cur / BBS_PAGESIZE) * BBS_PAGESIZE;
			move(LIST_START, 0);
			clrtobot();
			if ((*cp->display)(cp) == -1)
				return -1;
			update_endline();
		}

		if (cp->cur < cp->start || cp->cur >= cp->start + BBS_PAGESIZE) {
			cp->start = (cp->cur / BBS_PAGESIZE) * BBS_PAGESIZE;
			if ((*cp->display)(cp) == -1)
				return -1;
			update_endline();
			continue;
		}

		move(LIST_START + cp->cur - cp->start, 0);
		outs(">");

		ch = igetkey();

		move(LIST_START + cp->cur - cp->start, 0);
		outs(" ");

		switch (ch) {
			case 'q':
			case 'e':
			case 'KEY_LEFT':
			case EOF:
				end = true;
				break;
			case 'b':
			case Ctrl('B'):
			case KEY_PGUP:
				cp->cur -= BBS_PAGESIZE;
				break;
			case 'N':
			case Ctrl('F'):
			case KEY_PGDN:
				cp->cur += BBS_PAGESIZE;
				break;
			case 'p':
			case 'l':
			case KEY_UP:
				cp->cur--;
				break;
			case 'n':
			case 'j':
			case KEY_DOWN:
				cp->cur++;
				break;
			case '$':
			case KEY_END:
				cp->cur = cp->all - 1;
				break;
			case KEY_HOME:
				cp->cur = 0;
				break;
			case '\n':
			case '\r':
				if (number > 0) {
					cp->cur = number - 1;
					break;
				}
				// fall through
			default:
				if (ch >= '0' && ch <= '9') {
					number = number * 10 + (ch - '0');
					ch = '\0';
				} else {
					number = 0;
					ret = (*cp->handler)(cp);
				if (ret < 0)
					end = true;
				}
				break;
		}
	}
	return 0;
}
