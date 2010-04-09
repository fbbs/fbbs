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

int mailmode, numf;
int friendmode = 0;
int range, page, readplan;

struct user_info *user_record[USHM_SIZE];
struct userec *user_data;
// add by Flier - 2000.5.12 - Begin
enum sort_type {stUserID, stUserName, stIP, stState} st = stUserID;
// add by Flier - 2000.5.12 - End

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

//	ÓÃ»§uentp.uidÓëÓÃ»§uinfo.uidÊÇ·ñ±»¶Ô·½Éè½øÁËºÚÃûµ¥
int isreject(struct user_info *uentp) {
	int i;

	if (uentp->uid != uinfo.uid) {
		for (i = 0; i<MAXREJECTS&&(uentp->reject[i]||uinfo.reject[i]); i++) {
			if (uentp->reject[i]==uinfo.uid||uentp->uid==uinfo.reject[i])
				return YEA; /* ±»ÉèÎªºÚÃûµ¥ */
		}
	}
	return NA;
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

/*******************Modify following two functions to support Type 2 mailall by Ashinmarch 2008.3.30*******************/
/*******************ÏêÏ¸ËµÃ÷¼ûmail.cµÄmailtoallº¯Êý********************************************************************/
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
	/***************°Ñtype2¶ÀÁ¢³öÀ´×öÅÐ¶Ï£¬µ÷ÓÃsharedmail_fileº¯Êý************************/
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
	/********Ê¹ÓÃapply_recordº¯ÊýÖÐµÄvoid *args²ÎÊý´«µÝ¹²ÏíÎÄ¼þµÄÎÄ¼þÃû*********/
	mailmode = mode;
	if (apply_record(PASSFILE, mailto, sizeof(struct userec),
			(char*)fname , 0, 0, false) == -1) {
		prints("No Users Exist");
		pressreturn();
		return 0;
	}
	return 1;
}

setlistrange(i)
int i;
{
	range = i;
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

	cp->all = 0;
	cp->cur = 0;
	cp->start = 0;
	cp->update = FULLUPDATE;
	cp->valid = false;
	
	while (!end) {
		if (!cp->valid) {
			if ((*cp->loader)(cp) < 0)
				break;
			cp->valid = true;
			if (cp->update != FULLUPDATE)
				cp->update = PARTUPDATE;
		}

		// Rolling.
		if (cp->cur < 0) {
			cp->cur = cp->all - 1;
		}
		if (cp->cur >= cp->all) {
			cp->cur = 0;
		}

		if (cp->cur < cp->start || cp->cur >= cp->start + BBS_PAGESIZE) {
			cp->start = (cp->cur / BBS_PAGESIZE) * BBS_PAGESIZE;
			if (cp->update != FULLUPDATE)
				cp->update = PARTUPDATE;
		}

		if (cp->update != FULLUPDATE && cp->all > BBS_PAGESIZE)
			cp->update = PARTUPDATE;

		if (cp->update != DONOTHING) {
			if (cp->update == FULLUPDATE) {
				clear();
				(*cp->title)(cp);
				cp->update = PARTUPDATE;
			}
			if (cp->update == PARTUPDATE) {
				move(LIST_START, 0);
				clrtobot();
				if ((*cp->display)(cp) == -1)
					return -1;
			}
			update_endline();
			cp->update = DONOTHING;
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
			case KEY_LEFT:
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
					ret = (*cp->handler)(cp, ch);
					if (ret < 0)
						end = true;
					else
						cp->update = ret;
				}
				break;
		}
	}
	return 0;
}
