#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

//将光标移到当前的位置,并显示成>
#define PUTCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(">");
//清除以前的光标标记,即把>改成空格
#define RMVCURS   move(3+locmem->crs_line-locmem->top_line,0);prints(" ");

struct fileheader SR_fptr;
int SR_BMDELFLAG = NA;
char *pnt;

extern int noreply;

struct keeploc {
	char *key;
	int top_line;
	int crs_line;
	struct keeploc *next;
};

/*struct fileheader *files = NULL;*/
char currdirect[STRLEN];
char keyword[STRLEN]; /* for 相关主题 */
int screen_len;
int last_line;

#ifdef ENABLE_NOTICE
char noticedirect[256];
int notice_lastline, dir_lastline;
#endif

static int search_articles(struct keeploc *locmem, const char *query, int gid,
		int offset, int aflag, int newflag);

//在局部表态链表中查找与关键字符串s相匹配的项
//		找到:	直接返回
//		否则:	新加一项,放在链表表头,并返回新加项
struct keeploc * getkeep(char *s, int def_topline, int def_cursline)
{
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

void fixkeep(char *s, int first, int last)
{
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
		if (session.status != ST_RMAIL)
			prints("%s", str);
		else
			showstuff(str);
		prints("\n");
	}
	move(-1, 0);
	clrtoeol();
	update_endline();
}

void draw_bottom(char *buf) {
	char buf1[100];
	if (buf) {
		move(-1, 71);
		clrtoeol();
		sprintf(buf1, "\033[0;1;44;33m[%6.6s]\033[m", buf);
		outs(buf1);
	}
}

#ifdef ENABLE_NOTICE
void get_noticedirect(char *curr, char *notice)
{
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

/* calc cursor pos and show cursor correctly -cuteyu */
//将光标移到合适的位置,并显示
//	光标的更改反映在locmem的数据中
static int cursor_pos(struct keeploc *locmem, int val, int from_top)
{
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

static int i_read_key(struct one_key *rcmdlist, struct keeploc *locmem, int ch, int ssize)
{
	int i, mode = DONOTHING,savemode;
	char ans[4];
	switch (ch) {
		case 'q':
		case 'e':
		case KEY_LEFT:
		if( digestmode && session.status != ST_RMAIL)
			;
		else
		return DOQUIT;
		case Ctrl('L'):
		redoscr();
		break;
		case 'M':
		savemode = session.status;
		in_mail=YEA;
		m_new();
		in_mail=NA;
		//m_read();
		set_user_status(savemode);
		return FULLUPDATE;
		case 'u':
		savemode = session.status;
		set_user_status(ST_QUERY);
		t_query(NULL);
		set_user_status(savemode);
		return FULLUPDATE;
		case 'H':
		//% getdata(-1, 0, "您选择?(1) 本日十大  (2) 系统热点 [1]",ans, 2, DOECHO, YEA);
		getdata(-1, 0, "\xc4\xfa\xd1\xa1\xd4\xf1?(1) \xb1\xbe\xc8\xd5\xca\xae\xb4\xf3  (2) \xcf\xb5\xcd\xb3\xc8\xc8\xb5\xe3 [1]",ans, 2, DOECHO, YEA);
		if (ans[0] == '2')
		show_help("etc/hotspot");
		else
		show_help("0Announce/bbslist/day");
		return FULLUPDATE;
		case 'O':
		if (!strcmp("guest", currentuser.userid))
		break;
		{
			char *userid=
			((struct fileheader*)&pnt[(locmem->crs_line - locmem->top_line) * ssize])->owner;
			if(!strcmp(userid, currentuser.userid))
			break;
			move(-1, 0);
			//% sprintf(genbuf, "确定要把 %s 加入好友名单吗",userid);
			sprintf(genbuf, "\xc8\xb7\xb6\xa8\xd2\xaa\xb0\xd1 %s \xbc\xd3\xc8\xeb\xba\xc3\xd3\xd1\xc3\xfb\xb5\xa5\xc2\xf0",userid);
			if (askyn(genbuf, NA, NA) == NA)
			return FULLUPDATE;
			if (follow(session.uid, userid, NULL)) {
				//% sprintf(genbuf, "成功关注 %s", userid);
				sprintf(genbuf, "\xb3\xc9\xb9\xa6\xb9\xd8\xd7\xa2 %s", userid);
				show_message(genbuf);
			}
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
			msg_more();
			return FULLUPDATE;
		/*        case 'L':		//chenhao 解决在文章列表时看信的问题
		 if(session.status == RMAIL) return DONOTHING;
		 savemode = session.status;
		 m_read();
		 set_user_status(ST_savemode);
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
		savemode = session.status;
		set_user_status(ST_QUERY);
		show_online();
		set_user_status(savemode);
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
		case 'o':
			if (!HAS_PERM(PERM_LOGIN))
				break;
			show_online_followings();
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

void i_read(int cmdmode, const char *direct, int (*dotitle) (), char *(*doentry) (), struct one_key *rcmdlist, int ssize) {
	struct keeploc * locmem;
	char lbuf[11];
	char * ptr;
	int lbc, recbase, mode, ch;
	int num, entries;

	screen_len = t_lines - 4;
	set_user_status(cmdmode);
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
			case ST_RMAIL:
				//% prints("没有任何新信件...");
				prints("\xc3\xbb\xd3\xd0\xc8\xce\xba\xce\xd0\xc2\xd0\xc5\xbc\xfe...");
				pressreturn();
				clear();
				break;
			case ST_ADMIN:
				//% prints("目前无注册单...");
				prints("\xc4\xbf\xc7\xb0\xce\xde\xd7\xa2\xb2\xe1\xb5\xa5...");
				pressreturn();
				clear();
				break;
			default:
//				getdata(-1, 0, "本版新成立 (P)发表文章 (Q)离开？[Q] ",
//						genbuf, 4, DOECHO, YEA);
				break;
		}
		free(pnt);
		return;
	}
	num = last_line - screen_len + 2;
	if (cmdmode==ST_ADMIN)
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
		if (ch >= '0' && ch <= '9') {
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
			set_user_status(cmdmode);
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
			case MODECHANGED: // chenhao 解决文章列表看信的问题
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
					;
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

static int search_author(struct keeploc *locmem, int offset, char *powner)
{
	static char author[IDLEN + 1];
	char ans[IDLEN + 1], pmt[STRLEN];
	char currauth[STRLEN];
	strcpy(currauth, powner);

	//% sprintf(pmt, "%s的文章搜寻作者 [%s]: ", offset> 0 ? "往後来" : "往先前", currauth);
	sprintf(pmt, "%s\xb5\xc4\xce\xc4\xd5\xc2\xcb\xd1\xd1\xb0\xd7\xf7\xd5\xdf [%s]: ", offset> 0 ? "\xcd\xf9\xe1\xe1\xc0\xb4" : "\xcd\xf9\xcf\xc8\xc7\xb0", currauth);
	move(-1, 0);
	clrtoeol();
	//Modified by IAMFAT 2002-05-27
	//IDLEN->IDLEN+1
	getdata(-1, 0, pmt, ans, IDLEN+1, DOECHO, YEA);
	if (ans[0] != '\0')
	strcpy(author, ans);
	else
	strcpy(author, currauth);

	return search_articles(locmem, author, 0, offset, 1,0);
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

static int search_post(struct keeploc *locmem, int offset)
{
	static char query[STRLEN];
	char ans[STRLEN], pmt[STRLEN];
	strcpy(ans, query);
	//% sprintf(pmt, "搜寻%s的文章 [%s]: ", offset> 0 ? "往後来" : "往先前", ans);
	sprintf(pmt, "\xcb\xd1\xd1\xb0%s\xb5\xc4\xce\xc4\xd5\xc2 [%s]: ", offset> 0 ? "\xcd\xf9\xe1\xe1\xc0\xb4" : "\xcd\xf9\xcf\xc8\xc7\xb0", ans);
	move(-1, 0);
	clrtoeol();
	getdata(-1, 0, pmt, ans, 50, DOECHO, YEA);
	if (ans[0] != '\0')
	strcpy(query, ans);

	return search_articles(locmem, query, 0, offset, -1, 0);
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
	int has_yinyan=0; //Add by everlove 制作合集
	char buf[80], ch[32], BMch, annpath[512];
	char *buf1 = buf;

	//added by iamfat 2002.10.27 加入同主题大D功能
	int subflag = false;

	struct keeploc *locmem;
	//% char SR_BMitems[9][7] = { "删除", "保留", "文摘", "精华区", "水文", "不可RE", "合集",
	char SR_BMitems[9][7] = { "\xc9\xbe\xb3\xfd", "\xb1\xa3\xc1\xf4", "\xce\xc4\xd5\xaa", "\xbe\xab\xbb\xaa\xc7\xf8", "\xcb\xae\xce\xc4", "\xb2\xbb\xbf\xc9RE", "\xba\xcf\xbc\xaf",
		//% "恢复", "合并" };
		"\xbb\xd6\xb8\xb4", "\xba\xcf\xb2\xa2" };
	//% char subBMitems[3][9] = { "相同主题", "相同作者", "相关主题" };
	char subBMitems[3][9] = { "\xcf\xe0\xcd\xac\xd6\xf7\xcc\xe2", "\xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf", "\xcf\xe0\xb9\xd8\xd6\xf7\xcc\xe2" };

	if (!in_mail) {
		if (session.status != ST_READING)
			return DONOTHING;
		if (fileinfo->owner[0] == '-')
			return DONOTHING;
		if (!am_curr_bm())
			return DONOTHING;
	}
	saveline(t_lines - 1, 0);
	move(-1, 0);
	clrtoeol();
	ch[0] = '\0';
	//% getdata(-1, 0, "执行: 1) 相同主题  2) 相同作者 3) 相关主题 0) 取消 [0]: ",
	getdata(-1, 0, "\xd6\xb4\xd0\xd0: 1) \xcf\xe0\xcd\xac\xd6\xf7\xcc\xe2  2) \xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf 3) \xcf\xe0\xb9\xd8\xd6\xf7\xcc\xe2 0) \xc8\xa1\xcf\xfb [0]: ",
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
	getdata(-1, 0, buf, ch, 3, DOECHO, YEA);
	BMch = atoi(ch);
	if(BMch<=0||BMch>8||(digestmode != 0 && BMch==3)
			||(digestmode>2 && BMch<3)) {
		saveline(t_lines - 1, 1);
		return DONOTHING;
	}
	move(-1, 0);
	//% sprintf(buf,"确定要执行%s[%s]吗",subBMitems[dotype-1],(BMch!=8)?SR_BMitems[BMch-1]:SR_BMitems[8]);
	sprintf(buf,"\xc8\xb7\xb6\xa8\xd2\xaa\xd6\xb4\xd0\xd0%s[%s]\xc2\xf0",subBMitems[dotype-1],(BMch!=8)?SR_BMitems[BMch-1]:SR_BMitems[8]);
	if (askyn(buf, NA, NA) == 0) {
		saveline(t_lines - 1, 1);
		return PARTUPDATE;
	}

	if (digestmode != TRASH_MODE && digestmode !=JUNK_MODE && BMch == 8) {
		//% getdata(-1, 0, "本主题加至版面第几篇后？", ch, 8, DOECHO, YEA);
		getdata(-1, 0, "\xb1\xbe\xd6\xf7\xcc\xe2\xbc\xd3\xd6\xc1\xb0\xe6\xc3\xe6\xb5\xda\xbc\xb8\xc6\xaa\xba\xf3\xa3\xbf", ch, 8, DOECHO, YEA);
		if (ch[0] < '0' || ch[0]> '9' )
			return PARTUPDATE;
		result = atoi(ch);
		struct fileheader fh;
		get_record (direct, &fh, sizeof (fh), result);
		gid = fh.gid;
		fileinfo->reid = fh.id;
		substitute_record (direct, fileinfo, sizeof (*fileinfo), ent);
	}

	/* Add by everlove 制作合集 */
	if(BMch == 7) {
		move(-1,0);
		//% if (askyn("制作的合集需要引言吗？", YEA, YEA) == YEA)
		if (askyn("\xd6\xc6\xd7\xf7\xb5\xc4\xba\xcf\xbc\xaf\xd0\xe8\xd2\xaa\xd2\xfd\xd1\xd4\xc2\xf0\xa3\xbf", YEA, YEA) == YEA)
			has_yinyan=YEA;
		else
			has_yinyan=NA;
	}
	/*The End */

	if (dotype == 3) {
		strcpy(keyword, "");
		//% getdata(-1, 0, "请输入主题关键字: ", keyword, 50, DOECHO, YEA);
		getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd6\xf7\xcc\xe2\xb9\xd8\xbc\xfc\xd7\xd6: ", keyword, 50, DOECHO, YEA);
		if (keyword[0] == '\0') {
			saveline(t_lines - 1, 1);
			return DONOTHING;
		}
	} else if (dotype == 1) {
		strcpy(keyword, fileinfo->title);
	} else {
		strcpy(keyword, fileinfo->owner);
	}

	/* Add by everlove 制作合集 */
	if( (dotype == 1 || dotype == 3) && (BMch == 7))
	{
		sprintf(buf, "tmp/%s.combine", currentuser.userid);
		if(dashf(buf)) unlink(buf);
	}
	/* The End */

	//added by iamfat 2002.10.30
	if(BMch==1) { //删除
		//% subflag=askyn("是否小d", YEA, YEA);
		subflag=askyn("\xca\xc7\xb7\xf1\xd0\xa1""d", YEA, YEA);
	}

	move(-1, 0);
	//% sprintf(buf, "是否从%s第一篇开始%s (Y)第一篇 (N)目前这一篇",
	sprintf(buf, "\xca\xc7\xb7\xf1\xb4\xd3%s\xb5\xda\xd2\xbb\xc6\xaa\xbf\xaa\xca\xbc%s (Y)\xb5\xda\xd2\xbb\xc6\xaa (N)\xc4\xbf\xc7\xb0\xd5\xe2\xd2\xbb\xc6\xaa",
			//% (dotype == 2) ? "该作者" : "此主题", SR_BMitems[BMch - 1]);
			(dotype == 2) ? "\xb8\xc3\xd7\xf7\xd5\xdf" : "\xb4\xcb\xd6\xf7\xcc\xe2", SR_BMitems[BMch - 1]);
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
				//% 对不起, 您没有设定丝路. 请先用 f 设定丝路.
				presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xc3\xbb\xd3\xd0\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7. \xc7\xeb\xcf\xc8\xd3\xc3 f \xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7.", -1);
				saveline(t_lines - 1, 1);
				return DONOTHING;
			}
			fscanf(fn,"%s",annpath);
			fclose(fn);
			if (!dashd(annpath)) {
				//% 您设定的丝路已丢失, 请重新用 f 设定.
				presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xd3\xc3 f \xc9\xe8\xb6\xa8.", -1);
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
					a_Import(SR_fptr.title, SR_fptr.filename, YEA);
					break;
					/* Add by everlove 制作合集 */
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
					a_Import(SR_fptr.title, SR_fptr.filename, YEA);
					break;
				case 5:
					makeDELETEDflag(locmem->crs_line,&SR_fptr,currdirect);
					break;
				case 6:
					underline_post(locmem->crs_line,&SR_fptr,currdirect);
					break;
					/* Add by everlove 制作合集 */
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

	if (BMch == 7) {
		if (strneq(keyword, "Re: ", 4) || strneq(keyword, "RE: ", 4))
			//% snprintf(buf, sizeof(buf), "[合集]%s", keyword + 4);
			snprintf(buf, sizeof(buf), "[\xba\xcf\xbc\xaf]%s", keyword + 4);
		else
			//% snprintf(buf, sizeof(buf), "[合集]%s", keyword);
			snprintf(buf, sizeof(buf), "[\xba\xcf\xbc\xaf]%s", keyword);

		ansi_filter(keyword, buf);

		sprintf(buf, "tmp/%s.combine", currentuser.userid);

		if (in_mail)
			mail_file(buf, currentuser.userid, keyword);
		else
			Postfile(buf, currboard, keyword, 2);
		unlink(buf);
	}
	/* The End */
	if (!in_mail && BMch != 7 && !(dotype == 1 && BMch == 1)) {
		//% sprintf (buf, "%s版\"b\"%s%s", currboard, subBMitems[dotype
		sprintf (buf, "%s\xb0\xe6\"b\"%s%s", currboard, subBMitems[dotype
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

int combine_thread(int ent, struct fileheader *fileinfo, char *direct)
{
	char buf[16];
	int num;
	struct fileheader fh;
	//% getdata(-1, 0, "合并到版面第几篇后？",buf, 6, DOECHO, YEA);
	getdata(-1, 0, "\xba\xcf\xb2\xa2\xb5\xbd\xb0\xe6\xc3\xe6\xb5\xda\xbc\xb8\xc6\xaa\xba\xf3\xa3\xbf",buf, 6, DOECHO, YEA);
	if (buf[0] < '0'|| buf[0]> '9' )
	return PARTUPDATE;
	num = atoi(buf);
	get_record (direct, &fh, sizeof (fh), num);
	fileinfo->gid = fh.gid;
	fileinfo->reid = fh.id;
	substitute_record (direct, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

int _combine_thread(int ent, struct fileheader *fileinfo, char *direct, int gid)
{
	fileinfo->gid = gid;
	return substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
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

enum {
	SEARCH_BACKWARD = -1,
	SEARCH_FORWARD = 1,
	SEARCH_FIRST = 5,
	SEARCH_LAST = 3,

	SEARCH_CONTENT = -1,
	SEARCH_THREAD = 0,
	SEARCH_AUTHOR = 1,
	SEARCH_RELATED = 2,
};

static int searchpattern(const char *filename, const char *query)
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

static int strcasecmp2(const char *s1, const char *s2)
{
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

static int search_articles(struct keeploc *locmem, const char *query, int gid,
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
		if (offset == SEARCH_FIRST)
			offset = SEARCH_FORWARD;
		else
			offset = SEARCH_BACKWARD;
	} else {
		ent = locmem->crs_line;
		oldent = locmem->crs_line;
	}
	if (aflag != SEARCH_CONTENT && offset < 0)
		ent = 0;

	if (aflag == SEARCH_CONTENT) {
		move(-1, 0);
		clrtoeol();
		//% prints("\033[1;44;33m搜寻中，请稍候....                      "
		prints("\033[1;44;33m\xcb\xd1\xd1\xb0\xd6\xd0\xa3\xac\xc7\xeb\xc9\xd4\xba\xf2....                      "
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
		if (newflag && !brc_unread_legacy(SR_fptr.filename))
			continue;

		if (aflag == SEARCH_CONTENT) {
			char p_name[256];
			if (session.status != ST_RMAIL) {
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
	move(-1, 0);
	clrtoeol();
	fclose(fp);
	if (lastent == 0)
		return -1;
	get_record(currdirect, &SR_fptr, sizeof(SR_fptr), lastent);
	last_line = get_num_records(currdirect, sizeof(SR_fptr));
	return (cursor_pos(locmem, lastent, 10));
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

int search_title(struct keeploc *locmem, int offset)
{
	static char title[STRLEN];
	char ans[STRLEN], pmt[STRLEN];
	strcpy(ans, title);
	//% sprintf(pmt, "%s搜寻标题 [%.16s]: ", offset> 0 ? "往後" : "往前", ans);
	sprintf(pmt, "%s\xcb\xd1\xd1\xb0\xb1\xea\xcc\xe2 [%.16s]: ", offset> 0 ? "\xcd\xf9\xe1\xe1" : "\xcd\xf9\xc7\xb0", ans);
	move(-1, 0);
	clrtoeol();
	getdata(-1, 0, pmt, ans, 46, DOECHO, YEA);
	if (*ans != '\0')
		strcpy(title, ans);
	return search_articles(locmem, title, 0, offset, 2, 0);
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

int search_thread(struct keeploc *locmem, int offset, struct fileheader *fh)
{
	char *title = fh->title;

	if (title[0] == 'R' && (title[1] == 'e' || title[1] == 'E') && title[2] == ':')
	title += 4;
	setqtitle(title, fh->gid);
	return search_articles(locmem, title, fh->gid, offset, 0, 0);
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

int sread(int readfirst, int auser, struct fileheader *ptitle)
{
	struct keeploc *locmem;
	int rem_top, rem_crs, ch;
	int isend = 0, isstart = 0, isnext = 1;
	char tempbuf[STRLEN], title[STRLEN];

	if (readfirst) {
		isstart = 1;
	} else {
		isstart = 0;
		move(-1, 0);
		clrtoeol();
		prints(
				//% "[1;44;31m[%8s] [33m下一封 <Space>,<Enter>,↓│上一封 ↑,U                              [m",
				"[1;44;31m[%8s] [33m\xcf\xc2\xd2\xbb\xb7\xe2 <Space>,<Enter>,\xa1\xfd\xa9\xa6\xc9\xcf\xd2\xbb\xb7\xe2 \xa1\xfc,U                              [m",
				//% auser ? "相同作者" : "主题阅读");
				auser ? "\xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf" : "\xd6\xf7\xcc\xe2\xd4\xc4\xb6\xc1");
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
		if (session.status != ST_RMAIL)
			setbfile(tempbuf, currboard, SR_fptr.filename);
		else
			sprintf(tempbuf, "mail/%c/%s/%s",
					toupper(currentuser.userid[0]), currentuser.userid,
					SR_fptr.filename);
		setquotefile(tempbuf);
		ch = ansimore(tempbuf, NA);
		brc_addlist_legacy(SR_fptr.filename);
		isstart = 0;
		if (ch == KEY_UP || ch == 'u' || ch == 'U') {
			readfirst = (ch == KEY_UP);
			isnext = -1;
			continue;
		}
		move(-1, 0);
		clrtoeol();
		prints(
				//% "\033[1;44;31m[%8s] \033[33m回信 R │ 结束 Q,← │下一封 ↓,Enter│上一封 ↑,U │ ^R 回给作者   \033[m",
				"\033[1;44;31m[%8s] \033[33m\xbb\xd8\xd0\xc5 R \xa9\xa6 \xbd\xe1\xca\xf8 Q,\xa1\xfb \xa9\xa6\xcf\xc2\xd2\xbb\xb7\xe2 \xa1\xfd,Enter\xa9\xa6\xc9\xcf\xd2\xbb\xb7\xe2 \xa1\xfc,U \xa9\xa6 ^R \xbb\xd8\xb8\xf8\xd7\xf7\xd5\xdf   \033[m",
				//% auser ? "相同作者" : "主题阅读");
				auser ? "\xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf" : "\xd6\xf7\xcc\xe2\xd4\xc4\xb6\xc1");
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
				board_t board;
				if (!get_board(currboard, &board))
					return DONOTHING;
				if (!get_records(currdirect, ptitle, sizeof(*ptitle),
						rem_crs, 1))
					return DONOTHING;
				bool noreply = (ptitle->accessed[0] & FILE_NOREPLY)
						|| (board.flag & BOARD_NOREPLY_FLAG);
				if (!noreply || am_curr_bm()) {
					do_reply(ptitle);
				} else {
					clear();
					move(5, 6);
					//% prints("对不起, 该文章有不可 RE 属性, 您不能回复(RE) 这篇文章.");
					prints("\xb6\xd4\xb2\xbb\xc6\xf0, \xb8\xc3\xce\xc4\xd5\xc2\xd3\xd0\xb2\xbb\xbf\xc9 RE \xca\xf4\xd0\xd4, \xc4\xfa\xb2\xbb\xc4\xdc\xbb\xd8\xb8\xb4(RE) \xd5\xe2\xc6\xaa\xce\xc4\xd5\xc2.");
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
//				post_reply(0, &SR_fptr, (char *) NULL);
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

int locate_the_post(struct fileheader *fileinfo, char *query, int offset, //-1 当前向上  1 当前向下  3 最后一篇 5 第一篇
		int aflag, // 1 owner  0 同主题   2 相关主题
		int newflag // 1 必须为新文章   0 新旧均可
) {
	struct keeploc *locmem;
	locmem = getkeep(currdirect, 1, 1);
	if (query[0]=='R'&&(query[1]=='e'||query[1]=='E')&&query[2]==':')
		query += 4;
	setqtitle(query, fileinfo->gid);
	return search_articles(locmem, query, fileinfo->gid, offset, aflag,
			newflag);
}
