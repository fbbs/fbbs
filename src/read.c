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
 $Id: read.c 338 2006-11-25 06:44:42Z danielfree $
 */

#include "bbs.h"

//½«¹â±êÒÆµ½µ±Ç°µÄÎ»ÖÃ,²¢ÏÔÊ¾³É>
#define PUTCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(">");
//Çå³ýÒÔÇ°µÄ¹â±ê±ê¼Ç,¼´°Ñ>¸Ä³É¿Õ¸ñ
#define RMVCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(" ");

struct fileheader SR_fptr;
int SR_BMDELFLAG = NA;
char *pnt;

extern int noreply;
extern int local_article;

struct keeploc {
	char *key;
	int top_line;
	int crs_line;
	struct keeploc *next;
};

/*struct fileheader *files = NULL;*/
char currdirect[STRLEN];
char keyword[STRLEN]; /* for Ïà¹ØÖ÷Ìâ */
int screen_len;
int last_line;

#ifdef ENABLE_NOTICE
char noticedirect[256];
int notice_lastline, dir_lastline;
#endif

//ÔÚ¾Ö²¿±íÌ¬Á´±íÖÐ²éÕÒÓë¹Ø¼ü×Ö·û´®sÏàÆ¥ÅäµÄÏî
//		ÕÒµ½:	Ö±½Ó·µ»Ø
//		·ñÔò:	ÐÂ¼ÓÒ»Ïî,·ÅÔÚÁ´±í±íÍ·,²¢·µ»ØÐÂ¼ÓÏî
struct keeploc * getkeep(char *s, int def_topline, int def_cursline) {
	static struct keeploc *keeplist = NULL;
	struct keeploc *p;
	for (p = keeplist; p != NULL; p = p->next) {
		if (!strcmp(s, p->key)) {
			if (p->crs_line < 1)
				p->crs_line = 1; /* DAMMIT! - rrr */
			return p;
		}
	}

	p = (struct keeploc *) malloc(sizeof(*p));
	p->key = (char *) malloc(strlen(s) + 1);
	strcpy(p->key, s);
	p->top_line = def_topline;
	p->crs_line = def_cursline;
	p->next = keeplist;
	keeplist = p;
	return p;
}

void fixkeep(char *s, int first, int last) {
	struct keeploc *k;
	k = getkeep(s, 1, 1);
	if (k->crs_line >= first) {
		k->crs_line = (first == 1 ? 1 : first - 1);
		k->top_line = (first < 11 ? 1 : first - 10);
	}
}

void modify_locmem(struct keeploc *locmem, int total) {
	if (locmem->top_line > total) {
		locmem->crs_line = total;
		locmem->top_line = total - t_lines / 2;
		if (locmem->top_line < 1)
			locmem->top_line = 1;
	} else if (locmem->crs_line > total) {
		locmem->crs_line = total;
	}
}

int
move_cursor_line(locmem, mode)
struct keeploc *locmem;
int mode;
{
	int top, crs;
	int reload = 0;
	top = locmem->top_line;
	crs = locmem->crs_line;
	if (mode == READ_PREV) {
		if (crs <= top) {
			top -= screen_len - 1;
			if (top < 1)
			top = 1;
			reload = 1;
		}
		crs--;
		if (crs < 1) {
			crs = 1;
			reload = -1;
		}
	} else if (mode == READ_NEXT) {
		if (crs + 1 >= top + screen_len) {
			top += screen_len - 1;
			reload = 1;
		}
		crs++;
		if (crs> last_line) {
			crs = last_line;
			reload = -1;
		}
	}
	locmem->top_line = top;
	locmem->crs_line = crs;
	return reload;
}

void
draw_title(dotitle)
int (*dotitle) ();
{
	clear();
	(*dotitle) ();
}

void
draw_entry(doentry, locmem, num, ssize)
char *(*doentry) ();
struct keeploc *locmem;
int num, ssize;
{
	char *str;
	int base, i;
	base = locmem->top_line;
	move(3, 0);
	clrtobot();
	for (i = 0; i < num; i++) {
		str = (*doentry) (base + i, &pnt[i * ssize]);
		if (!check_stuffmode())
		prints("%s", str);
		else
		showstuff(str);
		prints("\n");
	}
	move(t_lines - 1, 0);
	clrtoeol();
	update_endline();
}

void draw_bottom(char *buf) {
	char buf1[100];
	if (buf) {
		move(t_lines-1, 71);
		clrtoeol();
		sprintf(buf1, "\033[0;1;44;33m[%6.6s]\033[m", buf);
		prints(buf1);
	}
}

#ifdef ENABLE_NOTICE
void get_noticedirect(char *curr, char *notice) {
	char *ptr;
	strcpy(notice, curr);
	ptr=strrchr(notice,'/');
	if(!ptr) {
		ptr=notice;
	} else {
		ptr++;
	}
	strcpy(ptr, NOTICE_DIR);
}
#endif
void i_read(int cmdmode, char *direct, int (*dotitle) (), char *(*doentry) (), struct one_key *rcmdlist, int ssize) {
	extern int talkrequest;
	extern int friendflag;
	struct keeploc * locmem;
	char lbuf[11];
	char * ptr;
	int lbc, recbase, mode, ch;
	int num, entries;

	screen_len = t_lines - 4;
	modify_user_mode(cmdmode);
	ptr = pnt = calloc(screen_len, ssize);
	strcpy(currdirect, direct);
	draw_title(dotitle);
#ifndef ENABLE_NOTICE
	last_line = get_num_records(currdirect, ssize);
#else
	last_line=dir_lastline=get_num_records(currdirect, ssize);
	get_noticedirect(currdirect, noticedirect);
	if(digestmode==0)
	notice_lastline=get_num_records(noticedirect, ssize);
	else
	notice_lastline=0;
	if(digestmode==0)
	last_line+=notice_lastline;
#endif

	if (last_line == 0) {
		switch (cmdmode) {
			case RMAIL:
				prints("Ã»ÓÐÈÎºÎÐÂÐÅ¼þ...");
				pressreturn();
				clear();
				break;
			case GMENU: {
				char desc[5];
				char buf[40];
				if (friendflag)
					strcpy(desc, "ºÃÓÑ");
				else
					strcpy(desc, "»µÈË");
				sprintf(buf, "Ã»ÓÐÈÎºÎ%s (A)ÐÂÔö%s (Q)Àë¿ª£¿[Q] ", desc, desc);
				getdata(t_lines - 1, 0, buf, genbuf, 4, DOECHO, YEA);
				if (genbuf[0] == 'a' || genbuf[0] == 'A')
					(friendflag) ? friend_add() : reject_add();
			}
				break;
			case ADMIN:
				prints("Ä¿Ç°ÎÞ×¢²áµ¥...");
				pressreturn();
				clear();
				break;
			default:
				getdata(t_lines - 1, 0, "±¾°æÐÂ³ÉÁ¢ (P)·¢±íÎÄÕÂ (Q)Àë¿ª£¿[Q] ",
						genbuf, 4, DOECHO, YEA);
				if (genbuf[0] == 'p' || genbuf[0] == 'P')
					do_post();
		}
		free(pnt);
		return;
	}
	num = last_line - screen_len + 2;
	if (cmdmode==ADMIN)
		locmem = getkeep(currdirect, 1, 1);
	else
		locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);

	modify_locmem(locmem, last_line);
	recbase = locmem->top_line;
#ifdef ENABLE_NOTICE
	if(recbase>dir_lastline)
	entries = get_records(noticedirect, pnt, ssize, recbase-dir_lastline, screen_len);
	else {
#endif
	entries = get_records(currdirect, pnt, ssize, recbase, screen_len);
#ifdef ENABLE_NOTICE
	if(entries<screen_len && digestmode==0 && notice_lastline) {
		entries+=get_records(noticedirect, pnt+ssize*entries, ssize, 1, screen_len-entries);
	}
}
#endif
	draw_entry(doentry, locmem, entries, ssize);
	PUTCURS
	;
	lbc = 0;
	mode = DONOTHING;
	while ((ch = egetch()) != EOF) {
		if (talkrequest) {
			talkreply();
			mode = FULLUPDATE;
		} else if (ch >= '0' && ch <= '9') {
			if (lbc < 6) {
				if (lbc==1 && lbuf[0]=='0')
					lbc=0;
				lbuf[lbc++] = ch;
				lbuf[lbc]='\0';
				draw_bottom(lbuf);
			}
		} else if (lbc > 0 && (ch == '\n' || ch == '\r')) {
			lbuf[lbc] = '\0';
			lbc = atoi(lbuf);
			if (cursor_pos(locmem, lbc, 10))
				mode = PARTUPDATE;
			lbc = 0;
			lbuf[0]='\0';
			update_endline();
		} else if (lbc >0 && ch == '\x08') {
			lbc--;
			lbuf[lbc]='\0';
			draw_bottom(lbuf);
		} else {
			if (lbc!=0)
				update_endline();
			lbc = 0;
			mode = i_read_key(rcmdlist, locmem, ch, ssize);

			while (mode == READ_NEXT || mode == READ_PREV || mode
					== READ_AGAIN) {
				int reload;
				if (mode==READ_AGAIN)
					reload = 1;
				else
					reload = move_cursor_line(locmem, mode);
				if (reload == -1) {
					mode = FULLUPDATE;
					break;
				} else if (reload) {
					recbase = locmem->top_line;
#ifdef ENABLE_NOTICE
					if(recbase>dir_lastline) {
						entries = get_records(noticedirect, pnt, ssize,recbase-dir_lastline, screen_len);
					} else {
#endif
					entries = get_records(currdirect, pnt, ssize, recbase,
							screen_len);
#ifdef ENABLE_NOTICE
					if(entries<screen_len && digestmode==0 && notice_lastline) {
						entries+=get_records(noticedirect, pnt+ssize*entries, ssize, 1, screen_len-entries);
					}
				}
#endif
					if (entries <= 0) {
						last_line = -1;
						break;
					}
				}
				num = locmem->crs_line - locmem->top_line;
				mode = i_read_key(rcmdlist, locmem, ch, ssize);
			}
			modify_user_mode(cmdmode);
		}
		if (mode == DOQUIT)
			break;
		if (mode == GOTO_NEXT) {
			cursor_pos(locmem, locmem->crs_line + 1, 1);
			mode = PARTUPDATE;
		}
		switch (mode) {
			case NEWDIRECT:
			case DIRCHANGED:
			case MODECHANGED: // chenhao ½â¾öÎÄÕÂÁÐ±í¿´ÐÅµÄÎÊÌâ
				recbase = -1;
				if (mode == MODECHANGED) { // chenhao
					setbdir(currdirect, currboard);
					pnt = ptr;
				}
#ifndef ENABLE_NOTICE
				last_line = get_num_records(currdirect, ssize);
#else
				last_line=dir_lastline=get_num_records(currdirect, ssize);
				get_noticedirect(currdirect, noticedirect);
				if(digestmode==0)
				notice_lastline=get_num_records(noticedirect, ssize);
				else
				notice_lastline=0;
				if(digestmode==0)
				last_line+=notice_lastline;
#endif
				if (last_line == 0 && digestmode > 0) {
					acction_mode();
				}
				if (mode == NEWDIRECT) {
					num = last_line - screen_len + 1;
					locmem = getkeep(currdirect, num < 1 ? 1 : num,
							last_line);
				}
			case FULLUPDATE:
				draw_title(dotitle);
			case PARTUPDATE:
				if (last_line < locmem->top_line + screen_len) {
					num = get_num_records(currdirect, ssize);
#ifdef ENABLE_NOTICE
					if (dir_lastline != num) {
						dir_lastline = num;
						recbase = -1;
					}
#else
					if (last_line != num) {
						last_line = num;
						recbase = -1;
					}
#endif
				}
#ifdef ENABLE_NOTICE
				last_line=dir_lastline;
				if(digestmode==0)
				last_line+=notice_lastline;
#endif
				if (last_line == 0) {
					prints("No Messages\n");
					entries = 0;
				} else if (recbase != locmem->top_line) {
					recbase = locmem->top_line;
					if (recbase > last_line) {
						recbase = last_line - screen_len / 2;
						if (recbase < 1)
							recbase = 1;
						locmem->top_line = recbase;
					}
#ifdef ENABLE_NOTICE
					if(recbase>dir_lastline) {
						entries = get_records(noticedirect, pnt, ssize, recbase-dir_lastline, screen_len);
					} else {
#endif
					entries = get_records(currdirect, pnt, ssize, recbase,
							screen_len);
#ifdef ENABLE_NOTICE
					if(entries<screen_len && digestmode==0 && notice_lastline) {
						entries+=get_records(noticedirect, pnt+ssize*entries, ssize, 1, screen_len-entries);
					}
				}
#endif
				}
				if (locmem->crs_line > last_line)
					locmem->crs_line = last_line;
				draw_entry(doentry, locmem, entries, ssize);
				PUTCURS
				;
				break;
			default:
				break;
		}
		mode = DONOTHING;
		if (entries == 0)
			break;
	}
	clear();
	free(pnt);
}

int
i_read_key(rcmdlist, locmem, ch, ssize)
struct one_key *rcmdlist;
struct keeploc *locmem;
int ch, ssize;
{
	int i, mode = DONOTHING,savemode;
	char ans[4];
	switch (ch) {
		case 'q':
		case 'e':
		case KEY_LEFT:
		//if ( digestmode )
		if( digestmode && uinfo.mode != RMAIL ) //chenhao
		return acction_mode();
		else
		return DOQUIT;
		case Ctrl('L'):
		redoscr();
		break;
		case 'M':
		savemode = uinfo.mode;
		in_mail=YEA;
		m_new();
		in_mail=NA;
		//m_read();
		modify_user_mode(savemode);
		return FULLUPDATE;
		case 'u':
		savemode = uinfo.mode;
		modify_user_mode(QUERY);
		t_query();
		modify_user_mode(savemode);
		return FULLUPDATE;
		case 'H':
		getdata(t_lines - 1, 0, "ÄúÑ¡Ôñ?(1) ±¾ÈÕÊ®´ó  (2) ÏµÍ³ÈÈµã [1]",ans, 2, DOECHO, YEA);
		if (ans[0] == '2')
		show_help("etc/hotspot");
		else
		show_help("0Announce/bbslist/day");
		return FULLUPDATE;
		case 'O':
		if (!strcmp("guest", currentuser.userid))
		break;
		//move(23, 0);
		//modified by iamfat 2003.11.20
		//if (askyn("ÄúÏëÌí¼ÓÍøÓÑµ½ºÃÓÑÃûµ¥Âð", NA, NA) == NA)
		//        return PARTUPDATE;
		{
			char *userid=
			((struct fileheader*)&pnt[(locmem->crs_line - locmem->top_line) * ssize])->owner;
			if(!strcmp(userid, currentuser.userid))
			break;
			move(t_lines-1, 0);
			sprintf(genbuf, "È·¶¨Òª°Ñ %s ¼ÓÈëºÃÓÑÃûµ¥Âð",userid);
			if (askyn(genbuf, NA, NA) == NA)
			return FULLUPDATE;
			if (addtooverride(userid) == -1) {
				sprintf(genbuf,"%s ÒÑÔÚÅóÓÑÃûµ¥", userid);
			} else {
				sprintf(genbuf, "%s ÁÐÈëÅóÓÑÃûµ¥", userid);
			}
			show_message(genbuf);
		}
		return FULLUPDATE;
		case 'k':
		case KEY_UP:
		if (cursor_pos(locmem, locmem->crs_line - 1, screen_len - 2))
		return PARTUPDATE;
		break;
		case 'j':
		case KEY_DOWN:
		if (cursor_pos(locmem, locmem->crs_line + 1, 0))
		return PARTUPDATE;
		break;
		case 'l': /* ppfoong */
		show_allmsgs();
		return FULLUPDATE;
		/*        case 'L':		//chenhao ½â¾öÔÚÎÄÕÂÁÐ±íÊ±¿´ÐÅµÄÎÊÌâ
		 if(uinfo.mode == RMAIL) return DONOTHING;
		 savemode = uinfo.mode;
		 m_read();
		 modify_user_mode(savemode);
		 return MODECHANGED;
		 */
		//wait for new key -> look all mail. 1.12. by money
		case 'N':
		case Ctrl('F'):
		case KEY_PGDN:
		case ' ':
		if (last_line >= locmem->top_line + screen_len) {
			locmem->top_line += screen_len - 1;
			locmem->crs_line = locmem->top_line;
			return PARTUPDATE;
		}
		RMVCURS;
		locmem->crs_line = last_line;
		PUTCURS;
		break;
		case '@':
		savemode = uinfo.mode;
		modify_user_mode(QUERY);
		show_online();
		modify_user_mode(savemode);
		return FULLUPDATE;
		case 'P':
		case Ctrl('B'):
		case KEY_PGUP:
		if (locmem->top_line> 1) {
			locmem->top_line -= screen_len - 1;
			if (locmem->top_line <= 0)
			locmem->top_line = 1;
			locmem->crs_line = locmem->top_line;
			return PARTUPDATE;
		} else {
			RMVCURS;
			locmem->crs_line = locmem->top_line;
			PUTCURS;
		}
		break;
		case KEY_HOME:
		locmem->top_line = 1;
		locmem->crs_line = 1;
		return PARTUPDATE;
		case '$':
		case KEY_END:
#ifdef ENABLE_NOTICE
		if(locmem->crs_line>dir_lastline) {
			if (dir_lastline >= locmem->top_line + screen_len) {
				locmem->top_line = dir_lastline - screen_len + 1;
				if (locmem->top_line <= 0)
				locmem->top_line = 1;
				locmem->crs_line = dir_lastline;
				return PARTUPDATE;
			}
			RMVCURS;
			locmem->crs_line = dir_lastline;
			PUTCURS;
		} else {
#endif
			if (last_line >= locmem->top_line + screen_len) {
				locmem->top_line = last_line - screen_len + 1;
				if (locmem->top_line <= 0)
				locmem->top_line = 1;
				locmem->crs_line = last_line;
				return PARTUPDATE;
			}
			RMVCURS;
			locmem->crs_line = last_line;
			PUTCURS;
#ifdef ENABLE_NOTICE
		}
#endif
		break;
		case 'S': /* youzi */
		if (!HAS_PERM(PERM_TALK))
		break;
		s_msg();
		return FULLUPDATE;
		break;
		/*case 'f':	modified by Seaman *//* youzi */
		case 'o' : /* added by Seaman */
		if (!HAS_PERM(PERM_LOGIN))
		break;
		t_friends();
		return FULLUPDATE;
		break;
		case '!': /* youzi leave */
		return Goodbye();
		break;
		case '\n':
		case '\r':
		case KEY_RIGHT:
		ch = 'r';
		/* lookup command table */
		default:
		for (i = 0; rcmdlist[i].fptr != NULL; i++) {
			if (rcmdlist[i].key == ch) {
				mode = (*(rcmdlist[i].fptr)) (locmem->crs_line,
						&pnt[(locmem->crs_line - locmem->top_line) * ssize],
						currdirect);
				break;
			}
		}
	}
	return mode;
}

int
auth_search_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, 1, fileinfo->owner))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
auth_search_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, -1, fileinfo->owner))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
post_search_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_post(locmem, 1))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
post_search_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_post(locmem, -1))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
show_author(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	t_query(fileinfo->owner);
	return FULLUPDATE;
}

int SR_BMfunc(int ent, struct fileheader *fileinfo, char *direct) {
	int i, dotype = 0, result = 0, gid = 0;
	int has_yinyan=0; //Add by everlove ÖÆ×÷ºÏ¼¯
	char buf[80], ch[32], BMch, annpath[512];
	char *buf1 = buf;

	//added by iamfat 2002.10.27 ¼ÓÈëÍ¬Ö÷Ìâ´óD¹¦ÄÜ
	int subflag;

	struct keeploc *locmem;
	char SR_BMitems[9][7] = { "É¾³ý", "±£Áô", "ÎÄÕª", "¾«»ªÇø", "Ë®ÎÄ", "²»¿ÉRE", "ºÏ¼¯",
		"»Ö¸´", "ºÏ²¢" };
	char subBMitems[3][9] = { "ÏàÍ¬Ö÷Ìâ", "ÏàÍ¬×÷Õß", "Ïà¹ØÖ÷Ìâ" };

	if (!in_mail) {
		if (uinfo.mode != READING)
			return DONOTHING;
		if (fileinfo->owner[0] == '-')
			return DONOTHING;
		if (!chkBM(currbp, &currentuser))
			return DONOTHING;
	}
	saveline(t_lines - 1, 0);
	move(t_lines - 1, 0);
	clrtoeol();
	ch[0] = '\0';
	getdata(t_lines - 1, 0, "Ö´ÐÐ: 1) ÏàÍ¬Ö÷Ìâ  2) ÏàÍ¬×÷Õß 3) Ïà¹ØÖ÷Ìâ 0) È¡Ïû [0]: ",
			ch, 3, DOECHO, YEA);
	dotype = atoi(ch);
	if (dotype < 1 || dotype > 3) { 
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	buf1 += sprintf(buf, "%s ", subBMitems[dotype - 1]);
	if (in_mail) {
		sprintf(buf1, "(%d)%s (%d)%s (%d)%s (%d)%s ",
				1, SR_BMitems[0],
				2, SR_BMitems[1],
				4, SR_BMitems[3],
				7, SR_BMitems[6]
			   );
	}
	else if (digestmode != TRASH_MODE && digestmode !=JUNK_MODE) {
		for (i = 0; i < 7; i++)
			buf1 += sprintf(buf1, "(%d)%s", i + 1, SR_BMitems[i]);
		sprintf(buf1, "(%d)%s", 8, SR_BMitems[8]);
	} else {
		for (i = 1; i < 8; i++)
			buf1 += sprintf(buf1, "(%d)%s ", i + 1, SR_BMitems[i]);
	}
	strcat(buf, "? [0]: ");
	getdata(t_lines - 1, 0, buf, ch, 3, DOECHO, YEA);
	BMch = atoi(ch);
	if(BMch<=0||BMch>8||(digestmode != 0 && BMch==3)
			||(digestmode>2 && BMch<3)) {
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	move(t_lines - 1, 0);
	sprintf(buf,"È·¶¨ÒªÖ´ÐÐ%s[%s]Âð",subBMitems[dotype-1],(BMch!=8)?SR_BMitems[BMch-1]:SR_BMitems[8]);
	if (askyn(buf, NA, NA) == 0) {
		saveline(t_lines - 1, 1);
		return PARTUPDATE;
	}

	if (digestmode != TRASH_MODE && digestmode !=JUNK_MODE && BMch == 8) {
		getdata(t_lines - 1, 0, "±¾Ö÷Ìâ¼ÓÖÁ°æÃæµÚ¼¸Æªºó£¿", ch, 8, DOECHO, YEA);
		if (ch[0] < '0' || ch[0]> '9' )
			return PARTUPDATE;
		result = atoi(ch);
		struct fileheader fh;
		get_record (direct, &fh, sizeof (fh), result);
		gid = fh.gid;
		fileinfo->reid = fh.id;
		substitute_record (direct, fileinfo, sizeof (*fileinfo), ent);
	}

	/* Add by everlove ÖÆ×÷ºÏ¼¯ */
	if(BMch == 7) {
		move(t_lines-1,0);
		if (askyn("ÖÆ×÷µÄºÏ¼¯ÐèÒªÒýÑÔÂð£¿", YEA, YEA) == YEA)
			has_yinyan=YEA;
		else
			has_yinyan=NA;
	}
	/*The End */

	if (dotype == 3) {
		strcpy(keyword, "");
		getdata(t_lines - 1, 0, "ÇëÊäÈëÖ÷Ìâ¹Ø¼ü×Ö: ", keyword, 50, DOECHO, YEA);
		if (keyword[0] == '\0') {
			saveline(t_lines - 1, 1);
			return DONOTHING;
		}
	} else if (dotype == 1) {
		strcpy(keyword, fileinfo->title);
	} else {
		strcpy(keyword, fileinfo->owner);
	}

	/* Add by everlove ÖÆ×÷ºÏ¼¯ */
	if( (dotype == 1 || dotype == 3) && (BMch == 7))
	{
		sprintf(buf, "tmp/%s.combine", currentuser.userid);
		if(dashf(buf)) unlink(buf);
	}
	/* The End */

	//added by iamfat 2002.10.30
	if(BMch==1) { //É¾³ý
		subflag=askyn("ÊÇ·ñÐ¡d", YEA, YEA);
	}

	move(t_lines - 1, 0);
	sprintf(buf, "ÊÇ·ñ´Ó%sµÚÒ»Æª¿ªÊ¼%s (Y)µÚÒ»Æª (N)Ä¿Ç°ÕâÒ»Æª",
			(dotype == 2) ? "¸Ã×÷Õß" : "´ËÖ÷Ìâ", SR_BMitems[BMch - 1]);
	if(askyn(buf, YEA, NA) == YEA) {
		result = locate_the_post(fileinfo, keyword,5,dotype-1,0);
	} else if(dotype == 3) {
		result = locate_the_post(fileinfo, keyword,1,2,0);
	} else {
		memcpy(&SR_fptr, fileinfo, sizeof(SR_fptr));
	}
	if( result == -1 ) {
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	if(BMch == 4) {
		if (DEFINE(DEF_MULTANNPATH)&& set_ann_path(NULL, NULL, ANNPATH_GETMODE)==0)
			return FULLUPDATE;
		else {
			FILE *fn;
			sethomefile(annpath, currentuser.userid,".announcepath");
			if((fn = fopen(annpath, "r")) == NULL ) {
				presskeyfor("¶Ô²»Æð, ÄúÃ»ÓÐÉè¶¨Ë¿Â·. ÇëÏÈÓÃ f Éè¶¨Ë¿Â·.",t_lines-1);
				saveline(t_lines - 1, 1);
				return DONOTHING;
			}
			fscanf(fn,"%s",annpath);
			fclose(fn);
			if (!dashd(annpath)) {
				presskeyfor("ÄúÉè¶¨µÄË¿Â·ÒÑ¶ªÊ§, ÇëÖØÐÂÓÃ f Éè¶¨.",t_lines-1);
				saveline(t_lines - 1, 1);
				return DONOTHING;
			}
		}
	}

	if (in_mail)
	{
		while (1)
		{
			locmem = getkeep(currdirect, 1, 1);
			switch(BMch) {
				case 1:
					if (SR_fptr.accessed[0] & FILE_MARKED)
						break;
					SR_BMDELFLAG = YEA;
					result = mail_del(locmem->crs_line, &SR_fptr, currdirect);
					SR_BMDELFLAG = NA;
					if(result == -1)
						return DIRCHANGED;
					if (result != DONOTHING) {
						last_line--;
						locmem->crs_line--;
					}
					break;
				case 2:
					mail_mark(locmem->crs_line, &SR_fptr, currdirect);
					break;
				case 4:
					a_Import("0Announce",currboard,NULL, &SR_fptr,annpath, YEA);
					break;
					/* Add by everlove ÖÆ×÷ºÏ¼¯ */
				case 7:
					Add_Combine(currboard,&SR_fptr,has_yinyan);
					break;
					/* The End */
			}
			if(locmem->crs_line <= 0) {
				result = locate_the_post(fileinfo, keyword,5,dotype-1,0);
			} else {
				result = locate_the_post(fileinfo, keyword,1,dotype-1,0);
			}
			if(result == -1) break;
		}
	} else {
		while(1) {
			locmem = getkeep(currdirect, 1, 1);
			switch(BMch) {
				case 1:
					SR_BMDELFLAG = YEA;
					result = _del_post(locmem->crs_line, &SR_fptr, currdirect, subflag, YEA);
					SR_BMDELFLAG = NA;
					if(result == -1)
						return DIRCHANGED;
					if (result != DONOTHING) {
						last_line--;
						locmem->crs_line--;
					}
					break;
				case 2:
					mark_post(locmem->crs_line, &SR_fptr, currdirect);
					break;
				case 3:
					digest_post(locmem->crs_line, &SR_fptr, currdirect);
					break;
				case 4:
					a_Import("0Announce",currboard,NULL, &SR_fptr,annpath, YEA);
					break;
				case 5:
					makeDELETEDflag(locmem->crs_line,&SR_fptr,currdirect);
					break;
				case 6:
					underline_post(locmem->crs_line,&SR_fptr,currdirect);
					break;
					/* Add by everlove ÖÆ×÷ºÏ¼¯ */
				case 7:
					Add_Combine(currboard,&SR_fptr,has_yinyan);
					break;
					/* The End */
				case 8:
					if (digestmode == TRASH_MODE || digestmode ==JUNK_MODE) {
						SR_BMDELFLAG = YEA;
						result= _UndeleteArticle(locmem->crs_line, &SR_fptr, currdirect,NA);
						SR_BMDELFLAG = NA;
						if(result == -1)
							return DIRCHANGED;
						if (result != DONOTHING) {
							last_line--;
							locmem->crs_line--;
						}
					} else {
						_combine_thread(locmem->crs_line, &SR_fptr, currdirect, gid);
					}

					break;
			}
			if(locmem->crs_line <= 0) {
				result = locate_the_post(fileinfo, keyword,5,dotype-1,0);
			} else {
				result = locate_the_post(fileinfo, keyword,1,dotype-1,0);
			}
			if(result == -1) break;
		}
	}
	/* Add by everlove ÖÆ×÷ºÏ¼¯ */
	if(/* (dotype == 1 || dotype == 3) &&*/BMch == 7) // add by jieer 2001.06.23
	{
		/*
		   if( !strncmp(keyword, "Re: ", 4)||!strncmp(keyword,"RE: ",4) )
		   sprintf(buf, "¡¾ºÏ¼¯¡¿%s", keyword + 4);
		   else
		   sprintf(buf, "¡¾ºÏ¼¯¡¿%s", keyword);
		   */
		//modified by iamfat 2002.08.17
		if( !strncmp(keyword, "Re: ", 4)||!strncmp(keyword,"RE: ",4) )
			sprintf(buf, "[ºÏ¼¯]%s", keyword + 4);
		else
			sprintf(buf, "[ºÏ¼¯]%s", keyword);
		//modified end
		ansi_filter(keyword, buf);
		sprintf(buf, "tmp/%s.combine", currentuser.userid);
		//modified by iamfat 2003.03.25
		if (in_mail)
			mail_file(buf, currentuser.userid, keyword);
		else
			Postfile(buf,currboard,keyword,2);
		unlink(buf);
	}
	/* The End */
	if (!in_mail && BMch != 7) {
		sprintf (buf, "%s°æ\"b\"%s%s", currboard, subBMitems[dotype
				- 1], SR_BMitems[BMch - 1]);
		securityreport(buf, 0, 2);
	}
	if (!in_mail)
		switch(BMch) {
			case 1:
				bm_log(currentuser.userid, currboard, BMLOG_RANGEDEL, 1);
				break;
			case 4:
				bm_log(currentuser.userid, currboard, BMLOG_RANGEANN, 1);
				break;
			case 7:
				bm_log(currentuser.userid, currboard, BMLOG_COMBINE, 1);
				break;
			default:
				bm_log(currentuser.userid, currboard, BMLOG_RANGEOTHER, 1);
				break;
		}
	return DIRCHANGED;
}

int BM_range(int ent, struct fileheader *fileinfo, char *direct) {
	struct fileheader fhdr;
	char annpath[512];
	char buf[STRLEN], ans[8], info[STRLEN], bname[STRLEN], dbname[STRLEN];

	char items[9][8] = { "±£Áô", "ÎÄÕª", "²»¿ÉRE", "É¾³ý", "¾«»ªÇø", "Ë®ÎÄ", "×ªÔØ",
			"É¾Ë®ÎÄ", "»Ö¸´" };
	int type, num1, num2, i, max = 9;
	int fdr, ssize;
	extern int SR_BMDELFLAG;
	extern char quote_file[120], quote_title[120], quote_board[120];

	if (uinfo.mode != READING)
		return DONOTHING;
	if (!chkBM(currbp, &currentuser))
		return DONOTHING;
	saveline(t_lines - 1, 0);
	if (digestmode != TRASH_MODE && digestmode != JUNK_MODE)
		max = 8;
	strcpy(info, "Çø¶Î:");
	for (i = 0; i < max; i++) {
		sprintf(buf, "%d)%s", i + 1, items[i]);
		strcat(info, buf);
	}
	strcat(info, "[0]:");
	getdata(t_lines-1, 0, info, ans, 2, DOECHO, YEA);
	type = atoi(ans);
	if (type <= 0 || type > max) {
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	if (((digestmode!=TRASH_MODE && digestmode!=JUNK_MODE)&&((digestmode
			&&(type==2||type==3||type==9)) || (digestmode>1 && digestmode
			!=TRASH_MODE && digestmode!=JUNK_MODE &&type!=5&&type!=6) ))
			|| ((digestmode==TRASH_MODE||digestmode==JUNK_MODE)&& type!=5
					&& type !=6 && type!=9)) { //modified by phrack :8->9
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	move(t_lines-1, 0);
	clrtoeol();
	prints("Çø¶Î²Ù×÷: %s", items[type-1]);
	getdata(t_lines-1, 20, "Ê×ÆªÎÄÕÂ±àºÅ: ", ans, 6, DOECHO, YEA);
	num1=atoi(ans);
	if (num1>0) {
		getdata(t_lines-1, 40, "Ä©Æ¬ÎÄÕÂ±àºÅ: ", ans, 6, DOECHO, YEA);
		num2=atoi(ans);
	}
	if (num1<=0||num2<=0||num2<=num1) {
		move(t_lines-1, 60);
		prints("²Ù×÷´íÎó...");
		egetch();
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	if (type != 7) {
		sprintf(info, "Çø¶Î [%s] ²Ù×÷·¶Î§ [ %d -- %d ]£¬È·¶¨Âð", items[type-1],
				num1, num2);
		if (askyn(info, NA, YEA)==NA) {
			saveline(t_lines - 1, 1);
			return DONOTHING;
		}
	}
	if (type == 5) {
		if (DEFINE(DEF_MULTANNPATH)&& set_ann_path(NULL, NULL,
				ANNPATH_GETMODE)==0)
			return FULLUPDATE;
		else {
			FILE *fn;
			sethomefile(annpath, currentuser.userid, ".announcepath");
			if ((fn = fopen(annpath, "r")) == NULL) {
				presskeyfor("¶Ô²»Æð, ÄúÃ»ÓÐÉè¶¨Ë¿Â·. ÇëÏÈÓÃ f Éè¶¨Ë¿Â·.", t_lines-1);
				saveline(t_lines - 1, 1);
				return DONOTHING;
			}
			fscanf(fn, "%s", annpath);
			fclose(fn);
			if (!dashd(annpath)) {
				presskeyfor("ÄúÉè¶¨µÄË¿Â·ÒÑ¶ªÊ§, ÇëÖØÐÂÓÃ f Éè¶¨.", t_lines-1);
				saveline(t_lines - 1, 1);
				return DONOTHING;
			}
		}
	} else if (type == 7) {
		clear();
		prints("\n\nÄú½«½øÐÐÇø¶Î×ªÔØ¡£×ªÔØ·¶Î§ÊÇ£º[%d -- %d]\n", num1, num2);
		prints("µ±Ç°°æÃæÊÇ£º[ %s ] \n", currboard);
		if (!get_a_boardname(bname, "ÇëÊäÈëÒª×ªÌùµÄÌÖÂÛÇøÃû³Æ: "))
			return FULLUPDATE;
		if (!strcmp(bname, currboard)&&uinfo.mode != RMAIL) {
			prints("\n\n¶Ô²»Æð£¬±¾ÎÄ¾ÍÔÚÄúÒª×ªÔØµÄ°æÃæÉÏ£¬ËùÒÔÎÞÐè×ªÔØ¡£\n");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (askyn("È·¶¨Òª×ªÔØÂð", NA, NA)==NA)
			return FULLUPDATE;
	}
	if ((fdr = open(direct, O_RDONLY, 0)) == -1) {
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	ssize = sizeof(struct fileheader);
	if (lseek(fdr, (num1-1)*ssize, SEEK_SET)==-1) {
		close(fdr);
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	sprintf(info, "%s°æ\"L\"Çø¶Î%s£¬·¶Î§[%d-%d]", currboard, items[type-1],
			num1, num2);
	securityreport(info, 0, 2);

	while (read(fdr, &fhdr, ssize) == ssize) {
		if (num1 > num2)
			break;
		switch (type) {
			case 1:
				mark_post(num1, &fhdr, direct);
				break;
			case 2:
				digest_post(num1, &fhdr, direct);
				break;
			case 3:
				underline_post(num1, &fhdr, direct);
				break;
			case 4: {
				int result;
				SR_BMDELFLAG = YEA;
				result = del_post(num1, &fhdr, direct);
				SR_BMDELFLAG = NA;
				if (result == -1) {
					close(fdr);
					return DIRCHANGED;
				}
				if (result != DONOTHING) {
					lseek(fdr, (-1)*ssize, SEEK_CUR);
					num1--;
					num2--;
				}
				break;
			}
			case 5:
				a_Import("0Announce", currboard, NULL, &fhdr, annpath, YEA);
				//a_Import( "0Announce", currboard, locmem->crs_line, &fhdr, currdirect, YEA);
				break;
			case 6:
				makeDELETEDflag(num1, &fhdr, direct);
				break;
			case 7:
				if (uinfo.mode != RMAIL)
					sprintf(genbuf, "boards/%s/%s", currboard,
							fhdr.filename);
				else
					sprintf(genbuf, "mail/%c/%s/%s",
							toupper(currentuser.userid[0]),
							currentuser.userid, fhdr.filename);
				strlcpy(quote_file, genbuf, sizeof(quote_file));
				strcpy(quote_title, fhdr.title);
				strcpy(quote_board, currboard);
				strcpy(dbname, currboard);
				strcpy(currboard, bname);
				post_cross('l', 0);
				strcpy(currboard, dbname);
				break;

				/**
				 * Added by phrack to support deletion of water posts on 2007.12.12
				 * */
			case 8: {
				/*delete the posts that have the deleted flag*/
				int result;
				if (fhdr.accessed[0] & FILE_DELETED) {
					SR_BMDELFLAG = YEA;
					result = del_post(num1, &fhdr, direct);
					SR_BMDELFLAG = NA;
					if (result == -1) {
						close(fdr);
						return DIRCHANGED;
					}
					if (result != DONOTHING) {
						lseek(fdr, (-1)*ssize, SEEK_CUR);
						num1--;
						num2--;
					}
				}
			}
				break;
			case 9: {
				int result;
				SR_BMDELFLAG = YEA;
				result=_UndeleteArticle(num1, &fhdr, direct, NA);
				SR_BMDELFLAG = NA;
				if (result == -1) {
					close(fdr);
					return DIRCHANGED;
				}
				if ((digestmode==TRASH_MODE||digestmode==JUNK_MODE)
						&& result != DONOTHING) {
					lseek(fdr, (-1)*ssize, SEEK_CUR);
					num1 --;
					num2--;
				}
			}
				break;
		}
		num1 ++;
	}
	close(fdr);
	saveline(t_lines - 1, 1);
	if (!in_mail)
		switch (type) {
			case 4:
				bm_log(currentuser.userid, currboard, BMLOG_RANGEDEL, 1);
				break;
			case 5:
				bm_log(currentuser.userid, currboard, BMLOG_RANGEANN, 1);
				break;
			default:
				bm_log(currentuser.userid, currboard, BMLOG_RANGEOTHER, 1);
				break;
		}

	return DIRCHANGED;
}

/* Add by shun 2000.3.16; Change by money 2002.1.12 */
int
BM_range2(ent,fileinfo,direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char num1[11],num2[11];
	int inum1, inum2;
	int i;
	char buf[STRLEN],ch[4],BMch;
	char *buf1 = buf;
	char *SR_BMitems[]= {"É¾³ý","±£Áô","¾«»ªÇø","ÔÝ´æµµ","¾«¼òÔÝ´æ","²»¿ÉRE"};//added by roly 02.03.27
	struct keeploc *locmem;
	struct fileheader fhdr;
	int fdr,cou;

	if(!chkBM(currbp, &currentuser))
		return DONOTHING;

	if(digestmode==2) return DONOTHING;
	saveline(t_lines-2, 0);
	move(t_lines-2, 0);
	clrtoeol();

	getdata(t_lines-2,0,"Çø¶Î¹¦ÄÜ:Ê×ÆªÎÄÕÂ±àºÅ: ",num1,10,DOECHO,YEA);
	inum1 = atoi(num1);
	if(inum1 <= 0) {
		saveline(t_lines-2, 1);
		return DONOTHING;
	}
	if (inum1>=get_num_records(direct,sizeof(fhdr))) {
		saveline(t_lines-2, 1);
		return DONOTHING;
	}
	getdata(t_lines-2,0,"Çø¶Î¹¦ÄÜ:Ä©ÆªÎÄÕÂ±àºÅ: ",num2,10,DOECHO,YEA);
	inum2 = atoi(num2);
	if(inum2 <= inum1) {
		saveline(t_lines-2, 1);
		return DONOTHING;
	}
	if (inum2>get_num_records(direct,sizeof(fhdr))) {
		saveline(t_lines-2, 1);
		return DONOTHING;
	}
	sprintf(buf,"Çø¶Î %d-%d : 0)È¡Ïû ",inum1,inum2);
	for (i = 0; i < 6; i++)
		buf1 += sprintf(buf1, "%d)%s ", i + 1, SR_BMitems[i]);
	strcat(buf,"? [0]: ");
	getdata(t_lines-2, 0,buf,ch,3,DOECHO,NULL,YEA);
	BMch=atoi(ch);
	if(BMch<=0||BMch>6) {//modified by roly from 5 to 6 02.03.27
		saveline(t_lines-2, 1);
		return DONOTHING;
	}
	locmem = getkeep( currdirect, 1, 1);
	locmem->crs_line=inum1;

	cou=inum1;
	if ((fdr = open(direct,O_RDONLY,0)) == -1) return DONOTHING;
	else {
		if(lseek(fdr,(sizeof(fhdr))*(inum1-1),SEEK_SET) == -1) {
			close(fdr);
			return DONOTHING;
		}
		if (BMch == 3 && DEFINE(DEF_MULTANNPATH) && set_ann_path(NULL, NULL, ANNPATH_GETMODE) == 0)
		return FULLUPDATE;

		while(read(fdr,&fhdr,sizeof(fhdr)) == sizeof(fhdr)) {
			if (cou<=inum2) {
				switch (BMch) {
					case 1:
					if (fhdr.accessed[0] & FILE_MARKED) break;
					SR_BMDELFLAG=YEA;
					del_post(locmem->crs_line,&fhdr,currdirect);
					SR_BMDELFLAG=NA;
					if(sysconf_eval("KEEP_DELETED_HEADER")<=0) locmem->crs_line--;
					lseek(fdr,(sizeof(fhdr))*(-1),SEEK_CUR);
					break;
					case 2:
					mark_post(locmem->crs_line, &fhdr,currdirect);
					break;
					case 3:
					a_Import( "0Announce", currboard, locmem->crs_line, &fhdr, currdirect, YEA);
					break;
					case 4:
					a_Save("0Announce", currboard, &fhdr ,YEA);
					break;
					case 5:
					quote_save("0Announce", currboard, &fhdr ,YEA);
					break;
					case 6: //added by roly 02.03.27
					{
						int result,ssize;
						extern int SR_BMDELFLAG;
						ssize = sizeof(struct fileheader);
						SR_BMDELFLAG = YEA;
						result = del_post(num1, &fhdr, direct);
						SR_BMDELFLAG = NA;
						if(result == -1) {
							close(fdr);
							return DIRCHANGED;
						}
						if (result != DONOTHING) {
							lseek(fdr, (-1)*ssize, SEEK_CUR);
							inum1 --;
							inum2--;
						}
					}
					break;
				}
				locmem->crs_line++;
			} else break;
			cou++;
		}
	}
	return DIRCHANGED;
}
/* End */

int combine_thread(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[16];
	int num;
	struct fileheader fh;
	getdata(t_lines - 1, 0, "ºÏ²¢µ½°æÃæµÚ¼¸Æªºó£¿",buf, 6, DOECHO, YEA);
	if (buf[0] < '0'|| buf[0]> '9' )
	return PARTUPDATE;
	num = atoi(buf);
	get_record (direct, &fh, sizeof (fh), num);
	fileinfo->gid = fh.gid;
	fileinfo->reid = fh.id;
	substitute_record (direct, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

int _combine_thread(ent, fileinfo, direct, gid)
int ent;
struct fileheader *fileinfo;
char *direct;
int gid;
{
	fileinfo->gid = gid;
	substitute_record (direct, fileinfo, sizeof (*fileinfo), ent);
}

int SR_first_new(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if(locate_the_post(fileinfo, fileinfo->title,5,0,1)!=-1) {
		sread(1, 0, &SR_fptr);
		return FULLUPDATE;
	}
	return PARTUPDATE;
}

int
SR_last(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	locate_the_post(fileinfo, fileinfo->title,3,0,0);
	return PARTUPDATE;
}

int
SR_first(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	locate_the_post(fileinfo, fileinfo->title,5,0,0);
	return PARTUPDATE;
}

int
SR_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	sread(1, 0, fileinfo);
	return FULLUPDATE;
}

int
SR_author(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	sread(1, 1, fileinfo);
	return FULLUPDATE;
}

int
search_author(locmem, offset, powner)
struct keeploc *locmem;
int offset;
char *powner;
{
	static char author[IDLEN + 1];
	char ans[IDLEN + 1], pmt[STRLEN];
	char currauth[STRLEN];
	strcpy(currauth, powner);

	sprintf(pmt, "%sµÄÎÄÕÂËÑÑ°×÷Õß [%s]: ", offset> 0 ? "ÍùááÀ´" : "ÍùÏÈÇ°", currauth);
	move(t_lines - 1, 0);
	clrtoeol();
	//Modified by IAMFAT 2002-05-27
	//IDLEN->IDLEN+1
	getdata(t_lines - 1, 0, pmt, ans, IDLEN+1, DOECHO, YEA);
	if (ans[0] != '\0')
	strcpy(author, ans);
	else
	strcpy(author, currauth);

	return search_articles(locmem, author, 0, offset, 1,0);
}

int
auth_post_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, 1, fileinfo->owner))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
auth_post_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_author(locmem, -1, fileinfo->owner))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
t_search_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_title(locmem, 1))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}

int
t_search_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_title(locmem, -1))
	return PARTUPDATE;
	else
	update_endline();
	return DONOTHING;
}
int
thread_up(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_thread(locmem, -1, fileinfo)) {
		update_endline();
		return PARTUPDATE;
	}
	update_endline();
	return DONOTHING;
}

int
thread_down(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct keeploc *locmem;
	locmem = getkeep(direct, 1, 1);
	if (search_thread(locmem, 1, fileinfo)) {
		update_endline();
		return PARTUPDATE;
	}
	update_endline();
	return DONOTHING;
}

int
search_post(locmem, offset)
struct keeploc *locmem;
int offset;
{
	static char query[STRLEN];
	char ans[STRLEN], pmt[STRLEN];
	strcpy(ans, query);
	sprintf(pmt, "ËÑÑ°%sµÄÎÄÕÂ [%s]: ", offset> 0 ? "ÍùááÀ´" : "ÍùÏÈÇ°", ans);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, 50, DOECHO, YEA);
	if (ans[0] != '\0')
	strcpy(query, ans);

	return search_articles(locmem, query, 0, offset, -1, 0);
}

int
search_title(locmem, offset)
struct keeploc *locmem;
int offset;
{
	static char title[STRLEN];
	char ans[STRLEN], pmt[STRLEN];
	strcpy(ans, title);
	sprintf(pmt, "%sËÑÑ°±êÌâ [%.16s]: ", offset> 0 ? "Íùáá" : "ÍùÇ°", ans);
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, pmt, ans, 46, DOECHO, YEA);
	if (*ans != '\0')
	strcpy(title, ans);
	return search_articles(locmem, title, 0, offset, 2, 0);
}

int
search_thread(locmem, offset, fh)
struct keeploc *locmem;
int offset;
struct fileheader *fh;
{
	char *title = fh->title;

	if (title[0] == 'R' && (title[1] == 'e' || title[1] == 'E') && title[2] == ':')
	title += 4;
	setqtitle(title, fh->gid);
	return search_articles(locmem, title, fh->gid, offset, 0, 0);
}

int sread(int readfirst, int auser, struct fileheader *ptitle) {
	struct keeploc *locmem;
	int rem_top, rem_crs, ch;
	int isend = 0, isstart = 0, isnext = 1;
	char tempbuf[STRLEN], title[STRLEN];

	if (readfirst) {
		isstart = 1;
	} else {
		isstart = 0;
		move(t_lines - 1, 0);
		clrtoeol();
		prints(
				"[1;44;31m[%8s] [33mÏÂÒ»·â <Space>,<Enter>,¡ý©¦ÉÏÒ»·â ¡ü,U                              [m",
				auser ? "ÏàÍ¬×÷Õß" : "Ö÷ÌâÔÄ¶Á");
		switch (egetch()) {
			case ' ':
			case '\n':
			case KEY_DOWN:
			case KEY_RIGHT:
				isnext = 1;
				break;
			case KEY_UP:
			case 'u':
			case 'U':
				isnext = -1;
				break;
			default:
				isend = 1;
				break;
		}
	}
	locmem = getkeep(currdirect, 1, 1);
	rem_top = locmem->top_line;
	rem_crs = locmem->crs_line;
	if (auser == 0) {
		strcpy(title, ptitle->title);
		setqtitle(title, ptitle->gid);
	} else {
		strcpy(title, ptitle->owner);
		setqtitle(ptitle->title, ptitle->gid);
	}
	if (!strncmp(title, "Re: ", 4) | !strncmp(title, "RE: ", 4)) {
		strcpy(title, title + 4);
	}
	memcpy(&SR_fptr, ptitle, sizeof(SR_fptr));
	while (!isend) {
		if (!isstart) {
			if (search_articles(locmem, title, ptitle->gid, isnext, auser,
					0)==-1)
				break;
		}
		if (uinfo.mode != RMAIL)
			setbfile(tempbuf, currboard, SR_fptr.filename);
		else
			sprintf(tempbuf, "mail/%c/%s/%s",
					toupper(currentuser.userid[0]), currentuser.userid,
					SR_fptr.filename);
		setquotefile(tempbuf);
		ch = ansimore(tempbuf, NA);
		brc_addlist(SR_fptr.filename);
		isstart = 0;
		if (ch == KEY_UP || ch == 'u' || ch == 'U') {
			readfirst = (ch == KEY_UP);
			isnext = -1;
			continue;
		}
		move(t_lines - 1, 0);
		clrtoeol();
		prints(
				"\033[1;44;31m[%8s] \033[33m»ØÐÅ R ©¦ ½áÊø Q,¡û ©¦ÏÂÒ»·â ¡ý,Enter©¦ÉÏÒ»·â ¡ü,U ©¦ ^R »Ø¸ø×÷Õß   \033[m",
				auser ? "ÏàÍ¬×÷Õß" : "Ö÷ÌâÔÄ¶Á");
		switch (ch = egetch()) {
			case KEY_LEFT:
			case 'N':
			case 'Q':
			case 'n':
			case 'q':
				isend = 1;
				break;
			case 'Y':
			case 'R':
			case 'y':
			case 'r': {
				struct boardheader *bp;
				extern struct boardheader *getbcache();

				bp = getbcache(currboard);
				if (!get_records(currdirect, ptitle, sizeof(*ptitle),
						rem_crs, 1))
					return DONOTHING;
				noreply=ptitle->accessed[0]&FILE_NOREPLY||bp->flag
						& BOARD_NOREPLY_FLAG;
				if (!noreply || chkBM(currbp, &currentuser)) {
					local_article=!(ptitle->filename[STRLEN-1]=='S');
					do_reply(ptitle);
				} else {
					clear();
					move(5, 6);
					prints("¶Ô²»Æð, ¸ÃÎÄÕÂÓÐ²»¿É RE ÊôÐÔ, Äú²»ÄÜ»Ø¸´(RE) ÕâÆªÎÄÕÂ.");
					pressreturn();
				}
				break;
			}
			case ' ':
			case '\n':
			case KEY_DOWN:
			case KEY_RIGHT:
				readfirst = (ch == KEY_DOWN);
				isnext = 1;
				break;
			case Ctrl('A'):
				clear();
				show_author(0, &SR_fptr, currdirect);
				isnext = 1;
				break;
			case KEY_UP:
			case 'u':
			case 'U':
				readfirst = (ch == KEY_UP);
				isnext = -1;
				break;
			case Ctrl('R'):
				post_reply(0, &SR_fptr, (char *) NULL);
				break;
			case 'g':
				digest_post(0, &SR_fptr, currdirect);
				break;
			default:
				break;
		}
	}
	if (readfirst == 0) {
		RMVCURS
		;
		locmem->top_line = rem_top;
		locmem->crs_line = rem_crs;
		PUTCURS
		;
	}
	return 1;
}

//Added by IAMFAT 2002-05-27
int strcasecmp2(char *s1, char *s2) {
	register int c1, c2;
	while (*s1 && *s2) {
		c1 = tolower(*s1);
		c2 = tolower(*s2);
		if (c1 != c2)
			return (c1 - c2);
		s1++;
		s2++;
	}
	if (!*s1 && !*s2)
		return 0;
	else if (*s1)
		return -1;
	else
		return 1;
}
//End IAMFAT

int
searchpattern(filename, query)
char *filename;
char *query;
{
	FILE *fp;
	char buf[256];
	if ((fp = fopen(filename, "r")) == NULL)
	return 0;
	while (fgets(buf, 256, fp) != NULL) {
		//Modified by IAMFAT 2002-05-25
		if (strcasestr_gbk(buf, query)) {
			fclose(fp);
			return YEA;
		}
	}
	fclose(fp);
	return NA;
}

enum {
	SEARCH_BACKWARD = -1,
	SEARCH_FORWARD = 1,
	SEARCH_FIRST = 3,
	SEARCH_LAST = 5,

	SEARCH_CONTENT = -1,
	SEARCH_THREAD = 0,
	SEARCH_AUTHOR = 1,
	SEARCH_RELATED = 2,
};

int search_articles(struct keeploc *locmem, const char *query, int gid,
		int offset, int aflag, int newflag)
{
	int complete, ent, oldent, lastent = 0;
	char *ptr;

	if (*query == '\0')
		return 0;
	if (aflag == SEARCH_RELATED) {
		complete = 0;
		aflag = SEARCH_THREAD;
	} else {
		complete = 1;
	}
	if ((offset == SEARCH_FIRST || offset == SEARCH_LAST)
			&& aflag != SEARCH_CONTENT) {
		ent = 0;
		oldent = 0;
		offset = -4; //?
	} else {
		ent = locmem->crs_line;
		oldent = locmem->crs_line;
	}
	if (aflag != SEARCH_CONTENT && offset < 0)
		ent = 0;

	if (aflag == SEARCH_CONTENT) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("\033[1;44;33mËÑÑ°ÖÐ£¬ÇëÉÔºò....                      "
				"                                       \033[m");
		refresh();
	}

	FILE *fp = fopen(currdirect, "rb");
	if (fp == NULL)
		return -1;
	
	if (ent) {
		if (aflag == SEARCH_CONTENT && offset < 0)
			ent -= 2;
		if (ent < 0 || fseek(fp, ent * sizeof(struct fileheader), SEEK_SET) < 0) {
			fclose(fp);
			return -1;
		}
	}
	if (aflag != SEARCH_CONTENT && offset > 0)
		ent = oldent;
	if (aflag == SEARCH_CONTENT && offset < 0)
		ent += 2;
	while (fread(&SR_fptr, sizeof(SR_fptr), 1, fp) == 1) {
		if (aflag == SEARCH_CONTENT && offset < 0)
			ent--;
		else
			ent++;
		if (aflag != SEARCH_CONTENT && offset < 0 && oldent > 0
				&& ent >= oldent)
			break;
		if (newflag && !brc_unread(SR_fptr.filename))
			continue;

		if (aflag == SEARCH_CONTENT) {
			char p_name[256];
			if (uinfo.mode != RMAIL) {
				setbfile(p_name, currboard, SR_fptr.filename);
			} else {
				sprintf(p_name, "mail/%c/%s/%s",
						toupper(currentuser.userid[0]),
						currentuser.userid, SR_fptr.filename);
			}
			if (searchpattern(p_name, query)) {
				lastent = ent;
				break;
			} else if (offset > 0) {
				continue;
			} else {
				if (fseek(fp, -2 * sizeof(SR_fptr), SEEK_CUR) < 0) {
					fclose(fp);
					return -1;
				}
				continue;
			}
		}

		ptr = (aflag == SEARCH_AUTHOR) ? SR_fptr.owner : SR_fptr.title;
		if (complete) {
			if (aflag == SEARCH_AUTHOR) {
				if (!strcasecmp(ptr, query)) {
					lastent = ent;
					if (offset > 0)
						break;
				}
			} else { // SEARCH_THREAD
				if (in_mail) {
					if (!strncasecmp(ptr, "Re: ", 4))
						ptr += 4;
					if (!strcasecmp2(ptr, query)) {
						lastent = ent;
						if (offset > 0)
							break;
					}
				} else {
					if (SR_fptr.gid == gid) {
						lastent = ent;
						if (offset > 0)
							break;
					}
				}
			}
		} else {// SEARCH_RELATED
			if (strcasestr_gbk(ptr, query) != NULL) {
				if (aflag) {
					if (strcasecmp(ptr, query))
						continue;
				}
				lastent = ent;
				if (offset > 0)
					break;
			}
		}
	}
	move(t_lines - 1, 0);
	clrtoeol();
	fclose(fp);
	if (lastent == 0)
		return -1;
	get_record(currdirect, &SR_fptr, sizeof(SR_fptr), lastent);
	last_line = get_num_records(currdirect, sizeof(SR_fptr));
	return (cursor_pos(locmem, lastent, 10));
}

int locate_the_post(struct fileheader *fileinfo, char *query, int offset, //-1 µ±Ç°ÏòÉÏ  1 µ±Ç°ÏòÏÂ  3 ×îºóÒ»Æª 5 µÚÒ»Æª
		int aflag, // 1 owner  0 Í¬Ö÷Ìâ   2 Ïà¹ØÖ÷Ìâ
		int newflag // 1 ±ØÐëÎªÐÂÎÄÕÂ   0 ÐÂ¾É¾ù¿É
) {
	struct keeploc *locmem;
	locmem = getkeep(currdirect, 1, 1);
	if (query[0]=='R'&&(query[1]=='e'||query[1]=='E')&&query[2]==':')
		query += 4;
	setqtitle(query, fileinfo->gid);
	return search_articles(locmem, query, fileinfo->gid, offset, aflag,
			newflag);
}
/* calc cursor pos and show cursor correctly -cuteyu */
//½«¹â±êÒÆµ½ºÏÊÊµÄÎ»ÖÃ,²¢ÏÔÊ¾
//	¹â±êµÄ¸ü¸Ä·´Ó³ÔÚlocmemµÄÊý¾ÝÖÐ
int cursor_pos(struct keeploc *locmem, int val, int from_top) {
	if (val > last_line) {
		val = DEFINE(DEF_CIRCLE) ? 1 : last_line;
	}
	if (val <= 0) {
		val = DEFINE(DEF_CIRCLE) ? last_line : 1;
	}
	if (val >= locmem->top_line && val < locmem->top_line + screen_len - 1) {
		RMVCURS
		;
		locmem->crs_line = val;
		PUTCURS
		;
		return 0;
	}
	locmem->top_line = val - from_top;
	if (locmem->top_line <= 0)
		locmem->top_line = 1;
	locmem->crs_line = val;
	return 1;
}

int r_searchall() {
	char id[20], patten[30], buf[5];
	char ans[5];
	int dt;
	int junk;
	int all;
	int flag;
	all = NA;
	modify_user_mode(QUERY);
	clear();
	usercomplete("ÇëÊäÈëÄúÏë²éÑ¯µÄ×÷ÕßÕÊºÅ: ", id);
	if (id[0] == 0) {
		getdata(0, 30, "²éÑ¯ËùÓÐµÄ×÷ÕßÂð?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y')) {
			return;
		} else
			all = YEA;
	} else if (!getuser(id)) {
		prints("²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ\n");
		pressreturn();
		return;
	}
	getdata(1, 0, "ÇëÊäÈëÎÄÕÂ±êÌâ¹Ø¼ü×Ö: ", patten, 29, DOECHO, YEA);
	getdata(2, 0, "²éÑ¯¾à½ñ¶àÉÙÌìÒÔÄÚµÄÎÄÕÂ?: ", buf, 4, DOECHO, YEA);
	dt = atoi(buf);
	if (dt == 0)
		return;
	getdata(3, 0, "ËÑË÷ 0)È¡Ïû 1) °æÃæ 2)°æÖ÷À¬»øÏä 3)Õ¾ÎñÀ¬»øÏä[0]:", buf, 4, DOECHO, YEA);
	junk = atoi(buf);
	if (junk < 1 || junk > 3)return ;
	getdata(4,0,"ÊÇ·ñÉ¾³ýÎÄÕÂ?[Y/N]",ans,2,DOECHO,YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
	flag=NA;
	else
	flag = YEA;
	searchallboard (id, patten, dt, all, junk,flag);
	report ("ÍøÓÑ´ó×÷²éÑ¯", currentuser.userid);
}

int searchallboard(char *id, char *patten, int dt, int all, int del,
		int flag) {
	FILE *fp, *fp2, *fp3;
	char f[100], buf2[150];
	char fname[STRLEN];
	char INDEX[20];
	struct fileheader xx;
	struct boardheader xx2;
	int counts = 0, n2 = 0, n3, now;
	long t0;

	if (del == 1)
		strcpy(INDEX, ".DIR");
	else if (del == 2)
		strcpy(INDEX, ".TRASH");
	else if (del == 3)
		strcpy(INDEX, ".JUNK");
	now = time(0);
	fp2 = fopen(".BOARDS", "r");
	sprintf(fname, "tmp/searchall.%s.%05d", currentuser.userid, uinfo.pid);
	fp3 = fopen(fname, "w");
	fprintf(fp3, "ÔÚËùÓÐ°å²éÑ¯%sÍøÓÑ%dÌìÒÔÄÚµÄ´ó×÷, ¹Ø¼ü×Ö'%s'.\n\n", id, dt, patten);
	dt = dt * 86400;
	while (!feof(fp2)) {
		fread(&xx2, sizeof (xx2), 1, fp2);
		if (xx2.flag & BOARD_POST_FLAG || HAS_PERM(xx2.level) || (xx2.flag
				& BOARD_NOZAP_FLAG)) {
			int n = 0;

			sprintf(f, "boards/%s/%s", xx2.filename, INDEX);

			if ((fp = fopen(f, "r")) != NULL) {
				n2 = 0;
				while (fread(&xx, sizeof (xx), 1, fp) > 0) {
					n++;
					t0 = atoi(xx.filename + 2);
					if ((all == YEA || !strcasecmp(xx.owner, id))
							&& strstr(xx.title, patten) && (abs(now - t0)
							< dt)) {
						counts++;
						n2++;
						sprintf(buf2, " %4d  %24s  %s\n", n, ctime(&t0),
								xx.title);
						for (n3 = 0; n3 < 90; n3++)
							if (buf2[n3] == 10)
								buf2[n3] = 32;
						fprintf(fp3, "%s\n", buf2);
						if (flag==YEA) {
							SR_BMDELFLAG=YEA;
							del_post(n, &xx, f);
							SR_BMDELFLAG=NA;
						}
					}
				}
				fclose(fp);
				if (n2 != 0)
					fprintf(fp3, "Above %d is found in board %s.\n\n", n2,
							xx2.filename);
			}
		}
		if (counts >= 1000)
			break;
	}
	sprintf(buf2, "[%s]²éÑ¯%sÔÚ%dÌìÄÚ¹Ø¼ü×Ö'%0.10s'", del==1 ? "°æÃæ"
			: (del==2 ? "Trash" : "Junk"), id, dt/86400, patten);
	fprintf(fp3, "%d matched found.\n", counts);
	fclose(fp2);
	fclose(fp3);
	mail_file(fname, currentuser.userid, buf2);
	unlink(fname);
	return 0;
}

