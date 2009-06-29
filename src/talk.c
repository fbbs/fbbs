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
 $Id: talk.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define M_INT 8			/* monitor mode update interval */
#define P_INT 20		/* interval to check for page req. in
				 * talk/chat */
extern char BoardName[];
extern int iscolor;
extern int numf, friendmode;
extern char *cexpstr();
int talkidletime = 0;
int ulistpage;
int friendflag = 1;
int friend_query();
int friend_mail();
int friend_dele();
int friend_add();
int friend_edit();
int friend_help();
int reject_query();
int reject_dele();
int reject_add();
int reject_edit();
int reject_help();

#ifdef TALK_LOG
void do_log();
int talkrec = -1;
char partner[IDLEN + 1];
#endif

struct one_key friend_list[] = { 'r', friend_query, 'm', friend_mail, 'M',
		friend_mail, 'a', friend_add, 'A', friend_add, 'd', friend_dele,
		'D', friend_dele, 'E', friend_edit, 'h', friend_help, 'H',
		friend_help, '\0', NULL };

struct one_key reject_list[] = { 'r', reject_query, 'a', reject_add, 'A',
		reject_add, 'd', reject_dele, 'D', reject_dele, 'E', reject_edit,
		'h', reject_help, 'H', reject_help, '\0', NULL };

struct talk_win {
	int curcol, curln;
	int sline, eline;
};

int nowmovie;
int bind();

static char *refuse[] = { "±§Ç¸£¬ÎÒÏÖÔÚÏë×¨ÐÄ¿´ Board¡£    ",
		"Çë²»Òª³³ÎÒ£¬ºÃÂð£¿..... :)      ", "ÎÒÏÖÔÚÓÐÊÂ£¬µÈÒ»ÏÂÔÙ Call Äã¡£  ",
		"ÎÒÂíÉÏÒªÀë¿ªÁË£¬ÏÂ´ÎÔÙÁÄ°É¡£    ", "ÇëÄã²»ÒªÔÙ Page£¬ÎÒ²»Ïë¸úÄãÁÄ¡£ ", "ÇëÏÈÐ´Ò»·â×ÔÎÒ½éÉÜ¸øÎÒ£¬ºÃÂð£¿  ",
		"¶Ô²»Æð£¬ÎÒÏÖÔÚÔÚµÈÈË¡£          ", "ÎÒ½ñÌìºÜÀÛ£¬²»Ïë¸ú±ðÈËÁÄÌì¡£    ", NULL };

extern int t_columns;

char save_page_requestor[40];
extern int t_cmpuids();
int cmpfnames();
char *ModeType();

int
ishidden(user)
char *user;
{
	int tuid;
	struct user_info uin;
	if (!(tuid = getuser(user)))
	return 0;
	if (!search_ulist(&uin, t_cmpuids, tuid))
	return 0;
	return (uin.invisible);
}
char pagerchar(int friend, int pager) {
	if (pager & ALL_PAGER)
		return ' ';
	if ((friend)
			) {
				if (pager & FRIEND_PAGER) return 'O';
				else return '#';
			}
			return '*';
		}

		int canpage(int friend, int pager) {
			if ((pager & ALL_PAGER) || HAS_PERM(PERM_OCHAT))
				return YEA;
			if ((pager & FRIEND_PAGER)) {
				if (friend) return YEA;
			}
			return NA;
		}
#ifdef SHOW_IDLE_TIME
		char *
		idle_str(uent)
		struct user_info *uent;
		{
			static char hh_mm_ss[32];
			time_t now, diff;
			int limit, hh, mm;
			if (uent == NULL) {
				strcpy(hh_mm_ss, "²»Ïê");
				return hh_mm_ss;
			}

			now = time(0);

			if ( uent->mode == TALK )
			diff = talkidletime; /* ÁÄÌìÁíÓÐ×Ô¼ºµÄ idle kick »úÖÆ */
			else if (uent->mode == BBSNET )
			diff = 0;
			else
			diff = now - uent->idle_time;

#ifdef DOTIMEOUT
			/*
			 * the 60 * 60 * 24 * 5 is to prevent fault /dev mount from kicking
			 * out all users
			 */

			if (uent->ext_idle)
			limit = IDLE_TIMEOUT * 3;
			else
			limit = IDLE_TIMEOUT;

			if ((diff> limit) && (diff < 86400 * 5)&&uent->pid)
				bbskill(uent, SIGHUP);
#endif

			hh = diff / 3600;
			mm = (diff / 60) % 60;

			if (hh> 0)
			sprintf(hh_mm_ss, "%02d:%02d", hh, mm);
			else if (mm> 0)
			sprintf(hh_mm_ss, "%d", mm);
			else
			sprintf(hh_mm_ss, "   ");

			return hh_mm_ss;
		}
#endif

int listcuent(struct user_info *uentp) {
	if (uentp == NULL) {
		CreateNameList();
		return 0;
	}
	if (uentp->uid == usernum)
		return 0;
	if (!uentp->active || !uentp->pid || isreject(uentp))
		return 0;
	if (uentp->invisible && !(HAS_PERM(PERM_SEECLOAK)))
		return 0;
	AddNameList(uentp->userid);
	return 0;
}

void creat_list() {
	listcuent(NULL);
	apply_ulist(listcuent);
}

int t_pager() {

	if (uinfo.pager & ALL_PAGER) {
		uinfo.pager &= ~ALL_PAGER;
		if (DEFINE(DEF_FRIENDCALL))
			uinfo.pager |= FRIEND_PAGER;
		else
			uinfo.pager &= ~FRIEND_PAGER;
	} else {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}

	if (!uinfo.in_chat && uinfo.mode != TALK) {
		move(1, 0);
		clrtoeol();
		prints("ÄúµÄºô½ÐÆ÷ (pager) ÒÑ¾­[1m%s[mÁË!",
				(uinfo.pager & ALL_PAGER) ? "´ò¿ª" : "¹Ø±Õ");
		pressreturn();
	}
	update_ulist(&uinfo, utmpent);
	return 0;
}

/*Add by SmallPig*/
/*´Ëº¯ÊýÖ»¸ºÔðÁÐÓ¡ËµÃ÷µµ£¬²¢²»¹ÜÇå³ý»ò¶¨Î»µÄÎÊÌâ¡£*/

int
show_user_plan(userid)
char *userid;
{
	char pfile[STRLEN];
	sethomefile(pfile, userid, "plans");
	if (show_one_file(pfile)==NA) {
		prints("[1;36mÃ»ÓÐ¸öÈËËµÃ÷µµ[m\n");
		return NA;
	}
	return YEA;
}
int show_one_file(char *filename) {
	int i, j, ci;
	char pbuf[256];
	FILE *pf;
	if ((pf = fopen(filename, "r")) == NULL) {
		return NA;
	} else {
		for (i = 1; i <= MAXQUERYLINES; i++) {
			if (fgets(pbuf, sizeof(pbuf), pf)) {
				for (j = 0; j < strlen(pbuf); j++)
					//Modified by IAMFAT 2002.06.05
					if (pbuf[j] != '\033') {
						if (pbuf[j]!='\r')
							outc(pbuf[j]);
					} else {
						ci = strspn(&pbuf[j], "\033[0123456789;");
						if (pbuf[ci + j] != 'm')
							j += ci;
						else
							outc(pbuf[j]);
					}
			} else
				break;
		}
		fclose(pf);
		return YEA;
	}
}

int show_bm(char* userid, char *bmstring) {
	FILE *bmfp;
	char bmfilename[STRLEN], tmp[20];
	int i;
	sethomefile(bmfilename, userid, ".bmfile");
	bmfp = fopen(bmfilename, "r");

	if (bmfp) {
		for (i = 0; i < 3; i++) {
			fscanf(bmfp, "%s\n", tmp);
			bmstring += sprintf(bmstring, "%s ", tmp);
			if (feof(bmfp))
				break;
		}
		fclose(bmfp);
		return 1;
	}
	return 0;
}

int show_volunteer(char *userid, char *volstring) {
	FILE *fp;
	char filename[STRLEN], tmp[20];
	sethomefile(filename, userid, ".volunteer");
	fp = fopen(filename, "r");
	int i = 0;
	if (fp) {
		while (!feof(fp)) {
			i++;
			fscanf(fp, "%s\n", tmp);
			sprintf(volstring, "%s%s%s", volstring, i == 1 ? "" : " ", tmp);
		}
		if (i == 0)
			return 0;
		fclose(fp);
		return 1;
	}
	return 0;
}

int show_position(char *buf) {
	if (lookupuser.userlevel & PERM_SPECIAL9) {
		if (lookupuser.userlevel & PERM_SYSOPS) {
			sprintf(buf, "[\033[1;32mÕ¾³¤\033[m]");
		} else if (lookupuser.userlevel & PERM_ANNOUNCE) {
			sprintf(buf, "[\033[1;32mÕ¾Îñ\033[m]");
		} else if (lookupuser.userlevel & PERM_OCHAT) {
			sprintf(buf, "[\033[1;32mÊµÏ°Õ¾Îñ\033[m]");
		} else if (lookupuser.userlevel & PERM_SPECIAL0) {
			sprintf(buf, "[\033[1;32mÕ¾ÎñÎ¯Ô±»áÃØÊé\033[m]");
		} else {
			sprintf(buf, "[\033[1;32mÀëÈÎÕ¾Îñ\033[m]");
		}
	} else {
		int normal = 1;
		char bms[STRLEN];
		bms[0] = '\0';
		if ((lookupuser.userlevel & PERM_XEMPT) && (lookupuser.userlevel
				& PERM_LONGLIFE)
				&& (lookupuser.userlevel & PERM_LARGEMAIL)) {
			buf += sprintf(buf, "[\033[1;32mÈÙÓþ°æÖ÷\033[m]");
			normal = 0;
		}

		if (show_volunteer(lookupuser.userid, bms)) {
			buf += sprintf(buf, "[\033[1;33m%s\033[m]", bms);
			normal = 0;
			bms[0] = 0;
		}

		if ((lookupuser.userlevel & PERM_BOARDS) && show_bm(
				lookupuser.userid, bms)) {
			buf += sprintf(buf, "[\033[1;33m%s\033[32m°æ°æÖ÷\033[m]", bms);
			normal = 0;
		}
		if (lookupuser.userlevel & PERM_ARBI) {
			buf += sprintf(buf, "[\033[1;32mÖÙ²Ã×é\033[m]");
			normal = 0;
		}
		if (lookupuser.userlevel & PERM_SERV) {
			buf += sprintf(buf, "[\033[1;32mÅàÑµ×é\033[m]");
			normal = 0;
		}
		if (lookupuser.userlevel & PERM_SPECIAL3) {
			buf += sprintf(buf, "[\033[1;32mÃÀ¹¤×é\033[m]");
			normal = 0;
		}
		if (lookupuser.userlevel & PERM_TECH) {
			buf += sprintf(buf, "[\033[1;32m¼¼Êõ×é\033[m]");
			normal = 0;
		}
		if (normal) {
#ifndef FDQUAN
			sprintf(buf, "[\033[1;32m¹â»ªÍøÓÑ\033[m]");
#else
			sprintf(buf, "[\033[1;32mÈªÕ¾ÍøÓÑ\033[m]");
#endif
		}
	}
}

extern char fromhost[60];

int t_search_ulist(struct user_info *uentp, int (*fptr) (), int farg, int show, int doTalk)
{
	int i, num;
	char col[14];

	resolve_utmp();
	num = 0;
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
		if ((*fptr)(farg, uentp)) {
			if (!uentp->active || !uentp->pid || isreject(uentp)) {
				continue;
			}
			if ( (uentp->invisible==0) ||(uentp->uid == usernum)
					||(uentp->invisible==1) &&HAS_PERM(PERM_SEECLOAK) ) {
				num++;
			} else {
				continue;
			}
			if (!show)
				continue;
			if (num == 1)
				prints("Ä¿Ç° %s ×´Ì¬ÈçÏÂ£º\n", uentp->userid);
			if (uentp->invisible)
				strcpy(col, "[Òþ][1;36m");
			else if (uentp->mode == POSTING || uentp->mode == MARKET)
				strcpy(col, "[1;32m");
			else if (uentp->mode == FIVE || uentp->mode == BBSNET)
				strcpy(col, "[1;33m");
			else
				strcpy(col, "[1m");
			if (doTalk) {
				prints(
						"(%d) ×´Ì¬£º%s%-10s[m£¬À´×Ô£º%.20s\n",
						num,
						col,
						ModeType(uentp->mode),
#ifdef SHOWMETOFRIEND		    
						/* The following line is modified by Amigo 2002.04.02. Let sysop view fromhost at »·¹ËËÄ·½. */
						((uentp->from[22] != 'H')||hisfriend(uentp)
								||HAS_PERM(PERM_USER))?uentp->from:"......"
						);
#else
						/* The following line is modified by Amigo 2002.04.02. Let sysop view fromhost at »·¹ËËÄ·½. */
						((uentp->from[22] != 'H')|| HAS_PERM(PERM_USER)) ? uentp->from
								: "......");
#endif
			} else {
				prints("%s%-10s[m ", col, ModeType(uentp->mode));
				if ((num) % 5 == 0)
					outc('\n');
			}
		}
	}
	if (show)
		outc('\n');
	return num;
}

/* Modified By Excellent*/
int t_query(q_id)
char q_id[IDLEN + 2];
{
	char uident[STRLEN];
	int tuid = 0, clr = 0;
	int exp, perf,num; /* Add by SmallPig */
	struct user_info uin;
	char qry_mail_dir[STRLEN];
	char planid[IDLEN + 2], buf[50];
	time_t now;
	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;
	if ( uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND
			&& uinfo.mode != READING && uinfo.mode != MAIL && uinfo.mode != RMAIL
			&& uinfo.mode != GMENU) {
		modify_user_mode(QUERY);
		refresh();
		move(1, 0);
		clrtobot();
		prints("²éÑ¯Ë­:\n<ÊäÈëÊ¹ÓÃÕß´úºÅ, °´¿Õ°×¼ü¿ÉÁÐ³ö·ûºÏ×Ö´®>\n");
		move(1, 8);
		usercomplete(NULL, uident);
		if (uident[0] == '\0') return 0;
	} else {
		if (*q_id == '\0') return 0;
		if (strchr(q_id, ' ')) strtok(q_id, " ");
		strlcpy(uident, q_id, sizeof(uident));
		uident[sizeof(uident) - 1] = '\0';
	}
	if (!(tuid = getuser(uident))) {
		move(2, 0);
		clrtoeol();
		prints("[1m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m\n");
		pressanykey();
		return -1;
	}
	uinfo.destuid = tuid;
	update_ulist(&uinfo, utmpent);
	move(0, 0);
	clrtobot();
	sprintf(qry_mail_dir, "mail/%c/%s/%s", toupper(lookupuser.userid[0]), lookupuser.userid, DOT_DIR);
	exp = countexp(&lookupuser);
	perf = countperf(&lookupuser);
	if ( HAS_DEFINE(lookupuser.userdefine, DEF_COLOREDSEX) )
	clr = (lookupuser.gender == 'F') ? 5 : 6;
	else
	clr = 2;
	if ( strcasecmp(lookupuser.userid, "guest") != 0 )
		sprintf(buf, "[\033[1;3%dm%s\033[m] ", clr, horoscope(lookupuser.birthmonth, lookupuser.birthday));
	else
		sprintf(buf, "");
	if (!HAS_DEFINE(lookupuser.userdefine, DEF_S_HOROSCOPE))
		buf[0] = '\0';
	prints("[1;37m%s [m([1;33m%s[m) ¹²ÉÏÕ¾ [1;32m%d[m ´Î  %s\n",
			lookupuser.userid, lookupuser.username,lookupuser.numlogins, buf);
	strcpy(planid, lookupuser.userid);
	prints("ÉÏ ´Î ÔÚ:[[1;32m%s[m] ´Ó [[1;32m%s[m] µ½±¾Õ¾Ò»ÓÎ¡£\n",
			getdatestring(lookupuser.lastlogin, DATE_ZH),
			(lookupuser.lasthost[0] == '\0' ? "(²»Ïê)" : lookupuser.lasthost));
	num = t_search_ulist(&uin, t_cmpuids, tuid, NA, NA);
	if( num ) {
		search_ulist(&uin, t_cmpuids, getuser(lookupuser.userid));
		sprintf(genbuf, "Ä¿Ç°ÔÚÏß:[[1;32mÑ¶Ï¢Æ÷:([36m%s[32m) ºô½ÐÆ÷:([36m%s[32m)[m] ",
				canmsg(&uin)?"´ò¿ª":"¹Ø±Õ",
				canpage(hisfriend(&uin), uin.pager)?"´ò¿ª":"¹Ø±Õ");
		prints("%s",genbuf);
	} else {
		if(lookupuser.lastlogout < lookupuser.lastlogin) {
			now = ((time(0)-lookupuser.lastlogin)/120)%47+1+lookupuser.lastlogin;
		} else {
			now = lookupuser.lastlogout;
		}
		prints("ÀëÕ¾Ê±¼ä:[[1;32m%s[m] ", getdatestring(now, DATE_ZH));
	}
	prints("±íÏÖÖµ:"
#ifdef SHOW_PERF
			"%d([1;33m%s[m)"
#else
			"[[1;33m%s[m]"
#endif
			" ÐÅÏä:[[1;5;32m%2s[m]\n"
#ifdef SHOW_PERF
			, perf
#endif
			, cperf(perf), (check_query_mail(qry_mail_dir) == 1) ? "ÐÅ" : "  ");

#ifdef ALLOWGAME
	prints("ÒøÐÐ´æ¿î: [[1;32m%dÔª[m] Ä¿Ç°´û¿î: [[1;32m%dÔª[m]([1;33m%s[m) ¾­ÑéÖµ£º[[1;32m%d[m]([1;33m%s[m)¡£\n",
			lookupuser.money,lookupuser.bet,
			cmoney(lookupuser.money-lookupuser.bet),exp,cexpstr(exp));
	/* The following 2 lines are modified by Amigo 2002.04.02. Omit ÎÄÕÂÊýµÈ¼¶ÏÔÊ¾. */
	/*   prints("ÎÄ ÕÂ Êý: [[1;32m%d[m]([1;33m%s[m) ½±ÕÂÊý: [[1;32m%d[m]([1;33m%s[m) ÉúÃüÁ¦£º[[1;32m%d[m]\n",*/
	/*      lookupuser.numposts,cnumposts(lookupuser.numposts),*/
	prints("ÎÄ ÕÂ Êý: [[1;32m%d[m] ½±ÕÂÊý: [[1;32m%d[m]([1;33m%s[m) ÉúÃüÁ¦£º[[1;32m%d[m]\n",
			lookupuser.numposts,
			lookupuser.nummedals,cnummedals(lookupuser.nummedals),
			compute_user_value(&lookupuser));
#else
	/*
	 prints("ÎÄ ÕÂ Êý: [[1;32m%d[m]([1;33m%s[m)  ¾­ Ñé Öµ£º[[1;32m%d[m]([1;33m%s[m)¡£ÉúÃüÁ¦£º[[1;32m%d[m]\n",
	 lookupuser.numposts,cnumposts(lookupuser.numposts), 
	 exp,cexp(exp),compute_user_value(&lookupuser));
	 *///modified by roly
	/* The following 2 lines are modified by Amigo 2002.04.02. Omit ÎÄÕÂÊýµÈ¼¶ÏÔÊ¾. */
	/* prints("ÎÄ ÕÂ Êý: [[1;32m%d[m]([1;33m%s[m)  ¾­ Ñé Öµ£º([1;33m%s[m)¡£ÉúÃüÁ¦£º[[1;32m%d[m]\n",
	 lookupuser.numposts,cnumposts(lookupuser.numposts), */
	prints("ÎÄ ÕÂ Êý:[[1;32m%d[m] ¾­ Ñé Öµ:"
#ifdef SHOWEXP
			"%d([1;33m%-10s[m)"
#else
			"[[1;33m%-10s[m]"
#endif
			" ÉúÃüÁ¦:[[1;32m%d[m]\n",
			lookupuser.numposts,
#ifdef SHOWEXP
			exp,
#endif
			cexpstr(exp),compute_user_value(&lookupuser));

#endif
	genbuf[0] = '\0';
	show_position(genbuf);
	prints("Éí·Ý: %s\n", genbuf);
	t_search_ulist(&uin, t_cmpuids, tuid, YEA, NA);
#if defined(QUERY_REALNAMES)
	if (HAS_PERM(PERM_SYSOPS)) prints("ÕæÊµÐÕÃû:%s \n", lookupuser.realname);
#endif
	show_user_plan(planid);

	/*
	 {
	 char query_log[100];
	 sprintf(query_log,"%s from %s", lookupuser.userid, fromhost);
	 do_report("QUERY_LOG", query_log);
	 }
	 */
	if ( uinfo.mode != LUSERS && uinfo.mode != LAUSERS
			&& uinfo.mode != FRIEND && uinfo.mode != GMENU)
	pressanykey();
	uinfo.destuid = 0;
	return 0;
}

int
count_active(uentp)
struct user_info *uentp;
{
	static int count;
	if (uentp == NULL) {
		int c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
	return 0;
	count++;
	return 1;
}

// ¼ÆËãÊ¹ÓÃÍâ²¿³ÌÐòµÄÈËÊý
int count_useshell(struct user_info *uentp) {
	static int count;
	if (uentp == NULL) {
		int c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->mode == SYSINFO	|| uentp->mode == DICT
		|| uentp->mode == BBSNET || uentp->mode == FIVE
		|| uentp->mode == GAME 	|| uentp->mode == LOGIN)
		count++;
	return 1;
}

int
count_user_logins(uentp)
struct user_info *uentp;
{
	static int count;
	if (uentp == NULL) {
		int c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
	return 0;
	if (!strcmp(uentp->userid, save_page_requestor))
	count++;
	return 1;
}

int count_visible_active(struct user_info *uentp) {
	static int count;

	if (uentp == NULL) {
		int c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid || isreject(uentp))
		return 0;
	if ( (!uentp->invisible)||(uentp->uid==usernum)
			||(HAS_PERM(PERM_SEECLOAK)))
		count ++;
	return 1;
}

int num_alcounter() {
	static time_t last=0;
	register int i;
	time_t now = time(0);
	if (last+10 > now)
		return;
	last=now;
	count_friends=0;
	for(i = 0; i < MAXFRIENDS && uinfo.friend[i]; i++) {
		count_friends+=get_status(uinfo.friend[i]);
	}
	return;
}

//	·µ»ØÊ¹ÓÃÍâ²¿³ÌÐòµÄÈËÊý
int num_useshell() {
	count_useshell(NULL);
	apply_ulist(count_useshell);
	return count_useshell(NULL);
}

int num_visible_users() {
	count_visible_active(NULL);
	apply_ulist(count_visible_active);
	return count_visible_active(NULL);
}

int
cmpfnames(userid, uv)
char *userid;
struct override *uv;
{
	return !strcasecmp(userid, uv->id);
}

int t_cmpuids(int uid, struct user_info* up) {
	return (up->active && uid == up->uid);
}

int t_talk() {
	int netty_talk;
#ifdef DOTIMEOUT
	init_alarm();
#else
	signal(SIGALRM, SIG_IGN);
#endif
	refresh();
	netty_talk = ttt_talk();
	clear();
	return (netty_talk);
}

int
ttt_talk(userinfo)
struct user_info *userinfo;
{
	char uident[STRLEN];
	char reason[STRLEN];
	int tuid, ucount, unum, tmp;
#ifdef FIVEGAME
	/*added by djq,99.07.19,for FIVE */
	int five=0;
#endif
	struct user_info uin;
	move(1, 0);
	clrtobot();
	/*
	 if (uinfo.invisible) {
	 move(2, 0);
	 prints("±§Ç¸, ´Ë¹¦ÄÜÔÚÒþÉí×´Ì¬ÏÂ²»ÄÜÖ´ÐÐ...\n");
	 pressreturn();
	 return 0;
	 }
	 */
	if (uinfo.mode != LUSERS && uinfo.mode != FRIEND) {
		move(2, 0);
		prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n");
		move(1, 0);
		clrtoeol();
		prints("¸úË­ÁÄÌì: ");
		creat_list();
		namecomplete(NULL, uident);
		if (uident[0] == '\0') {
			clear();
			return 0;
		}
		if (!(tuid = searchuser(uident)) || tuid == usernum) {
			wrongid:
			move(2, 0);
			prints("´íÎó´úºÅ\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
		ucount = t_search_ulist(&uin, t_cmpuids, tuid, NA, YEA);
		if (ucount> 1) {
			list: move(3, 0);
			ucount = t_search_ulist(&uin, t_cmpuids, tuid,YEA,YEA);
			clrtobot();
			tmp = ucount + 5;
			getdata(tmp,0,"ÇëÑ¡Ò»¸öÄã¿´µÄ±È½ÏË³ÑÛµÄ [0 -- ²»ÁÄÁË]: ",
					genbuf, 4, DOECHO, YEA);
			unum = atoi(genbuf);
			if (unum == 0) {
				clear();
				return 0;
			}
			if (unum> ucount || unum < 0) {
				move(tmp, 0);
				prints("±¿±¿£¡ÄãÑ¡´íÁËÀ²£¡\n");
				clrtobot();
				pressreturn();
				goto list;
			}
			if (!search_ulistn(&uin, t_cmpuids, tuid, unum))
			goto wrongid;
		} else if (!search_ulist(&uin, t_cmpuids, tuid))
		goto wrongid;
	} else {
		/*     memcpy(&uin,userinfo,sizeof(uin));     */
		uin = *userinfo;
		tuid = uin.uid;
		strcpy(uident, uin.userid);
		move(1, 0);
		clrtoeol();
		prints("¸úË­ÁÄÌì: %s", uin.userid);
	}
	/* youzi : check guest */
	if (!strcmp(uin.userid, "guest") && !HAS_PERM(PERM_OCHAT))
	return -1;

	/* check if pager on/off       --gtv */
	if (!canpage(hisfriend(&uin), uin.pager)) {
		move(2, 0);
		prints("¶Ô·½ºô½ÐÆ÷ÒÑ¹Ø±Õ.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}
	if (uin.mode == SYSINFO || uin.mode == BBSNET
			|| uin.mode == DICT || uin.mode == ADMIN
			|| uin.mode == LOCKSCREEN || uin.mode == GAME
			|| uin.mode == PAGE
			|| uin.mode == FIVE || uin.mode == LOGIN) {
		move(2, 0);
		prints("Ä¿Ç°ÎÞ·¨ºô½Ð.\n");
		clrtobot();
		pressreturn();
		return -1;
	}
	if (!uin.active || (bbskill(&uin, 0) == -1)) {
		move(2, 0);
		prints("¶Ô·½ÒÑÀë¿ª\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	} else {
		int sock, msgsock, length;
		struct sockaddr_in server = {0};
		char c;
#ifdef FIVEGAME
		char answer[2]="";
#endif 
		char buf[512];
		move(3, 0);
		clrtobot();
		show_user_plan(uident);
#ifndef FIVEGAME
		/* modified by djq,99.07.19,for FIVE */
		move(2, 0);
		if (askyn("È·¶¨ÒªºÍËû/ËýÌ¸ÌìÂð", NA, NA) == NA) {
			clear();
			return 0;
		}
#else
		getdata(2, 0, "ÏëÕÒ¶Ô·½Ì¸ÌìÇë°´\'y\',ÏÂ¡ºÎå×ÓÆå¡»Çë°´\'w\'(Y/W/N)[N]:", answer, 4, DOECHO,YEA);
		if(*answer != 'y' && *answer != 'w')
		{
			clear();
			return 0;
		}

		if( *answer == 'w' )
		five = 1;

		if( five == 1 )
		sprintf(buf,"FIVE to %s",uident);
		else
		sprintf(buf,"TALK to %s",uident);

		/* modified end. */
#endif
#ifndef FIVEGAME
		sprintf(buf, "Talk to '%s'", uident);
#endif
		report(buf, currentuser.userid);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) return -1;
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = 0;
		if (bind(sock,(struct sockaddr *)&server,sizeof(server))<0)
		return -1;
		length = sizeof(server);
		if (getsockname(sock,(struct sockaddr *)&server,&length)<0)
		return -1;
		uinfo.sockactive = YEA;
		uinfo.sockaddr = server.sin_port;
		uinfo.destuid = tuid;
#ifndef FIVEGAME
		/* modified by djq,99.07.19,for FIVE */

		modify_user_mode(PAGE);
#else
		if( five == 1)
		modify_user_mode(PAGE_FIVE);
		else
		modify_user_mode(PAGE);

		/* modified end */
#endif
		bbskill(&uin, SIGUSR1);
		clear();
		prints("ºô½Ð %s ÖÐ...\nÊäÈë Ctrl-D ½áÊø\n", uident);

		listen(sock, 1);
		add_io(sock, 20);
		while (YEA) {
			int ch;
			ch = igetkey();
			if (ch == I_TIMEOUT) {
				move(0, 0);
				prints("ÔÙ´Îºô½Ð.\n");
				add_io(sock,20);/* 1999.12.20 */
				bell();
				if (bbskill(&uin, SIGUSR1) == -1) {
					move(0, 0);
					prints("¶Ô·½ÒÑÀëÏß\n");
					pressreturn();
					/* by SmallPig 2 lines */
					uinfo.sockactive = NA;
					uinfo.destuid = 0;
					return -1;
				}
				continue;
			}
			if (ch == I_OTHERDATA)
			break;
			if (ch == '\004') {
				add_io(0, 0);
				close(sock);
				uinfo.sockactive = NA;
				uinfo.destuid = 0;
				clear();
				return 0;
			}
		}

		msgsock = accept(sock, (struct sockaddr *) 0, (int *) 0);
		add_io(0, 0);
		close(sock);
		uinfo.sockactive = NA;
		/*      uinfo.destuid = 0 ;*/
		read(msgsock, &c, sizeof(c));

		clear();

		switch (c) {
#ifdef FIVEGAME
			case 'y': case 'Y': case 'w': case 'W': /*added for FIVE,by djq.*/
			/* modified by djq,99.07.19,for FIVE */
			sprintf( save_page_requestor, "%s (%s)", uin.userid, uin.username);
			if( five == 1)
			five_pk(msgsock,1);
			else
			do_talk(msgsock);
			break;
#else

			case 'y':
			case 'Y':
			sprintf(save_page_requestor, "%s (%s)", uin.userid, uin.username);
			do_talk(msgsock);
			break;
#endif
			case 'a':
			case 'A':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[0]);
			pressreturn();
			break;
			case 'b':
			case 'B':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[1]);
			pressreturn();
			break;
			case 'c':
			case 'C':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[2]);
			pressreturn();
			break;
			case 'd':
			case 'D':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[3]);
			pressreturn();
			break;
			case 'e':
			case 'E':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[4]);
			pressreturn();
			break;
			case 'f':
			case 'F':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[5]);
			pressreturn();
			break;
			case 'g':
			case 'G':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[6]);
			pressreturn();
			break;
			case 'n':
			case 'N':
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, refuse[7]);
			pressreturn();
			break;
			case 'm':
			case 'M':
			read(msgsock, reason, sizeof(reason));
			prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, reason);
			pressreturn();
			default:
			sprintf(save_page_requestor, "%s (%s)", uin.userid, uin.username);
#ifdef TALK_LOG
			strcpy(partner, uin.userid);
#endif
			do_talk(msgsock);
			break;
		}
		close(msgsock);
		clear();
		refresh();
		uinfo.destuid = 0;
	}
	return 0;
}

extern int talkrequest;
struct user_info ui;
char page_requestor[STRLEN];
char page_requestorid[STRLEN];

int
cmpunums(unum, up)
int unum;
struct user_info *up;
{
	if (!up->active)
	return 0;
	return (unum == up->destuid);
}

int
cmpmsgnum(unum, up)
int unum;
struct user_info *up;
{
	if (!up->active)
	return 0;
	return (unum == up->destuid && up->sockactive == 2);
}

int
setpagerequest(mode)
int mode;
{
	int tuid;
	if (mode == 0)
	tuid = search_ulist(&ui, cmpunums, usernum);
	else
	tuid = search_ulist(&ui, cmpmsgnum, usernum);
	if (tuid == 0)
	return 1;
	if (!ui.sockactive)
	return 1;
	uinfo.destuid = ui.uid;
	sprintf(page_requestor, "%s (%s)", ui.userid, ui.username);
	strcpy(page_requestorid, ui.userid);
	return 0;
}

int
servicepage(line, mesg)
int line;
char *mesg;
{
	static time_t last_check;
	time_t now;
	char buf[STRLEN];
	int tuid = search_ulist(&ui, cmpunums, usernum);
	if (tuid == 0 || !ui.sockactive)
	talkrequest = NA;
	if (!talkrequest) {
		if (page_requestor[0]) {
			switch (uinfo.mode) {
				case TALK:
#ifdef FIVEGAME
				case FIVE: //added by djq,for five
#endif
				move(line, 0);
				printdash(mesg);
				break;
				default: /* a chat mode */
				sprintf(buf, "** %s ÒÑÍ£Ö¹ºô½Ð.", page_requestor);
				printchatline(buf);
			}
			memset(page_requestor, 0, STRLEN);
			last_check = 0;
		}
		return NA;
	} else {
		now = time(0);
		if (now - last_check> P_INT) {
			last_check = now;
			if (!page_requestor[0] && setpagerequest(0 /* For Talk */))
			return NA;
			else
			switch (uinfo.mode) {
				case TALK:
				move(line, 0);
				sprintf(buf, "** %s ÕýÔÚºô½ÐÄã", page_requestor);
				printdash(buf);
				break;
				default: /* chat */
				sprintf(buf, "** %s ÕýÔÚºô½ÐÄã", page_requestor);
				printchatline(buf);
			}
		}
	}
	return YEA;
}

int talkreply() {
	int a;
	struct hostent *h;
	char buf[512];
	char reason[51];
	char hostname[STRLEN];
	struct sockaddr_in sin= { 0 };
	char inbuf[STRLEN * 2];
#ifdef FIVEGAME
	/* added by djq 99.07.19,for FIVE */
	struct user_info uip;
	int five=0;
	int tuid;
#endif
	talkrequest = NA;
	if (setpagerequest(0 /* For Talk */))
		return 0;
#ifdef DOTIMEOUT
	init_alarm();
#else
	signal(SIGALRM, SIG_IGN);
#endif
	clear();
#ifdef FIVEGAME
	/* modified by djq, 99.07.19, for FIVE */
	if(!(tuid = getuser(page_requestorid)))
	return 0;
	who_callme( &uip, t_cmpuids, tuid, uinfo.uid );
	uinfo.destuid = uip.uid;
	getuser(uip.userid);
	if(uip.mode == PAGE_FIVE)
	five=1;

#endif
	move(5, 0);
	clrtobot();
	show_user_plan(page_requestorid);
	move(1, 0);
	prints("(A)¡¾%s¡¿(B)¡¾%s¡¿\n", refuse[0], refuse[1]);
	prints("(C)¡¾%s¡¿(D)¡¾%s¡¿\n", refuse[2], refuse[3]);
	prints("(E)¡¾%s¡¿(F)¡¾%s¡¿\n", refuse[4], refuse[5]);
	prints("(G)¡¾%s¡¿(N)¡¾%s¡¿\n", refuse[6], refuse[7]);
	prints("(M)¡¾ÁôÑÔ¸ø %-13s            ¡¿\n", page_requestorid);
#ifndef FIVEGAME
	/* modified by djq ,99.07.19, for FIVE */

	sprintf(inbuf, "ÄúÏë¸ú %s ÁÄÁÄÌìÂð? (Y N A B C D E F G M)[Y]: ",
			page_requestor);
#else
	sprintf(inbuf, "ÄúÏë¸ú %s %sÂð£¿ÇëÑ¡Ôñ(Y/N/A/B/C/D)[Y] ",page_requestor,(five)?"ÏÂÎå×ÓÆå":"ÁÄÁÄÌì");
#endif
	strcpy(save_page_requestor, page_requestor);
#ifdef TALK_LOG
	strcpy(partner, page_requestorid);
#endif
	memset(page_requestor, 0, sizeof(page_requestor));
	memset(page_requestorid, 0, sizeof(page_requestorid));
	getdata(0, 0, inbuf, buf, 2, DOECHO, YEA);
	gethostname(hostname, STRLEN);
	if (!(h = gethostbyname(hostname))) {
		prints("%s\n", strerror(errno));
		return -1;
	}
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = h->h_addrtype;
	memcpy(&sin.sin_addr, h->h_addr, h->h_length);
	sin.sin_port = ui.sockaddr;
	a = socket(sin.sin_family, SOCK_STREAM, 0);
	if ((connect(a, (struct sockaddr *) &sin, sizeof(sin))))
		return -1;
	if (buf[0] != 'A' && buf[0] != 'a' && buf[0] != 'B' && buf[0] != 'b'
			&& buf[0] != 'C' && buf[0] != 'c' && buf[0] != 'D' && buf[0]
			!= 'd' && buf[0] != 'e' && buf[0] != 'E' && buf[0] != 'f'
			&& buf[0] != 'F' && buf[0] != 'g' && buf[0] != 'G' && buf[0]
			!= 'n' && buf[0] != 'N' && buf[0] != 'm' && buf[0] != 'M')
		buf[0] = 'y';
	if (buf[0] == 'M' || buf[0] == 'm') {
		move(1, 0);
		clrtobot();
		getdata(1, 0, "Áô»°£º", reason, 50, DOECHO, YEA);
	}
	write(a, buf, 1);
	if (buf[0] == 'M' || buf[0] == 'm')
		write(a, reason, sizeof(reason));
	if (buf[0] != 'y') {
		close(a);
		report("page refused", currentuser.userid);
		clear();
		refresh();
		return 0;
	}
	report("page accepted", currentuser.userid);
	clear();
#ifndef FIVEGAME
	/* modified by djq 99.07.19 for FIVE */
	do_talk(a) ;
#else 
	if(!five)
	do_talk(a);
	else
	five_pk(a,0);
#endif
	close(a);
	clear();
	refresh();
	return 0;
}

void
do_talk_nextline(twin)
struct talk_win *twin;
{

	twin->curln = twin->curln + 1;
	if (twin->curln> twin->eline)
	twin->curln = twin->sline;
	if (twin->curln != twin->eline) {
		move(twin->curln + 1, 0);
		clrtoeol();
	}
	move(twin->curln, 0);
	clrtoeol();
	twin->curcol = 0;
}

void
do_talk_char(twin, ch)
struct talk_win *twin;
int ch;
{
	extern int dumb_term;
	if (isprint2(ch)) {
		if (twin->curcol < 79) {
			move(twin->curln, (twin->curcol)++);
			prints("%c", ch);
			return;
		}
		do_talk_nextline(twin);
		twin->curcol++;
		prints("%c", ch);
		return;
	}
	switch (ch) {
		case Ctrl('H'):
		case '\177':
		if (dumb_term)
		ochar(Ctrl('H'));
		if (twin->curcol == 0) {
			return;
		}
		(twin->curcol)--;
		move(twin->curln, twin->curcol);
		if (!dumb_term)
		prints(" ");
		move(twin->curln, twin->curcol);
		return;
		case Ctrl('M'):
		case Ctrl('J'):
		if (dumb_term)
		prints("\n");
		do_talk_nextline(twin);
		return;
		case Ctrl('G'):
		bell();
		return;
		default:
		break;
	}
	return;
}

void
do_talk_string(twin, str)
struct talk_win *twin;
char *str;
{
	while (*str) {
		do_talk_char(twin, *str++);
	}
}

char talkobuf[80];
int talkobuflen;
int talkflushfd;

void talkflush() {
	if (talkobuflen)
		write(talkflushfd, talkobuf, talkobuflen);
	talkobuflen = 0;
}

int
moveto(mode, twin)
int mode;
struct talk_win *twin;
{
	if (mode == 1)
	twin->curln--;
	if (mode == 2)
	twin->curln++;
	if (mode == 3)
	twin->curcol++;
	if (mode == 4)
	twin->curcol--;
	if (twin->curcol < 0) {
		twin->curln--;
		twin->curcol = 0;
	} else if (twin->curcol> 79) {
		twin->curln++;
		twin->curcol = 0;
	}
	if (twin->curln < twin->sline) {
		twin->curln = twin->eline;
	}
	if (twin->curln> twin->eline) {
		twin->curln = twin->sline;
	}
	move(twin->curln, twin->curcol);
}

void endmsg() {
	int x, y;
	int tmpansi;
	tmpansi = showansi;
	showansi = 1;
	talkidletime += 60;
	if (talkidletime >= IDLE_TIMEOUT)
		kill(getpid(), SIGHUP);
	if (uinfo.in_chat == YEA)
		return;
	getyx(&x, &y);
	update_endline();
	signal(SIGALRM, endmsg);
	move(x, y);
	refresh();
	alarm(60);
	showansi = tmpansi;
	return;
}

int do_talk(int fd) {
	struct talk_win mywin, itswin;
	char mid_line[256];
	int page_pending = NA;
	int i, i2;
	char ans[3];
	int previous_mode;
#ifdef TALK_LOG
	char mywords[80], itswords[80], talkbuf[80];
	int mlen = 0, ilen = 0;
	time_t now;
	mywords[0] = itswords[0] = '\0';
#endif

	signal(SIGALRM, SIG_IGN);
	endmsg();
	refresh();
	previous_mode = uinfo.mode;
	modify_user_mode(TALK);
	sprintf(mid_line, " %s (%s) ºÍ %s ÕýÔÚ³©Ì¸ÖÐ", currentuser.userid,
			currentuser.username, save_page_requestor);

	memset(&mywin, 0, sizeof(mywin));
	memset(&itswin, 0, sizeof(itswin));
	i = (t_lines - 1) / 2;
	mywin.eline = i - 1;
	itswin.curln = itswin.sline = i + 1;
	itswin.eline = t_lines - 2;
	move(i, 0);
	printdash(mid_line);
	move(0, 0);

	talkobuflen = 0;
	talkflushfd = fd;
	add_io(fd, 0);
	add_flush(talkflush);

	while (YEA) {
		int ch;
		if (talkrequest)
			page_pending = YEA;
		if (page_pending)
			page_pending = servicepage((t_lines - 1) / 2, mid_line);
		ch = igetkey();
		talkidletime = 0;
		if (ch == '') {
			igetkey();
			igetkey();
			continue;
		}
		if (ch == I_OTHERDATA) {
			char data[80];
			int datac;
			register int i;
			datac = read(fd, data, 80);
			if (datac <= 0)
				break;
			for (i = 0; i < datac; i++) {
				if (data[i] >= 1 && data[i] <= 4) {
					moveto(data[i] - '\0', &itswin);
					continue;
				}
#ifdef TALK_LOG
				/*
				 * Sonny.990514 add an robust and fix some
				 * logic problem
				 */
				/*
				 * Sonny.990606 change to different algorithm
				 * and fix the
				 */
				/* existing do_log() overflow problem       */
				else if (isprint2(data[i])) {
					if (ilen >= 80) {
						itswords[80] = '\0';
						(void) do_log(itswords, 2);
						ilen = 0;
					} else {
						itswords[ilen] = data[i];
						ilen++;
					}
				} else if ((data[i] == Ctrl('H') || data[i] == '\177') && !ilen) {
					itswords[ilen--] = '\0';
				} else if (data[i] == Ctrl('M') || data[i] == '\r' ||
						data[i] == '\n') {
					itswords[ilen] = '\0';
					(void) do_log(itswords, 2);
					ilen = 0;
				}
#endif
				do_talk_char(&itswin, data[i]);
			}
		} else {
			if (ch == Ctrl('D') || ch == Ctrl('C'))
				break;
			if (isprint2(ch) || ch == Ctrl('H') || ch == '\177' || ch
					== Ctrl('G')) {
				talkobuf[talkobuflen++] = ch;
				if (talkobuflen == 80)
					talkflush();
#ifdef TALK_LOG
				if (mlen < 80) {
					if ((ch == Ctrl('H') || ch == '\177') && mlen != 0) {
						mywords[mlen--] = '\0';
					} else {
						mywords[mlen] = ch;
						mlen++;
					}
				} else if (mlen >= 80) {
					mywords[80] = '\0';
					(void) do_log(mywords, 1);
					mlen = 0;
				}
#endif
				do_talk_char(&mywin, ch);
			} else if (ch == '\n' || ch == Ctrl('M') || ch == '\r') {
#ifdef TALK_LOG
				if (mywords[0] != '\0') {
					mywords[mlen++] = '\0';
					(void) do_log(mywords, 1);
					mlen = 0;
				}
#endif
				talkobuf[talkobuflen++] = '\r';
				talkflush();
				do_talk_char(&mywin, '\r');
			} else if (ch >= KEY_UP && ch <= KEY_LEFT) {
				moveto(ch - KEY_UP + 1, &mywin);
				talkobuf[talkobuflen++] = ch - KEY_UP + 1;
				if (talkobuflen == 80)
					talkflush();
			} else if (ch == Ctrl('E')) {
				for (i2 = 0; i2 <= 10; i2++) {
					talkobuf[talkobuflen++] = '\r';
					talkflush();
					do_talk_char(&mywin, '\r');
				}
			} else if (ch == Ctrl('P') && HAS_PERM(PERM_LOGIN)) {
				t_pager();
				update_ulist(&uinfo, utmpent);
				update_endline();
			}
		}
	}
	add_io(0, 0);
	talkflush();
	signal(SIGALRM, SIG_IGN);
	add_flush(NULL);
	modify_user_mode(previous_mode);

#ifdef TALK_LOG
	/* edwardc.990106 ÁÄÌì¼ÍÂ¼ */
	mywords[mlen] = '\0';
	itswords[ilen] = '\0';

	if (mywords[0] != '\0')
	do_log(mywords, 1);
	if (itswords[0] != '\0')
	do_log(itswords, 2);

	now = time(NULL);
	sprintf(talkbuf, "\n\033[1;34mÍ¨»°½áÊø, Ê±¼ä: %s \033[m\n",
			getdatestring(now, DATE_ENWEEK));
	write(talkrec, talkbuf, strlen(talkbuf));

	close(talkrec);

	sethomefile(genbuf, currentuser.userid, "talklog");

	getdata(23, 0, "ÊÇ·ñ¼Ä»ØÁÄÌì¼ÍÂ¼ [Y/n]: ", ans, 2, DOECHO, YEA);

	switch (ans[0]) {
		case 'n':
		case 'N':
			break;
		default:
			sethomefile(talkbuf, currentuser.userid, "talklog");
			sprintf(mywords, "¸ú %s µÄÁÄÌì¼ÇÂ¼ [%s]", partner,
					getdatestring(now, DATE_ENWEEK) + 4);
			{
				char temp[STRLEN];
				strcpy(temp, save_title);
				mail_file(talkbuf, currentuser.userid, mywords);
				strcpy(save_title,temp);
			}
		}
	sethomefile(talkbuf, currentuser.userid, "talklog");
	unlink(talkbuf);
#endif

	return 0;
}

int
shortulist(uentp)
struct user_info *uentp;
{
	int i;
	int pageusers = 60;
	extern struct user_info *user_record[];
	extern int range;
	fill_userlist(-1);
	if (ulistpage> ((range - 1) / pageusers))
	ulistpage = 0;
	if (ulistpage < 0)
	ulistpage = (range - 1) / pageusers;
	move(1, 0);
	clrtoeol();
	prints("Ã¿¸ô [1;32m%d[m Ãë¸üÐÂÒ»´Î£¬[1;32mCtrl-C[m »ò [1;32mCtrl-D[m Àë¿ª£¬[1;32mF[m ¸ü»»Ä£Ê½ [1;32m¡ü¡ý[m ÉÏ¡¢ÏÂÒ»Ò³ µÚ[1;32m %1d[m Ò³", M_INT, ulistpage + 1);
	clrtoeol();
	move(3, 0);
	clrtobot();
	for (i = ulistpage * pageusers; i < (ulistpage + 1) * pageusers && i < range; i++) {
		char ubuf[STRLEN];
		int ovv;
		if (i < numf || friendmode)
		ovv = YEA;
		else
		ovv = NA;
		sprintf(ubuf, "%s%-12.12s %s%-10.10s[m", (ovv) ? "[1;32m¡Ì" : "  ", user_record[i]->userid, (user_record[i]->invisible == YEA) ? "[1;34m" : "",ModeType(user_record[i]->mode));
		//modestring(user_record[i]->mode, user_record[i]->destuid, 0, NULL));
		prints("%s", ubuf);
		if ((i + 1) % 3 == 0)
		outc('\n');
		else
		prints(" |");
	}
	return range;
}

int
do_list(modestr)
char *modestr;
{
	char buf[STRLEN];
	int count;
	extern int RMSG;
	if (RMSG != YEA) { /* Èç¹ûÊÕµ½ Msg µÚÒ»ÐÐ²»ÏÔÊ¾¡£ */
		move(0, 0);
		clrtoeol();
		if (chkmail())
		showtitle(modestr, "[ÄúÓÐÐÅ¼þ]");
		else
		showtitle(modestr, BoardName);
	}
	move(2, 0);
	clrtoeol();
	sprintf(buf, "  %-12s %-10s", "Ê¹ÓÃÕß´úºÅ", "Ä¿Ç°¶¯Ì¬");
	prints("[1;33;44m%s |%s |%s[m", buf, buf, buf);
	count = shortulist();
	if (uinfo.mode == MONITOR) {
		move(t_lines - 1, 0);
		sprintf(genbuf,"[1;44;33m  Ä¿Ç°ÓÐ [32m%3d[33m %6sÉÏÏß, Ê±¼ä: [32m%22.22s [33m, Ä¿Ç°×´Ì¬£º[36m%10s   [m"
				,count, friendmode ? "ºÃÅóÓÑ" : "Ê¹ÓÃÕß", getdatestring(time(NULL), DATE_ZH), friendmode ? "ÄãµÄºÃÅóÓÑ" : "ËùÓÐÊ¹ÓÃÕß");
		prints(genbuf);
	}
	refresh();
	return 0;
}

int t_list() {
	modify_user_mode(LUSERS);
	report("t_list", currentuser.userid);
	do_list("Ê¹ÓÃÕß×´Ì¬");
	pressreturn();
	refresh();
	clear();
	return 0;
}

void sig_catcher() {
	ulistpage++;
	if (uinfo.mode != MONITOR) {
#ifdef DOTIMEOUT
		init_alarm();
#else
		signal(SIGALRM, SIG_IGN);
#endif
		return;
	}
	if (signal(SIGALRM, sig_catcher) == SIG_ERR)
		exit(1);
	do_list("Ì½ÊÓÃñÇé");
	alarm(M_INT);
}

int t_monitor() {
	int i;
	char modestr[] = "Ì½ÊÓÃñÇé";
	alarm(0);
	signal(SIGALRM, sig_catcher);
	/*    idle_monitor_time = 0; */
	report("monitor", currentuser.userid);
	modify_user_mode(MONITOR);
	ulistpage = 0;
	do_list(modestr);
	alarm(M_INT);
	while (YEA) {
		i = egetch();
		if (i == 'f' || i == 'F') {
			if (friendmode == YEA)
				friendmode = NA;
			else
				friendmode = YEA;
			do_list(modestr);
		}
		if (i == KEY_DOWN) {
			ulistpage++;
			do_list(modestr);
		}
		if (i == KEY_UP) {
			ulistpage--;
			do_list(modestr);
		}
		if (i == Ctrl('D') || i == Ctrl('C') || i == KEY_LEFT)
			break;
		/*        else if (i == -1) {
		 if (errno != EINTR) exit(1);
		 } else idle_monitor_time = 0;*/
	}
	move(2, 0);
	clrtoeol();
	clear();
	return 0;
}

int listfilecontent(char *fname, int y) {
	FILE *fp;
	int x = 0, cnt = 0, max = 0, len;
	//char    u_buf[20], line[STRLEN], *nick;
	char u_buf[20], line[512], *nick;
	//modified by roly 02.03.22 »º´æÇøÒç³ö
	move(y, x);
	CreateNameList();
	strcpy(genbuf, fname);
	if ((fp = fopen(genbuf, "r")) == NULL) {
		prints("(none)\n");
		return 0;
	}
	while (fgets(genbuf, 1024, fp) != NULL) {
		strtok(genbuf, " \n\r\t");
		strlcpy(u_buf, genbuf, 20);
		u_buf[19] = '\0';
		if (!AddNameList(u_buf))
			continue;
		nick = (char *) strtok(NULL, "\n\r\t");
		if (nick != NULL) {
			while (*nick == ' ')
				nick++;
			if (*nick == '\0')
				nick = NULL;
		}
		if (nick == NULL) {
			strcpy(line, u_buf);
		} else {
			sprintf(line, "%-12s%s", u_buf, nick);
		}
		if ((len = strlen(line)) > max)
			max = len;
		if (x + len > 78)
			line[78 - x] = '\0';
		prints("%s", line);
		cnt++;
		if ((++y) >= t_lines - 1) {
			y = 3;
			x += max + 2;
			max = 0;
			if (x > 70)
				break;
		}
		move(y, x);
	}
	fclose(fp);
	if (cnt == 0)
		prints("(none)\n");
	return cnt;
}

int add_to_file(filename, str)
//char    filename[STRLEN], str[STRLEN];
char filename[256], str[256];
{
	FILE *fp;
	int rc;
	if ((fp = fopen(filename, "a")) == NULL)
	return -1;
	FLOCK(fileno(fp), LOCK_EX);
	rc = fprintf(fp, "%s\n", str);
	FLOCK(fileno(fp), LOCK_UN);
	fclose(fp);
	return (rc == EOF ? -1 : 1);
}

int
addtooverride(uident)
char *uident;
{
	struct override tmp;
	int n;
	char buf[STRLEN];
	char desc[5];
	memset(&tmp, 0, sizeof(tmp));
	if (friendflag) {
		setuserfile(buf, "friends");
		n = MAXFRIENDS;
		strcpy(desc, "ºÃÓÑ");
	} else {
		setuserfile(buf, "rejects");
		n = MAXREJECTS;
		strcpy(desc, "»µÈË");
	}
	if (get_num_records(buf, sizeof(struct override)) >= n) {
		move(t_lines - 2, 0);
		clrtoeol();
		prints("±§Ç¸£¬±¾Õ¾Ä¿Ç°½ö¿ÉÒÔÉè¶¨ %d ¸ö%s, Çë°´ÈÎºÎ¼þ¼ÌÐø...", n, desc);
		igetkey();
		move(t_lines - 2, 0);
		clrtoeol();
		return -1;
	} else {
		if (friendflag) {
			if (myfriend(searchuser(uident))) {
				sprintf(buf, "%s ÒÑÔÚºÃÓÑÃûµ¥", uident);
				show_message(buf);
				return -1;
			}
		} else if (search_record(buf, &tmp, sizeof(tmp), cmpfnames, uident)> 0) {
			sprintf(buf, "%s ÒÑÔÚ»µÈËÃûµ¥", uident);
			show_message(buf);
			return -1;
		}
	}
	if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND)
	n = 2;
	else
	n = t_lines - 2;

	strcpy(tmp.id, uident);
	move(n, 0);
	clrtoeol();
	refresh();
	sprintf(genbuf, "ÇëÊäÈë¸ø%s¡¾%s¡¿µÄËµÃ÷: ", desc, tmp.id);
	getdata(n, 0, genbuf, tmp.exp, 40, DOECHO, YEA);

	n = append_record(buf, &tmp, sizeof(struct override));
	if (n != -1)
	(friendflag) ? getfriendstr() : getrejectstr();
	else
	report("append override error", currentuser.userid);
	return n;
}

int del_from_file(filename, str)
char filename[STRLEN], str[STRLEN];
{
	FILE *fp, *nfp;
	int deleted = NA;
	char tmpbuf[1024],fnnew[STRLEN];

	if ((fp = fopen(filename, "r")) == NULL) return -1;
	sprintf(fnnew, "%s.%d", filename, getuid());
	if ((nfp = fopen(fnnew, "w")) == NULL) {
		fclose(nfp);
		return -1;
	}
	while (fgets(tmpbuf, 1024, fp) != NULL) {
		if (strncmp(tmpbuf, str, strlen(str)) == 0
				&&(tmpbuf[strlen(str)]=='\0'||tmpbuf[strlen(str)]==' '||tmpbuf[strlen(str)]=='\n'))
		deleted = YEA;
		else if (*tmpbuf> ' ') fputs(tmpbuf, nfp);
	}
	fclose(fp);
	fclose(nfp);
	if (!deleted) return -1;
	return (rename(fnnew, filename) + 1);
}

int
deleteoverride(uident, filename)
char *uident;
char *filename;
{
	int deleted;
	struct override fh;
	char buf[STRLEN];
	int oldstate=in_mail;
	setuserfile(buf, filename);
	deleted = search_record(buf, &fh, sizeof(fh), cmpfnames, uident);
	if (deleted> 0) {
		in_mail=YEA;
		if (delete_record(buf, sizeof(fh), deleted,NULL,NULL) != -1) {
			(friendflag) ? getfriendstr() : getrejectstr();
		} else {
			deleted = -1;
			report("delete override error", currentuser.userid);
		}
		in_mail=oldstate;
	}
	return (deleted> 0) ? 1 : -1;
}
override_title() {
	char desc[5];
	if (chkmail())
		strcpy(genbuf, "[ÄúÓÐÐÅ¼þ]");
	else
		strcpy(genbuf, BoardName);
	if (friendflag) {
		showtitle("[±à¼­ºÃÓÑÃûµ¥]", genbuf);
		strcpy(desc, "ºÃÓÑ");
	} else {
		showtitle("[±à¼­»µÈËÃûµ¥]", genbuf);
		strcpy(desc, "»µÈË");
	}
	prints(
			" [[1;32m¡û[m,[1;32me[m] Àë¿ª [[1;32mh[m] ÇóÖú [[1;32m¡ú[m,[1;32mRtn[m] %sËµÃ÷µµ [[1;32m¡ü[m,[1;32m¡ý[m] Ñ¡Ôñ [[1;32ma[m] Ôö¼Ó%s [[1;32md[m] É¾³ý%s\n",
			desc, desc, desc);
	prints(
			"[1;44m ±àºÅ  %s´úºÅ      %sËµÃ÷                                                   [m\n",
			desc, desc);
}

char *
override_doentry(ent, fh)
int ent;
struct override *fh;
{
	static char buf[STRLEN];
	sprintf(buf, " %4d  %-12.12s  %s", ent, fh->id, fh->exp);
	return buf;
}

int
override_edit(ent, fh, direc)
int ent;
struct override *fh;
char *direc;
{
	struct override nh;
	char buf[STRLEN / 2];
	int pos;
	pos = search_record(direc, &nh, sizeof(nh), cmpfnames, fh->id);
	move(t_lines - 2, 0);
	clrtoeol();
	if (pos> 0) {
		sprintf(buf, "ÇëÊäÈë %s µÄÐÂ%sËµÃ÷: ", fh->id, (friendflag) ? "ºÃÓÑ" : "»µÈË");
		getdata(t_lines - 2, 0, buf, nh.exp, 40, DOECHO, NA);
	}
	if (substitute_record(direc, &nh, sizeof(nh), pos) < 0)
	report("Override files subs err", currentuser.userid);
	move(t_lines - 2, 0);
	clrtoeol();
	return NEWDIRECT;
}

int
override_add(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	char uident[13];
	clear();
	move(1, 0);
	usercomplete("ÇëÊäÈëÒªÔö¼ÓµÄ´úºÅ: ", uident);
	while (uident[0] != '\0') {
		if (getuser(uident) <= 0) {
			move(2, 0);
			prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
			pressanykey();
			return FULLUPDATE;
		} else
		addtooverride(uident);
		move(2,0);
		prints("\n°Ñ %s ¼ÓÈë%sÃûµ¥ÖÐ...", uident, (friendflag) ? "ºÃÓÑ" : "»µÈË");
		//		pressanykey();
		move(1,0);
		usercomplete("ÇëÊäÈëÒªÔö¼ÓµÄ´úºÅ: ", uident);
	}
	return FULLUPDATE;
}

int
override_dele(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	char buf[STRLEN];
	char desc[5];
	char fname[10];
	int deleted = NA;
	if (friendflag) {
		strcpy(desc, "ºÃÓÑ");
		strcpy(fname, "friends");
	} else {
		strcpy(desc, "»µÈË");
		strcpy(fname, "rejects");
	}
	saveline(t_lines - 2, 0);
	move(t_lines - 2, 0);
	sprintf(buf, "ÊÇ·ñ°Ñ¡¾%s¡¿´Ó%sÃûµ¥ÖÐÈ¥³ý", fh->id, desc);
	if (askyn(buf, NA, NA) == YEA) {
		move(t_lines - 2, 0);
		clrtoeol();
		if (deleteoverride(fh->id, fname) == 1) {
			prints("ÒÑ´Ó%sÃûµ¥ÖÐÒÆ³ý¡¾%s¡¿,°´ÈÎºÎ¼ü¼ÌÐø...", desc, fh->id);
			deleted = YEA;
		} else
		prints("ÕÒ²»µ½¡¾%s¡¿,°´ÈÎºÎ¼ü¼ÌÐø...", fh->id);
	} else {
		move(t_lines - 2, 0);
		clrtoeol();
		prints("È¡ÏûÉ¾³ý%s...", desc);
	}
	igetkey();
	move(t_lines - 2, 0);
	clrtoeol();
	saveline(t_lines - 2, 1);
	return (deleted) ? PARTUPDATE : DONOTHING;
}

int
friend_edit(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = YEA;
	return override_edit(ent, fh, direct);
}

int
friend_add(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = YEA;
	return override_add(ent, fh, direct);
}

int
friend_dele(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = YEA;
	return override_dele(ent, fh, direct);
}

int
friend_mail(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	/* Following line modified by Amigo 2002.06.08. To add mail right. */
	/*	if (!HAS_PERM(PERM_POST))*/
	if (!HAS_PERM(PERM_MAIL))
	return DONOTHING;
	m_send(fh->id);
	return FULLUPDATE;
}

int
friend_query(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	int ch;
	if (t_query(fh->id) == -1)
	return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints("[0;1;44;31m[¶ÁÈ¡ºÃÓÑËµÃ÷µµ][33m ¼ÄÐÅ¸øºÃÓÑ m ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»Î» ¡ü©¦ÏÂÒ»Î» <Space>,¡ý      [m");
	ch = egetch();
	switch (ch) {
		case 'N':
		case 'Q':
		case 'n':
		case 'q':
		case KEY_LEFT:
		break;
		case 'm':
		case 'M':
		m_send(fh->id);
		break;
		case ' ':
		case 'j':
		case KEY_RIGHT:
		case KEY_DOWN:
		case KEY_PGDN:
		return READ_NEXT;
		case KEY_UP:
		case KEY_PGUP:
		return READ_PREV;
		default:
		break;
	}
	return FULLUPDATE;
}

int friend_help() {
	show_help("help/friendshelp");
	return FULLUPDATE;
}

int
reject_edit(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = NA;
	return override_edit(ent, fh, direct);
}

int
reject_add(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = NA;
	return override_add(ent, fh, direct);
}

int
reject_dele(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = NA;
	return override_dele(ent, fh, direct);
}

int
reject_query(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	int ch;
	if (t_query(fh->id) == -1)
	return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints("[0;1;44;31m[¶ÁÈ¡»µÈËËµÃ÷µµ][33m ½áÊø Q,¡û ©¦ÉÏÒ»Î» ¡ü©¦ÏÂÒ»Î» <Space>,¡ý                      [m");
	ch = egetch();
	switch (ch) {
		case 'N':
		case 'Q':
		case 'n':
		case 'q':
		case KEY_LEFT:
		break;
		case ' ':
		case 'j':
		case KEY_RIGHT:
		case KEY_DOWN:
		case KEY_PGDN:
		return READ_NEXT;
		case KEY_UP:
		case KEY_PGUP:
		return READ_PREV;
		default:
		break;
	}
	return FULLUPDATE;
}

int reject_help() {
	show_help("help/rejectshelp");
	return FULLUPDATE;
}

void t_friend() {
	char buf[STRLEN];
	friendflag = YEA;
	setuserfile(buf, "friends");
	i_read(GMENU, buf, override_title, override_doentry, friend_list,
			sizeof(struct override));
	clear();
	return;
}

void t_reject() {
	char buf[STRLEN];
	friendflag = NA;
	setuserfile(buf, "rejects");
	i_read(GMENU, buf, override_title, override_doentry, reject_list,
			sizeof(struct override));
	clear();
	return;
}

struct user_info *t_search(char *sid, int pid) {
	int i;
	struct user_info *cur, *tmp = NULL;
	resolve_utmp();
	/* added by roly 02.06.02 */
	if (pid<0)
		return NULL;
	/* add end */

	for (i = 0; i < USHM_SIZE; i++) {
		cur = &(utmpshm->uinfo[i]);
		if (!cur->active || !cur->pid)
			continue;
		if (!strcasecmp(cur->userid, sid)) {
			if (pid == 0)
				return isreject(cur) ? NULL : cur;
			tmp = cur;
			if (pid == cur->pid)
				break;
		}
	}
	/*
	 if (tmp != NULL) {
	 if (tmp->invisible && !HAS_PERM(PERM_SEECLOAK))
	 return NULL;  
	 }
	 */
	return isreject(cur) ? NULL : tmp;
}

int
cmpfuid(a, b)
int *a, *b;
{
	return *a - *b;
}

int getfriendstr() {
	int i;
	struct override *tmp;
	memset(uinfo.friend, 0, sizeof(uinfo.friend));
	setuserfile(genbuf, "friends");
	uinfo.fnum = get_num_records(genbuf, sizeof(struct override));
	if (uinfo.fnum <= 0)
		return 0;
	uinfo.fnum = (uinfo.fnum >= MAXFRIENDS) ? MAXFRIENDS : uinfo.fnum;
	tmp = (struct override *) calloc(sizeof(struct override), uinfo.fnum);
	get_records(genbuf, tmp, sizeof(struct override), 1, uinfo.fnum);
	for (i = 0; i < uinfo.fnum; i++) {
		uinfo.friend[i] = searchuser(tmp[i].id);
		if (uinfo.friend[i] == 0)
		deleteoverride(tmp[i].id, "friends");
		/* Ë³±ãÉ¾³ýÒÑ²»´æÔÚÕÊºÅµÄºÃÓÑ */
	}
	free(tmp);
	qsort(&uinfo.friend, uinfo.fnum, sizeof(uinfo.friend[0]), cmpfuid);
	update_ulist(&uinfo, utmpent);
}

int getrejectstr() {
	int nr, i;
	struct override *tmp;
	memset(uinfo.reject, 0, sizeof(uinfo.reject));
	setuserfile(genbuf, "rejects");
	nr = get_num_records(genbuf, sizeof(struct override));
	if (nr <= 0)
		return 0;
	nr = (nr >= MAXREJECTS) ? MAXREJECTS : nr;
	tmp = (struct override *) calloc(sizeof(struct override), nr);
	get_records(genbuf, tmp, sizeof(struct override), 1, nr);
	for (i = 0; i < nr; i++) {
		uinfo.reject[i] = searchuser(tmp[i].id);
		if (uinfo.reject[i] == 0)
			deleteoverride(tmp[i].id, "rejects");
	}
	free(tmp);
	update_ulist(&uinfo, utmpent);
}

#ifdef TALK_LOG
/* edwardc.990106 ·Ö±ðÎªÁ½Î»ÁÄÌìµÄÈË×÷¼ÍÂ¼ */
/* -=> ×Ô¼ºËµµÄ»° */
/* --> ¶Ô·½ËµµÄ»° */

void
do_log(char *msg, int who)
{
	/* Sonny.990514 ÊÔÖø×¥ overflow µÄÎÊÌâ... */
	/* Sonny.990606 overflow ÎÊÌâ½â¾ö. buf[100] ÊÇÕýÈ·µÄ. ²Î¿¼ man sprintf() */
	time_t now;
	char buf[100];
	now = time(0);
	if (msg[strlen(msg)] == '\n')
	msg[strlen(msg)] = '\0';

	if (strlen(msg) < 1 || msg[0] == '\r' || msg[0] == '\n')
	return;

	/* Ö»°ï×Ô¼º×ö */
	sethomefile(buf, currentuser.userid, "talklog");

	if (!dashf(buf) || talkrec == -1) {
		talkrec = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0644);
		sprintf(buf, "\033[1;32mÓë %s µÄÇé»°ÃàÃà, ÈÕÆÚ: %s \033[m\n", save_page_requestor, getdatestring(now, DATE_ENWEEK));
		write(talkrec, buf, strlen(buf));
		sprintf(buf, "\tÑÕÉ«·Ö±ð´ú±í: [1;33m%s[m [1;36m%s[m \n\n", currentuser.userid, partner);
		write(talkrec, buf, strlen(buf));
	}
	if (who == 1) { /* ×Ô¼ºËµµÄ»° */
		sprintf(buf, "[1;33m-=> %s [m\n", msg);
		write(talkrec, buf, strlen(buf));
	} else if (who == 2) { /* ±ðÈËËµµÄ»° */
		sprintf(buf, "[1;36m--> %s [m\n", msg);
		write(talkrec, buf, strlen(buf));
	}
}
#endif
