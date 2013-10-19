#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/list.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

enum {
	POST_LIST_POSITION_KEY_LEN = 8,
};

typedef struct post_list_position_t {
	char key[POST_LIST_POSITION_KEY_LEN];
	int top;
	int cur;
	SLIST_FIELD(post_list_position_t) next;
} post_list_position_t;

SLIST_HEAD(post_list_position_list_t, post_list_position_t);

typedef struct {
	record_t *record;
	record_t *record_sticky;
	post_index_record_t *pir;
	post_info_t *buf;
	post_list_position_t *plp;
	post_id_t current_tid;
	post_id_t mark_pid;
	post_list_type_e type;
	int bid;
	int archive;
	int record_count;
} post_list_t;

static void post_list_position_key(const post_list_t *pl, char *buf)
{
	memcpy(buf, &(pl->bid), sizeof(pl->bid));
	buf[4] = pl->type;
}

static post_list_position_t *get_post_list_position(const post_list_t *pl)
{
	static struct post_list_position_list_t *list = NULL;
	if (!list) {
		list = malloc(sizeof(*list));
		SLIST_INIT_HEAD(list);
	}

	char buf[POST_LIST_POSITION_KEY_LEN] = { 0 };
	post_list_position_key(pl, buf);

	SLIST_FOREACH(post_list_position_t, plp, list, next) {
		if (memcmp(plp->key, buf, sizeof(plp->key)) == 0)
			return plp;
	}

	post_list_position_t *plp = malloc(sizeof(*plp));
	memcpy(plp->key, buf, sizeof(plp->key));
	plp->cur = -1;
	plp->top = -1;
	SLIST_INSERT_HEAD(list, plp, next);
	return plp;
}

static void save_post_list_position(tui_list_t *tl)
{
	post_list_t *pl = tl->data;
	if (!pl->plp)
		return;
	pl->plp->top = tl->begin;
	pl->plp->cur = tl->cur;
}

static int last_read_filter(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	fb_time_t *stamp = args;
	if (pib->stamp > *stamp)
		return -1;
	return 0;
}

static void load_posts(tui_list_t *tl)
{
	post_list_t *pl = tl->data;
	tl->all = pl->record_count = record_count(pl->record);
	if (pl->record_sticky)
		tl->all += record_count(pl->record_sticky);

	if (tl->begin > tl->all)
		tl->begin = tl->all - tl->lines;
	if (tl->begin < 0)
		tl->begin = 0;

	if (tl->begin < pl->record_count) {
		int loaded = post_index_board_read(pl->record, tl->begin, pl->pir,
				pl->buf, tl->lines, pl->type);
		if (loaded < tl->lines && tl->all > pl->record_count
				&& pl->record_sticky) {
			post_index_board_read(pl->record_sticky, 0, pl->pir,
					pl->buf + loaded, tl->lines - loaded, POST_LIST_NORMAL);
		}
	} else {
		if (pl->record_sticky) {
			post_index_board_read(pl->record_sticky,
					tl->begin - pl->record_count, pl->pir, pl->buf, tl->lines,
					POST_LIST_NORMAL);
		}
	}
}

static tui_list_loader_t post_list_loader(tui_list_t *tl)
{
	post_list_t *pl = tl->data;
	if (!pl->plp) {
		pl->plp = get_post_list_position(pl);
		if (pl->plp->top < 0) {
			fb_time_t stamp = brc_last_read();
			int offset = record_search(pl->record, last_read_filter, &stamp,
					-1, true);
			if (offset > 0) {
				tl->cur = offset + 1;
				tl->begin = tl->cur - tl->lines / 2;
				if (tl->begin < 0)
					tl->begin = 0;
			} else {
				tl->cur = tl->begin = 0;
			}
		} else {
			tl->cur = pl->plp->cur;
			tl->begin = pl->plp->top;
		}
	}

	load_posts(tl);
	save_post_list_position(tl);
	return 0;
}

static basic_session_info_t *get_sessions_by_name(const char *uname)
{
	return db_query("SELECT "BASIC_SESSION_INFO_FIELDS
			" FROM sessions s JOIN alive_users u ON s.user_id = u.id"
			" WHERE s.active AND lower(u.name) = lower(%s)", uname);
}

static int bm_color(const char *uname)
{
	basic_session_info_t *s = get_sessions_by_name(uname);
	int visible = 0, color = 33;

	if (s) {
		for (int i = 0; i < basic_session_info_count(s); ++i) {
			if (basic_session_info_visible(s, i))
				++visible;
		}

		if (visible)
			color = 32;
		else if (HAS_PERM(PERM_SEECLOAK) && basic_session_info_count(s) > 0)
			color = 36;
	}
	basic_session_info_clear(s);
	return color;
}

enum {
	BM_NAME_LIST_LEN = 56,
};

static int show_board_managers(const board_t *bp)
{
	prints("\033[33m");

	if (!bp->bms[0]) {
		//% prints("诚征版主中");
		prints("\xb3\xcf\xd5\xf7\xb0\xe6\xd6\xf7\xd6\xd0");
		return 10;
	}

	char bms[sizeof(bp->bms)];
	strlcpy(bms, bp->bms, sizeof(bms));

	//% prints("版主:");
	prints("\xb0\xe6\xd6\xf7:");
	int width = 5;
	for (const char *s = strtok(bms, " "); s; s = strtok(NULL, " ")) {
		++width;
		prints(" ");

		int len = strlen(s);
		if (width + len > BM_NAME_LIST_LEN) {
			prints("...");
			return width + 3;
		}

		width += len;
		int color = bm_color(s);
		prints("\033[%dm%s", color, s);
	}

	prints("\033[36m");
	return width;
}

static void repeat(int c, int repeat)
{
	for (int i = 0; i < repeat; ++i)
		outc(c);
}

static void align_center(const char *s, int remain)
{
	int len = strlen(s);
	if (len > remain) {
		prints("%s", s);
	} else {
		int spaces = (remain - len) / 2;
		repeat(' ', spaces);
		prints("%s", s);
		spaces = remain - len - spaces;
		repeat(' ', spaces);
	}
}

static void show_prompt(const board_t *bp, const char *prompt, int remain)
{
	GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
	convert_u2g(bp->descr, gbk_descr);
	int blen = strlen(bp->name) + 2;
	int plen = prompt ? strlen(prompt) : strlen(gbk_descr);

	if (blen + plen > remain) {
		if (prompt) {
			align_center(prompt, remain);
		} else {
			repeat(' ', remain - blen);
			prints("[%s]", bp->name);
		}
		return;
	} else {
		align_center(prompt ? prompt : gbk_descr, remain - blen);
		prints("\033[33m[%s]", bp->name);
	}
}

static const char *mode_description(post_list_type_e type)
{
	const char *s;
	switch (type) {
		case POST_LIST_NORMAL:
			if (DEFINE(DEF_THESIS))
				//% 主题模式
				s = "\xd6\xf7\xcc\xe2\xc4\xa3\xca\xbd";
			else
				//% 一般模式
				s =  "\xd2\xbb\xb0\xe3\xc4\xa3\xca\xbd";
			break;
		case POST_LIST_THREAD:
			//% 同主题
			s = "\xcd\xac\xd6\xf7\xcc\xe2";
			break;
		case POST_LIST_TOPIC:
			//% 原作
			s = "\xd4\xad\xd7\xf7";
			break;
		case POST_LIST_MARKED:
			//% 保留
			s = "\xb1\xa3\xc1\xf4";
			break;
		case POST_LIST_DIGEST:
			//% 文摘
			s = "\xce\xc4\xd5\xaa";
			break;
		case POST_LIST_AUTHOR:
			//% 同作者
			s = "\xcd\xac\xd7\xf7\xd5\xdf";
			break;
		case POST_LIST_KEYWORD:
			//% 标题关键字
			s = "\xb1\xea\xcc\xe2\xb9\xd8\xbc\xfc\xd7\xd6";
			break;
		case POST_LIST_TRASH:
			//% 垃圾箱
			s = "\xc0\xac\xbb\xf8\xcf\xe4";
			break;
		case POST_LIST_JUNK:
			//% 站务垃圾箱
			s = "\xd5\xbe\xce\xf1\xc0\xac\xbb\xf8\xcf\xe4";
			break;
		default:
			//% s = "未定义";
			s = "\xce\xb4\xb6\xa8\xd2\xe5";
	}
	return s;
}

void _post_list_title(int archive_list, const char *mode)
{
	prints("\033[1;44m");
	int width = show_board_managers(currbp);

	const char *prompt = NULL;
	if (chkmail())
		//% prompt = "[您有信件，按 M 看新信]";
		prompt = "[\xc4\xfa\xd3\xd0\xd0\xc5\xbc\xfe\xa3\xac\xb0\xb4 M \xbf\xb4\xd0\xc2\xd0\xc5]";
	else if ((currbp->flag & BOARD_VOTE_FLAG))
		//% prompt = "※投票中,按 v 进入投票※";
		prompt = "\xa1\xf9\xcd\xb6\xc6\xb1\xd6\xd0,\xb0\xb4 v \xbd\xf8\xc8\xeb\xcd\xb6\xc6\xb1\xa1\xf9";
	show_prompt(currbp, prompt, 80 - width);

	move(1, 0);
	//% prints("\033[m 离开[\033[1;32m←\033[m,\033[1;32me\033[m] "
	prints("\033[m \xc0\xeb\xbf\xaa[\033[1;32m\xa1\xfb\033[m,\033[1;32me\033[m] "
		//% "选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] "
		"\xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd\033[m] "
		//% "阅读[\033[1;32m→\033[m,\033[1;32mRtn\033[m]");
		"\xd4\xc4\xb6\xc1[\033[1;32m\xa1\xfa\033[m,\033[1;32mRtn\033[m]");
	if (!archive_list) {
		//% prints(" 发文章[\033[1;32mCtrl-P\033[m] 砍信[\033[1;32md\033[m] "
		prints(" \xb7\xa2\xce\xc4\xd5\xc2[\033[1;32mCtrl-P\033[m] \xbf\xb3\xd0\xc5[\033[1;32md\033[m] "
				//% "备忘录[\033[1;32mTAB\033[m]");
				"\xb1\xb8\xcd\xfc\xc2\xbc[\033[1;32mTAB\033[m]");
	}
	//% prints(" 求助[\033[1;32mh\033[m]\n");
	prints(" \xc7\xf3\xd6\xfa[\033[1;32mh\033[m]\n");

	//% 编号 在线 刊登者 日期 标题
	prints("\033[1;37;44m  \xb1\xe0\xba\xc5   %-12s %6s %-25s "
			"\xd4\xda\xcf\xdf:%-4d", "\xbf\xaf \xb5\xc7 \xd5\xdf",
			"\xc8\xd5  \xc6\xda", " \xb1\xea  \xcc\xe2",
			count_onboard(currbp->id));
	if (archive_list)
		//% prints("[存档]");
		prints("[\xb4\xe6\xb5\xb5]");
	else
		prints("    [%s]", mode);
	prints("\033[K\033[m\n");
	clrtobot();

}

static tui_list_title_t post_list_title(tui_list_t *tl)
{
	post_list_t *pl = tl->data;
	const char *mode = mode_description(pl->type);
	_post_list_title(pl->archive, mode);
}

#ifdef ENABLE_FDQUAN
#define ENABLE_COLOR_ONLINE_STATUS
#endif

#ifdef ENABLE_COLOR_ONLINE_STATUS
const char *get_board_online_color(const char *uname, int bid)
{
	if (streq(uname, currentuser.userid))
		return session.visible ? "1;37" : "1;36";

	const char *color = "";
	bool online = false;
	int invisible = 0;

	basic_session_info_t *s = get_sessions_by_name(uname);
	if (s) {
		for (int i = 0; i < basic_session_info_count(s); ++i) {
			if (!basic_session_info_visible(s, i)) {
				if (HAS_PERM(PERM_SEECLOAK))
					++invisible;
				else
					continue;
			}

			online = true;
			if (get_current_board(basic_session_info_sid(s, i)) == bid) {
				if (basic_session_info_visible(s, i))
					color = "1;37";
				else
					color = "1;36";
				basic_session_info_clear(s);
				return color;
			}
		}
	}

	if (!online)
		color = "1;30";
	else if (invisible > 0 && invisible == basic_session_info_count(s))
		color = "36";

	basic_session_info_clear(s);
	return color;
}
#else
#define get_board_online_color(n, i)  ""
#endif

const char *get_post_date(fb_time_t stamp)
{
#ifdef FDQUAN
	if (fb_time() - stamp < 24 * 60 * 60)
		return fb_ctime(&stamp) + 10;
#endif
	return fb_ctime(&stamp) + 4;
}

static tui_list_display_t post_list_display(tui_list_t *tl, int offset)
{
	post_list_t *pl = tl->data;
	post_info_t *pi = pl->buf + offset - tl->begin;

	char num[16];
	if (pi->flag & POST_FLAG_STICKY)
		//% "∞"
		strlcpy(num, " \033[1;31m[\xa1\xde]\033[m", sizeof(num));
	else
		snprintf(num, sizeof(num), "%5d", offset + 1);

	char mark = (pi->id == pl->mark_pid) ? '@' : get_post_mark(pi);

	const char *mark_prefix = "", *mark_suffix = "";
	if ((pi->flag & POST_FLAG_IMPORT) && am_curr_bm()) {
		mark_prefix = (mark == ' ') ? "\033[42m" : "\033[32m";
		mark_suffix = "\033[m";
	}

	const char *date = get_post_date(pi->stamp);

	const char *idcolor = get_board_online_color(pi->owner, currbp->id);

	char color[10] = "";
#ifdef COLOR_POST_DATE
	struct tm *mytm = fb_localtime(&pi->stamp);
	snprintf(color, sizeof(color), "\033[1;%dm", 30 + mytm->tm_wday + 1);
#endif

	char gbk_title[80];
	if (strneq(pi->utf8_title, "Re: ", 4)) {
		if (pl->type == POST_LIST_THREAD) {
			GBK_BUFFER(title2, POST_TITLE_CCHARS);
			convert_u2g(pi->utf8_title, gbk_title2);
			snprintf(gbk_title, sizeof(gbk_title), "%s %s",
					//% └├
					// TODO
					false ? "\xa9\xb8" : "\xa9\xc0", gbk_title2 + 4);
		} else {
			convert_u2g(pi->utf8_title, gbk_title);
		}
	} else {
		GBK_BUFFER(title2, POST_TITLE_CCHARS);
		convert_u2g(pi->utf8_title, gbk_title2);
		//% ◆
		snprintf(gbk_title, sizeof(gbk_title), "\xa1\xf4 %s", gbk_title2);
	}

	const int title_width = 49;
	if (is_deleted(pl->type)) {
		char buf[sizeof(gbk_title)], date[12];
		ellipsis(gbk_title,
				title_width - sizeof(date) - strlen(pi->ename) - 1);
		struct tm *t = fb_localtime(&pi->estamp);
		strftime(date, sizeof(date), "%m-%d %H:%M", t);
		snprintf(buf, sizeof(buf), "%s\033[1;%dm[%s %s]\033[m", gbk_title,
				(pi->flag & POST_FLAG_JUNK) ? 31 : 32, pi->ename, date);
		strlcpy(gbk_title, buf, sizeof(gbk_title));
	} else {
		ellipsis(gbk_title, title_width);
	}

	const char *thread_color = "0;37";
	if (pi->tid == pl->current_tid) {
		if (pi->id == pi->tid)
			thread_color = "1;32";
		else
			thread_color = "1;36";
	}

	char buf[128];
	snprintf(buf, sizeof(buf),
			" %s %s%c%s \033[%sm%-12.12s %s%6.6s %s\033[%sm%s\033[m\n",
			num, mark_prefix, mark, mark_suffix,
			idcolor, pi->owner, color, date,
			(pi->flag & POST_FLAG_LOCKED) ? "\033[1;33mx" : " ",
			thread_color, gbk_title);
	outs(buf);
	return 0;
}

static void reopen_post_record(post_list_t *pl, post_info_t *pi)
{
	if (pi->flag & POST_FLAG_STICKY) {
		record_close(pl->record_sticky);
		post_index_board_open_sticky(pl->bid, RECORD_WRITE, pl->record_sticky);
	} else if (pl->type == POST_LIST_NORMAL) {
		record_close(pl->record);
		post_index_board_open(pl->bid, RECORD_WRITE, pl->record);
	}
}

static int toggle_post_lock(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (pl->type == POST_LIST_NORMAL && pi) {
		bool locked = pi->flag & POST_FLAG_LOCKED;
		if (am_curr_bm() || (session.id == pi->uid && !locked)) {
			reopen_post_record(pl, pi);
			record_t *rec = (pi->flag & POST_FLAG_STICKY)
					? pl->record_sticky : pl->record;
			post_index_board_t pib = { .id = pi->id };
			set_post_flag_one(rec, &pib, tl->cur, POST_FLAG_LOCKED,
					false, true);
			tl->valid = false;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int toggle_post_stickiness(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (pl->type == POST_LIST_NORMAL && pi) {
		bool sticky = pi->flag & POST_FLAG_STICKY;
		if (sticky) {
			post_remove_sticky(pl->bid, pi->id);
		} else {
			post_add_sticky(pl->bid, pi);
		}
		tl->valid = false;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int toggle_post_flag(tui_list_t *tl, post_info_t *pi, post_flag_e flag)
{
	post_list_t *pl = tl->data;
	if (pl->type == POST_LIST_NORMAL && pi && am_curr_bm()) {
		reopen_post_record(pl, pi);
		record_t *rec = (pi->flag & POST_FLAG_STICKY)
			? pl->record_sticky : pl->record;
		post_index_board_t pib = { .id = pi->id };
		set_post_flag_one(rec, &pib, tl->cur, flag, false, true);
		tl->valid = false;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int post_list_with_filter(const post_filter_t *filter);

static int post_list_deleted(tui_list_t *tl, post_index_trash_e trash)
{
	post_list_t *pl = tl->data;
	if (pl->type != POST_LIST_NORMAL
			|| (trash == POST_INDEX_TRASH && !am_curr_bm())
			|| (trash == POST_INDEX_JUNK && !HAS_PERM(PERM_OBOARDS)))
		return DONOTHING;

	post_filter_t filter = {
		.type = trash == POST_INDEX_TRASH ? POST_LIST_TRASH : POST_LIST_JUNK,
		.bid = pl->bid,
	};
	post_list_with_filter(&filter);

	tl->valid = false;
	return FULLUPDATE;
}

typedef struct {
	post_index_record_t *pir;
	post_index_board_t *pib;
	const post_filter_t *filter;
	int size;
	int capacity;
} post_index_board_append_t;

static void filtered_record_name(char *file, size_t size)
{
	snprintf(file, size, "tmp/record_%d", getpid());
}

static int post_index_thread_cmp(const void *r1, const void *r2)
{
	const post_index_board_t *p1 = r1, *p2 = r2;
	int diff = (p1->id - p1->tid_delta) - (p2->id - p2->tid_delta);
	if (diff)
		return diff;
	return p1->id - p2->id;
}

static record_callback_e post_index_board_append(void *p, void *args,
		int offset)
{
	post_index_board_t *pib = p;
	post_index_board_append_t *piba = args;

	if (match_filter(pib, piba->pir, piba->filter, offset)) {
		if (piba->size < piba->capacity)
			piba->pib[piba->size++] = *pib;
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int filtered_record_open(const post_filter_t *f, record_perm_e rdonly,
		char *file, size_t size, record_t *record)
{
	record_cmp_t cmp;
	if (f->type == POST_LIST_THREAD)
		cmp = post_index_thread_cmp;
	else
		cmp = post_index_cmp;
	return record_open(file, cmp, sizeof(post_index_board_t), rdonly, record);
}

static int filtered_record_generate(record_t *r, post_filter_t *f,
		post_index_record_t *pir)
{
	if (f->type == POST_LIST_MARKED)
		f->flag |= POST_FLAG_MARKED;
	else if (f->type == POST_LIST_DIGEST)
		f->flag |= POST_FLAG_DIGEST;

	char file[HOMELEN];
	filtered_record_name(file, sizeof(file));
	record_t record;
	filtered_record_open(f, RECORD_WRITE, file, sizeof(file), &record);

	int count = record_count(r);
	if (count <= 0) {
		record_close(&record);
		unlink(file);
	}

	post_index_board_append_t piba = {
		.pib = malloc(sizeof(post_index_board_t) * count),
		.size = 0, .capacity = count, .pir = pir, .filter = f,
	};
	if (f->type == POST_LIST_THREAD) {
		piba.size = record_read_after(r, piba.pib, piba.capacity, 0);
		qsort(piba.pib, piba.size, sizeof(*piba.pib), post_index_thread_cmp);
	} else {
		record_foreach(r, NULL, 0, post_index_board_append, &piba);
	}
	record_append(&record, piba.pib, piba.size);
	record_close(&record);
	free(piba.pib);
	return 0;
}

static int tui_post_list_selected(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (!pi || !pl->bid || pl->type != POST_LIST_NORMAL)
		return DONOTHING;

	char ans[3];
	//% 切换模式到: 1)文摘 2)同主题 3)被 m 文章 4)原作 5)同作者 6)标题关键字
	getdata(-1, 0, "\xc7\xd0\xbb\xbb\xc4\xa3\xca\xbd\xb5\xbd:"
			" 1)\xce\xc4\xd5\xaa 2)\xcd\xac\xd6\xf7\xcc\xe2"
			" 3)\xb1\xbb m \xce\xc4\xd5\xc2 4)\xd4\xad\xd7\xf7"
			" 5)\xcd\xac\xd7\xf7\xd5\xdf"
			" 6)\xb1\xea\xcc\xe2\xb9\xd8\xbc\xfc\xd7\xd6 [1]: ",
			ans, sizeof(ans), DOECHO, YEA);
	int c = ans[0];
	if (!c)
		c = '1';
	c -= '1';

	const post_list_type_e types[] = {
		POST_LIST_DIGEST, POST_LIST_THREAD, POST_LIST_MARKED,
		POST_LIST_TOPIC, POST_LIST_AUTHOR, POST_LIST_KEYWORD,
	};
	if (c < 0 || c >= ARRAY_SIZE(types))
		return MINIUPDATE;

	post_filter_t filter = { .bid = pl->bid, .type = types[c] };
	if (filter.type == POST_LIST_AUTHOR) {
		char uname[IDLEN + 1];
		strlcpy(uname, pi->owner, sizeof(uname));
		//% 您想查找哪位网友的文章
		getdata(-1, 0, "\xc4\xfa\xcf\xeb\xb2\xe9\xd5\xd2\xc4\xc4"
				"\xce\xbb\xcd\xf8\xd3\xd1\xb5\xc4\xce\xc4\xd5\xc2? ",
				uname, sizeof(uname), DOECHO, false);
		user_id_t uid = get_user_id(uname);
		if (!uid)
			return MINIUPDATE;
		filter.uid = uid;
	} else if (filter.type == POST_LIST_KEYWORD) {
		GBK_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
		//% 您想查找的文章标题关键字
		getdata(-1, 0, "\xc4\xfa\xcf\xeb\xb2\xe9\xd5\xd2\xb5\xc4"
				"\xce\xc4\xd5\xc2\xb1\xea\xcc\xe2\xb9\xd8\xbc\xfc\xd7\xd6: ",
				gbk_keyword, sizeof(gbk_keyword), DOECHO, YEA);
		convert_g2u(gbk_keyword, filter.utf8_keyword);
		if (!filter.utf8_keyword[0])
			return MINIUPDATE;
	}

	filtered_record_generate(pl->record, &filter, pl->pir);
	post_list_with_filter(&filter);
	return FULLUPDATE;
}

typedef struct {
	post_index_record_t *pir;
	const post_filter_t *filter;
} post_index_board_filter_t;

static int post_index_board_filter(void *ptr, void *args, int offset)
{
	const post_index_board_filter_t *pibf = args;
	return match_filter(ptr, pibf->pir, pibf->filter, offset) ? 0 : -1;
}

static int post_search(tui_list_t *tl, const post_filter_t *filter,
		int offset, bool upward)
{
	post_list_t *pl = tl->data;
	post_index_board_filter_t pibf = {
		.pir = pl->pir, .filter = filter,
	};
	int pos = record_search(pl->record, post_index_board_filter, &pibf,
			offset, upward);
	if (pos >= 0) {
		tl->cur = pos;
		tl->valid = false;
		return PARTUPDATE;
	}
	return MINIUPDATE;
}

static int tui_search_author(tui_list_t *tl, const post_info_t *pi,
		bool upward)
{
	char prompt[80];
	//% 向%s搜索作者
	snprintf(prompt, sizeof(prompt),
			"\xcf\xf2%s\xcb\xd1\xcb\xf7\xd7\xf7\xd5\xdf [%s]: ",
			//% "上" : "下"
			upward ? "\xc9\xcf" : "\xcf\xc2", pi->owner);
	char ans[IDLEN + 1];
	getdata(-1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);

	user_id_t uid = pi->uid;
	if (*ans && !streq(ans, pi->owner))
		uid = get_user_id(ans);
	if (!uid)
		return MINIUPDATE;

	post_filter_t filter = { .uid = uid };
	return post_search(tl, &filter, tl->cur, upward);
}

static int tui_search_title(tui_list_t *tl, bool upward)
{
	char prompt[80];
	static GBK_BUFFER(title, POST_TITLE_CCHARS) = "";
	//% 向%s搜索标题
	snprintf(prompt, sizeof(prompt),
			"\xcf\xf2%s\xcb\xd1\xcb\xf7\xb1\xea\xcc\xe2[%s]: ",
			//% "上" : "下"
			upward ? "\xc9\xcf" : "\xcf\xc2", gbk_title);

	GBK_BUFFER(ans, POST_TITLE_CCHARS);
	getdata(-1, 0, prompt, gbk_ans, sizeof(gbk_ans), DOECHO, YEA);

	if (*gbk_ans != '\0')
		strlcpy(gbk_title, gbk_ans, sizeof(gbk_title));

	if (!*gbk_title != '\0')
		return MINIUPDATE;

	post_filter_t filter = { .utf8_keyword = { '\0' } };
	convert_g2u(gbk_title, filter.utf8_keyword);
	return post_search(tl, &filter, tl->cur, upward);
}

static int jump_to_thread_first(tui_list_t *tl, post_info_t *pi)
{
	if (pi && pi->id != pi->tid) {
		post_list_t *pl = tl->data;
		pl->current_tid = pi->tid;

		post_filter_t filter = { .tid = pi->tid };
		return post_search(tl, &filter, -1, false);
	}
	return DONOTHING;
}

static int jump_to_thread_prev(tui_list_t *tl, post_info_t *pi)
{
	if (!pi || pi->id == pi->tid)
		return DONOTHING;

	post_list_t *pl = tl->data;
	pl->current_tid = pi->tid;

	post_filter_t filter = { .tid = pi->tid };
	return post_search(tl, &filter, tl->cur, true);
}

static int jump_to_thread_next(tui_list_t *tl, post_info_t *pi)
{
	if (pi) {
		post_list_t *pl = tl->data;
		pl->current_tid = pi->tid;

		post_filter_t filter = { .tid = pi->tid };
		return post_search(tl, &filter, tl->cur, false);
	}
	return DONOTHING;
}

static int read_posts(tui_list_t *tl, post_info_t *pi, bool thread, bool user);

static int thread_first_unread_filter(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	post_id_t tid = *(post_id_t *) args;
	if (pib->id - pib->tid_delta == tid && brc_unread(pib->stamp))
		return 0;
	return -1;
}

static int jump_to_thread_first_unread(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (!pi || pl->type != POST_LIST_NORMAL)
		return DONOTHING;

	pl->current_tid = pi->tid;
	post_index_board_t pib;
	int pos = record_search_copy(pl->record, thread_first_unread_filter,
			&pi->tid, 0, false, &pib);
	if (pos >= 0) {
		tl->cur = pos;
		post_info_t pi_buf;
		post_index_board_to_info(pl->pir, &pib, &pi_buf, 1);
		read_posts(tl, &pi_buf, true, false);
		return FULLUPDATE;
	}
	return DONOTHING;
}

static int jump_to_thread_last(tui_list_t *tl, post_info_t *pi)
{
	if (pi) {
		post_list_t *pl = tl->data;
		pl->current_tid = pi->tid;

		post_filter_t filter = { .tid = pi->tid };
		return post_search(tl, &filter, -1, true);
	}
	return DONOTHING;
}

static int skip_post(tui_list_t *tl, post_info_t *pi)
{
	if (pi) {
		brc_mark_as_read(pi->stamp);
		if (++tl->cur >= tl->begin + tl->lines)
			tl->valid = false;
	}
	return DONOTHING;
}

static int tui_delete_single_post(tui_list_t *tl, post_info_t *pi, int bid)
{
	post_list_t *pl = tl->data;
	if (pl->type != POST_LIST_NORMAL)
		return DONOTHING;

	if (pi && (pi->uid == session.uid || am_curr_bm())) {
		move(-1, 0);
		//% 确定删除
		if (askyn("\xc8\xb7\xb6\xa8\xc9\xbe\xb3\xfd", NA, NA)) {
			post_filter_t filter = {
				.bid = bid, .min = pi->id, .max = pi->id,
			};
			if (post_index_board_delete(&filter, NULL, 0, true, false, true)) {
				tl->valid = false;
				return PARTUPDATE;
			}
		} else {
			return MINIUPDATE;
		}
	}
	return DONOTHING;
}

static int tui_undelete_single_post(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (pi && is_deleted(pl->type)) {
		post_filter_t f = { .bid = pl->bid, .min = pi->id, .max = pi->id, };
		if (post_index_board_undelete(&f, NULL, 0,
					pl->type == POST_LIST_TRASH)) {
			tl->valid = false;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int show_post_info(const post_info_t *pi)
{
	if (!pi)
		return DONOTHING;

	clear();
	move(0, 0);
	//% 版面文章的详细信息
	prints("\xb0\xe6\xc3\xe6\xce\xc4\xd5\xc2\xb5\xc4\xcf\xea\xcf\xb8"
			"\xd0\xc5\xcf\xa2:\n\n");

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pi->utf8_title, gbk_title);
	//% 标题
	prints("\xb1\xea\xcc\xe2: %s\n", gbk_title);
	//% 作者
	prints("\xd7\xf7\xd5\xdf: %s\n", pi->owner);
	//% 时间
	prints("\xca\xb1\xbc\xe4: %s\n", format_time(pi->stamp, TIME_FORMAT_ZH));

	prints("id:   %"PRIdPID"\ntid:  %"PRIdPID"\nreid: %"PRIdPID,
			pi->id, pi->tid, pi->reid);

	char link[STRLEN];
	snprintf(link, sizeof(link),
			"http://%s/bbs/con?new=1&bid=%d&f=%"PRIdPID"%s",
			BBSHOST, currbp->id, pi->id,
			(pi->flag & POST_FLAG_STICKY) ? "&s=1" : "");
	prints("\n%s", link);
	if (pi->flag & POST_FLAG_ARCHIVE)
		prints("&archive=1");
	prints("\n");

	pressanykey();
	return FULLUPDATE;
}

static bool dump_content(post_id_t id, char *file, size_t size)
{
	char buf[4096];
	char *str = post_content_get(id, buf, sizeof(buf));
	if (!str)
		return false;

	int ret = dump_content_to_gbk_file(str, strlen(str), file, size);

	if (str != buf)
		free(str);
	return ret == 0;
}

extern int tui_cross_post_legacy(const char *file, const char *title);

static int tui_cross_post(const post_info_t *pi)
{
	char file[HOMELEN];
	if (!pi || !dump_content(pi->id, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pi->utf8_title, gbk_title);

	tui_cross_post_legacy(file, gbk_title);

	unlink(file);
	return FULLUPDATE;
}

static int forward_post(const post_info_t *pi, bool uuencode)
{
	char file[HOMELEN];
	if (!pi || !dump_content(pi->id, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pi->utf8_title, gbk_title);

	tui_forward(file, gbk_title, uuencode);

	unlink(file);
	return FULLUPDATE;
}

static int reply_with_mail(const post_info_t *pi)
{
	char file[HOMELEN];
	if (!pi || !dump_content(pi->id, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pi->utf8_title, gbk_title);

	post_reply(pi->owner, gbk_title, file);

	unlink(file);
	return FULLUPDATE;
}

static int tui_edit_post_title(tui_list_t *tl, post_info_t *pi)
{
	if (!pi || (pi->uid != session.uid && !am_curr_bm()))
		return DONOTHING;

	GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);

	ansi_filter(utf8_title, pi->utf8_title);
	convert_u2g(utf8_title, gbk_title);

	//% 新文章标题
	getdata(-1, 0, "\xd0\xc2\xce\xc4\xd5\xc2\xb1\xea\xcc\xe2: ",
			gbk_title, sizeof(gbk_title), DOECHO, NA);

	check_title(gbk_title, sizeof(gbk_title));
	convert_g2u(gbk_title, utf8_title);

	if (!*utf8_title || streq(utf8_title, pi->utf8_title))
		return MINIUPDATE;

	post_list_t *pl = tl->data;
	strlcpy(pi->utf8_title, utf8_title, sizeof(pi->utf8_title));
	if (alter_title(pl->pir, pi))
		return PARTUPDATE;
	return MINIUPDATE;
}

static int tui_edit_post_content(post_info_t *pi)
{
	if (!pi || (pi->uid != session.uid && !am_curr_bm()))
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(pi->id, file, sizeof(file)))
		return DONOTHING;

	int status = session.status;
	set_user_status(ST_EDIT);

	clear();
	if (vedit(file, NA, NA, NULL) != -1) {
		char *content = convert_file_to_utf8_content(file);
		if (content) {
			if (post_content_write(pi->id, content, strlen(content))) {
				char buf[STRLEN];
				snprintf(buf, sizeof(buf), "edited post #%"PRIdPID, pi->id);
				report(buf, currentuser.userid);
			}
			free(content);
		}
	}

	unlink(file);
	set_user_status(status);
	return FULLUPDATE;
}

extern int show_board_notes(const char *bname, int command);
extern int noreply;

static int tui_new_post(int bid, post_info_t *pi)
{
	time_t now = fb_time();
	if (now - get_my_last_post_time() < 3) {
		move(-1, 0);
		clrtoeol();
		//% 您太辛苦了，先喝杯咖啡歇会儿，3 秒钟后再发表文章。
		prints("\xc4\xfa\xcc\xab\xd0\xc1\xbf\xe0\xc1\xcb\xa3\xac\xcf\xc8\xba\xc8\xb1\xad\xbf\xa7\xb7\xc8\xd0\xaa\xbb\xe1\xb6\xf9\xa3\xac""3 \xc3\xeb\xd6\xd3\xba\xf3\xd4\xd9\xb7\xa2\xb1\xed\xce\xc4\xd5\xc2\xa1\xa3\n");
		pressreturn();
		return MINIUPDATE;
	}

	board_t board;
	if (!get_board_by_bid(bid, &board) || !has_post_perm(&board)) {
		move(-1, 0);
		clrtoeol();
		//% prints("此讨论区是唯读的, 或是您尚无权限在此发表文章。");
		prints("\xb4\xcb\xcc\xd6\xc2\xdb\xc7\xf8\xca\xc7\xce\xa8\xb6\xc1\xb5\xc4, \xbb\xf2\xca\xc7\xc4\xfa\xc9\xd0\xce\xde\xc8\xa8\xcf\xde\xd4\xda\xb4\xcb\xb7\xa2\xb1\xed\xce\xc4\xd5\xc2\xa1\xa3");
		pressreturn();
		return FULLUPDATE;
	}

	int status = session.status;
	set_user_status(ST_POSTING);

	clear();
	show_board_notes(board.name, 1);

	struct postheader header = { .mail_owner = false, };
	GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
	if (pi) {
		header.reply = true;
		if (strncaseeq(pi->utf8_title, "Re: ", 4)) {
			convert_u2g(pi->utf8_title, header.title);
		} else {
			convert_u2g(pi->utf8_title, gbk_title);
			snprintf(header.title, sizeof(header.title), "Re: %s", gbk_title);
		}
	}

	strlcpy(header.ds, board.name, sizeof(header.ds));
	header.postboard = true;
	header.prefix[0] = '\0';

	if (post_header(&header) == YEA) {
		if (!header.reply && header.prefix[0]) {
#ifdef FDQUAN
			if (board.flag & BOARD_PREFIX_FLAG) {
				snprintf(gbk_title, sizeof(gbk_title),
						"\033[1;33m[%s]\033[m%s", header.prefix, header.title);
			} else {
				snprintf(gbk_title, sizeof(gbk_title),
						"\033[1m[%s]\033[m%s", header.prefix, header.title);
			}
#else
			snprintf(gbk_title, sizeof(gbk_title),
					"[%s]%s", header.prefix, header.title);
#endif
		} else {
			ansi_filter(gbk_title, header.title);
		}
	} else {
		set_user_status(status);
		return FULLUPDATE;
	}
	convert_g2u(gbk_title, utf8_title);

	set_my_last_post_time(fb_time());

	in_mail = NA;

	char file[HOMELEN];
	snprintf(file, sizeof(file), "tmp/editbuf.%d", getpid());
	if (pi) {
		char orig[HOMELEN];
		dump_content(pi->id, orig, sizeof(orig));
		do_quote(orig, file, header.include_mode, header.anonymous);
		unlink(orig);
	} else {
		do_quote("", file, header.include_mode, header.anonymous);
	}

	if (vedit(file, true, true, &header) == -1) {
		unlink(file);
		clear();
		set_user_status(status);
		return FULLUPDATE;
	}

	valid_title(header.title);

	if (header.mail_owner && header.reply) {
		if (header.anonymous) {
			//% prints("对不起，您不能在匿名版使用寄信给原作者功能。");
			prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xc4\xfa\xb2\xbb\xc4\xdc\xd4\xda\xc4\xe4\xc3\xfb\xb0\xe6\xca\xb9\xd3\xc3\xbc\xc4\xd0\xc5\xb8\xf8\xd4\xad\xd7\xf7\xd5\xdf\xb9\xa6\xc4\xdc\xa1\xa3");
		} else {
			if (!is_blocked(pi->owner)
					&& !mail_file(file, pi->owner, gbk_title)) {
				//% 信件已成功地寄给原作者
				prints("\xd0\xc5\xbc\xfe\xd2\xd1\xb3\xc9\xb9\xa6\xb5\xd8\xbc\xc4\xb8\xf8\xd4\xad\xd7\xf7\xd5\xdf %s", pi->owner);
			}
			else {
				//% 信件邮寄失败，%s 无法收信。
				prints("\xd0\xc5\xbc\xfe\xd3\xca\xbc\xc4\xca\xa7\xb0\xdc\xa3\xac%s \xce\xde\xb7\xa8\xca\xd5\xd0\xc5\xa1\xa3", pi->owner);
			}
		}
		pressanykey();
	}

	post_request_t req = {
		.autopost = false,
		.crosspost = false,
		.uname = currentuser.userid,
		.nick = currentuser.username,
		.user = &currentuser,
		.board = &board,
		.title = utf8_title,
		.content = NULL,
		.gbk_file = file,
		.sig = 0,
		.ip = NULL,
		.reid = pi ? pi->id : 0,
		.tid = pi ? pi->tid : 0,
		.locked = header.locked,
		.marked = false,
		.anony = header.anonymous,
		.cp = NULL,
	};

	post_id_t pid = publish_post(&req);
	unlink(file);
	if (pid) {
		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "posted '%s' on %s",
				gbk_title, board.name);
		report(buf, currentuser.userid);
	}

	if (!is_junk_board(&board) && !header.anonymous) {
		set_safe_record();
		currentuser.numposts++;
		substitut_record(PASSFILE, &currentuser, sizeof (currentuser),
				usernum);
	}
	bm_log(currentuser.userid, currboard, BMLOG_POST, 1);
	set_user_status(status);
	return FULLUPDATE;
}

static int tui_save_post(const post_info_t *pi)
{
	if (!pi || !am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(pi->id, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pi->utf8_title, gbk_title);

	a_Save(gbk_title, file, false, true);

	return MINIUPDATE;
}

static int tui_import_post(const post_info_t *pi)
{
	if (!pi || !HAS_PERM(PERM_BOARDS))
		return DONOTHING;

	if (DEFINE(DEF_MULTANNPATH)
			&& set_ann_path(NULL, NULL, ANNPATH_GETMODE) == 0)
		return FULLUPDATE;

	char file[HOMELEN];
	if (dump_content(pi->id, file, sizeof(file))) {
		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(pi->utf8_title, gbk_title);

		a_Import(gbk_title, file, NA);
	}

	if (DEFINE(DEF_MULTANNPATH))
		return FULLUPDATE;
	return DONOTHING;
}

static int tui_mark_range(const tui_list_t *tl, const post_info_t *pi,
		post_id_t *min, post_id_t *max)
{
	*min = *max = 0;
	if (!am_curr_bm())
		return DONOTHING;

	if (pi->flag & POST_FLAG_STICKY)
		return DONOTHING;

	post_list_t *pl = tl->data;
	if (!pl->mark_pid) {
		pl->mark_pid = pi->id;
		return PARTUPDATE;
	}

	if (pi->id >= pl->mark_pid) {
		*min = pl->mark_pid;
		*max = pi->id;
	} else {
		*min = pi->id;
		*max = pl->mark_pid;
	}
	pl->mark_pid = 0;
	return DONOTHING;
}

static int tui_delete_posts_in_range(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (pl->type != POST_LIST_NORMAL)
		return DONOTHING;

	post_id_t min, max;
	int ret = tui_mark_range(tl, pi, &min, &max);
	if (!max)
		return ret;

	move(-1, 0);
	clrtoeol();
	//% 确定删除
	if (askyn("\xc8\xb7\xb6\xa8\xc9\xbe\xb3\xfd", NA, NA)) {
		post_filter_t filter = { .bid = pl->bid, .min = min, .max = max };
		if (post_index_board_delete(&filter, NULL, 0, false,
				!HAS_PERM(PERM_OBOARDS), false)) {
			bm_log(currentuser.userid, currboard, BMLOG_DELETE, 1);
			tl->valid = false;
		}
		return PARTUPDATE;
	}
	move(-1, 50);
	clrtoeol();
	//% 放弃删除
	prints("\xb7\xc5\xc6\xfa\xc9\xbe\xb3\xfd...");
	egetch();
	return MINIUPDATE;
}

#if 0
static bool count_posts_in_range(post_id_t min, post_id_t max, bool asc,
		int sort, int least, char *file, size_t size)
{
	query_t *q = query_new(0);
	query_select(q, "uname, COUNT(*) AS a, SUM(marked::integer) AS m"
			", SUM(digest::integer) AS g, SUM(water::integer) AS w, "
			"SUM((NOT marked AND NOT digest AND NOT water)::integer) AS n");
	query_from(q, "posts_recent");
	query_where(q, "id >= %"DBIdPID, min);
	query_and(q, "id <= %"DBIdPID, max);
	query_groupby(q, "uname");

	const char *field[] = { "a", "m", "g", "w", "n" };
	query_orderby(q, field[sort], asc);

	db_res_t *res = query_exec(q);

	if (res) {
		snprintf(file, sizeof(file), "tmp/count.%d", getpid());
		FILE *fp = fopen(file, "w");
		if (fp) {
			//% fprintf(fp, "版    面: \033[1;33m%s\033[m\n", currboard);
			fprintf(fp, "\xb0\xe6    \xc3\xe6: \033[1;33m%s\033[m\n", currboard);

			int count = 0;
			for (int i = db_res_rows(res) - 1; i >=0; --i) {
				count += db_get_bigint(res, i, 1);
			}
			char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
			//% fprintf(fp, "有效篇数: \033[1;33m%d\033[m 篇"
			fprintf(fp, "\xd3\xd0\xd0\xa7\xc6\xaa\xca\xfd: \033[1;33m%d\033[m \xc6\xaa"
					" [\033[1;33m%s-%s\033[m]\n", count,
					pid_to_base32(min, min_str, sizeof(min_str)),
					pid_to_base32(max, max_str, sizeof(max_str)));
			
			const char *descr[] = {
				//% "总数", "被m的", "被g的", "被w的", "无标记"
				"\xd7\xdc\xca\xfd", "\xb1\xbbm\xb5\xc4", "\xb1\xbbg\xb5\xc4", "\xb1\xbbw\xb5\xc4", "\xce\xde\xb1\xea\xbc\xc7"
			};
			//% fprintf(fp, "排序方式: \033[1;33m按%s%s\033[m\n", descr[sort],
			fprintf(fp, "\xc5\xc5\xd0\xf2\xb7\xbd\xca\xbd: \033[1;33m\xb0\xb4%s%s\033[m\n", descr[sort],
					//% asc ? "升序" : "降序");
					asc ? "\xc9\xfd\xd0\xf2" : "\xbd\xb5\xd0\xf2");
			//% fprintf(fp, "文章数下限: \033[1;33m%d\033[m\n\n", least);
			fprintf(fp, "\xce\xc4\xd5\xc2\xca\xfd\xcf\xc2\xcf\xde: \033[1;33m%d\033[m\n\n", least);
			//% fprintf(fp, "\033[1;44;37m 使用者代号  │总  数│ 被M的│ 被G的"
			fprintf(fp, "\033[1;44;37m \xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5  \xa9\xa6\xd7\xdc  \xca\xfd\xa9\xa6 \xb1\xbbM\xb5\xc4\xa9\xa6 \xb1\xbbG\xb5\xc4"
					//% "│ 被w的│无标记 \033[m\n");
					"\xa9\xa6 \xb1\xbbw\xb5\xc4\xa9\xa6\xce\xde\xb1\xea\xbc\xc7 \033[m\n");

			for (int i = 0; i < db_res_rows(res); ++i) {
				int all = db_get_bigint(res, i, 1);
				if (all > least) {
					int m = db_get_bigint(res, i, 2);
					int g = db_get_bigint(res, i, 3);
					int w = db_get_bigint(res, i, 4);
					int n = db_get_bigint(res, i, 5);
					fprintf(fp, " %-12.12s  %6d  %6d  %6d  %6d  %6d \n",
							db_get_value(res, i, 0), all, m, g, w, n);
				}
			}
			fclose(fp);
			db_clear(res);
			return true;
		}
		db_clear(res);
	}
	return false;
}

static int tui_count_posts_in_range(slide_list_t *p)
{
	post_list_t *l = p->data;
	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	post_id_t min, max;
	int ret = tui_mark_range(p, &min, &max);
	if (!max)
		return ret;

	char num[8];
	//% getdata(-1, 0, "排序方式 (0)降序 (1)升序 ? : ", num, 2, DOECHO, YEA);
	getdata(-1, 0, "\xc5\xc5\xd0\xf2\xb7\xbd\xca\xbd (0)\xbd\xb5\xd0\xf2 (1)\xc9\xfd\xd0\xf2 ? : ", num, 2, DOECHO, YEA);
	bool asc = (num[0] == '1');

	//% getdata(-1, 0, "排序选项 (0)总数 (1)被m (2)被g (3)被w (4)无标记 ? : ",
	getdata(-1, 0, "\xc5\xc5\xd0\xf2\xd1\xa1\xcf\xee (0)\xd7\xdc\xca\xfd (1)\xb1\xbbm (2)\xb1\xbbg (3)\xb1\xbbw (4)\xce\xde\xb1\xea\xbc\xc7 ? : ",
			num, 2, DOECHO, YEA);
	int sort = strtol(num, NULL, 10);
	if (sort < 0 || sort > 4)
		sort = 0;

	//% getdata(-1, 0, "文章数下限(默认0): ", num, 6, DOECHO, YEA);
	getdata(-1, 0, "\xce\xc4\xd5\xc2\xca\xfd\xcf\xc2\xcf\xde(\xc4\xac\xc8\xcf""0): ", num, 6, DOECHO, YEA);
	int least = strtol(num, NULL, 10);

	char file[HOMELEN];
	if (!count_posts_in_range(min, max, asc, sort, least, file, sizeof(file)))
		return MINIUPDATE;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
	//% snprintf(gbk_title, sizeof(gbk_title), "[%s]统计文章数(%s-%s)",
	snprintf(gbk_title, sizeof(gbk_title), "[%s]\xcd\xb3\xbc\xc6\xce\xc4\xd5\xc2\xca\xfd(%s-%s)",
			currboard, pid_to_base32(min, min_str, sizeof(min_str)),
			pid_to_base32(max, max_str, sizeof(max_str)));
	mail_file(file, currentuser.userid, gbk_title);
	unlink(file);

	return MINIUPDATE;
}
#endif

static const char *get_prompt(bool thread, bool user)
{
	if (thread)
		//% [主题阅读] 下一封 <Space>,<Enter>,↓│上一封 ↑,U
		return "\033[0;1;31;44m[\xd6\xf7\xcc\xe2\xd4\xc4\xb6\xc1]"
			" \033[33m\xcf\xc2\xd2\xbb\xb7\xe2 <Space>,<Enter>,"
			"\xa1\xfd\xa9\xa6\xc9\xcf\xd2\xbb\xb7\xe2 \xa1\xfc,U"
			"                              ";
	if (user)
		//% [相同作者] 回信 R │ 结束 Q,← │下一封 ↓,Enter
		//% │上一封 ↑,U │ ^R 回给作者
		return "\033[0;1;31;44m[\xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf]"
			" \033[33m\xbb\xd8\xd0\xc5 R \xa9\xa6 \xbd\xe1\xca\xf8 Q,\xa1\xfb"
			" \xa9\xa6\xcf\xc2\xd2\xbb\xb7\xe2 \xa1\xfd,Enter\xa9\xa6\xc9\xcf"
			"\xd2\xbb\xb7\xe2 \xa1\xfc,U \xa9\xa6 ^R \xbb\xd8\xb8\xf8\xd7\xf7"
			"\xd5\xdf";
	//% [阅读文章]  回信 R │ 结束 Q,← │上一封 ↑
	//% "│下一封 <Space>,↓│主题阅读 ^s或p");
	return "\033[0;1;31;44m[\xd4\xc4\xb6\xc1\xce\xc4\xd5\xc2]  "
		"\033[33m\xbb\xd8\xd0\xc5 R \xa9\xa6 \xbd\xe1\xca\xf8 "
		"Q,\xa1\xfb \xa9\xa6\xc9\xcf\xd2\xbb\xb7\xe2 \xa1\xfc"
		"\xa9\xa6\xcf\xc2\xd2\xbb\xb7\xe2 <Space>,\xa1\xfd\xa9\xa6"
		"\xd6\xf7\xcc\xe2\xd4\xc4\xb6\xc1 ^s\xbb\xf2p";
}

static int read_posts(tui_list_t *tl, post_info_t *pi, bool thread, bool user)
{
	post_list_t *pl = tl->data;
	bool end = false, upward = false, sticky = false, entering = false;
	post_id_t thread_entry = 0, last_id = 0, tid = 0;
	user_id_t uid = 0;
	char file[HOMELEN];
	post_info_t pi_buf;

	while (!end) {
		int ch = 0;
		if (!entering) {
			if (!pi || !dump_content(pi->id, file, sizeof(file)))
				return DONOTHING;
			if (thread)
				tid = pi->tid;

			brc_mark_as_read(pi->stamp);
			last_id = pi->id;
			pl->current_tid = pi->tid;
			end = sticky = pi->flag & POST_FLAG_STICKY;

			ch = ansimore(file, false);
		}

		move(-1, 0);
		clrtoeol();
		prints(get_prompt(tid, uid));
		prints("\033[m");
		refresh();

		if (!(ch == KEY_UP || ch == KEY_PGUP))
			ch = egetch();
		entering = false;
		switch (ch) {
			case 'N': case 'Q': case 'n': case 'q': case KEY_LEFT:
				end = true;
				break;
			case 'Y': case 'R': case 'y': case 'r':
				end = true;
				tl->valid = false;
				tui_new_post(pi->bid, pi);
				break;
			case '\n': case 'j': case KEY_DOWN: case KEY_PGDN:
				upward = false;
				thread_entry = 0;
				break;
			case ' ': case 'p': case KEY_RIGHT: case Ctrl('S'):
				upward = false;
				if (!uid && !tid) {
					thread_entry = pi->id;
					tid = pi->tid;
					entering = true;
				}
				break;
			case KEY_UP: case KEY_PGUP: case 'u': case 'U':
				upward = true;
				break;
			case Ctrl('A'):
				t_query(pi->owner);
				break;
			case Ctrl('R'):
				reply_with_mail(pi);
				break;
			case 'g':
				toggle_post_flag(tl, pi, POST_FLAG_DIGEST);
				break;
			case '*':
				show_post_info(pi);
				break;
			case Ctrl('U'):
				if (!uid && !tid) {
					uid = pi->uid;
					user = true;
				}
				break;
			default:
				break;
		}

		if (end || !entering)
			unlink(file);

		if (!end && !entering) {
			post_filter_t filter = {
				.tid = tid,
				.uid = user ? pi->uid : 0,
			};
			post_index_board_filter_t pibf = {
				.pir = pl->pir, .filter = &filter,
			};
			post_index_board_t pib;
			int pos = record_search_copy(pl->record, post_index_board_filter,
					&pibf, tl->cur, upward, &pib);
			if (pos < 0) {
				end = true;
			} else {
				post_index_board_to_info(pl->pir, &pib, &pi_buf, 1);
				pi = &pi_buf;
				tl->cur = pos;
			}
		}
	}
	return FULLUPDATE;
}

static void construct_prompt(char *s, size_t size, const char **options,
		size_t len)
{
	char *p = s;
	//% strappend(&p, &size, "区段:");
	strappend(&p, &size, "\xc7\xf8\xb6\xce:");
	for (int i = 0; i < len; ++i) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d)%s", i + 1, options[i]);
		strappend(&p, &size, buf);
	}
	strappend(&p, &size, "[0]:");
}

#if 0
		clear();
		//% prints("\n\n您将进行区段转载。转载范围是：[%d -- %d]\n", num1, num2);
		prints("\n\n\xc4\xfa\xbd\xab\xbd\xf8\xd0\xd0\xc7\xf8\xb6\xce\xd7\xaa\xd4\xd8\xa1\xa3\xd7\xaa\xd4\xd8\xb7\xb6\xce\xa7\xca\xc7\xa3\xba[%d -- %d]\n", num1, num2);
		//% prints("当前版面是：[ %s ] \n", currboard);
		prints("\xb5\xb1\xc7\xb0\xb0\xe6\xc3\xe6\xca\xc7\xa3\xba[ %s ] \n", currboard);
		//% board_complete(6, "请输入要转贴的讨论区名称: ", bname, sizeof(bname),
		board_complete(6, "\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xd7\xaa\xcc\xf9\xb5\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6: ", bname, sizeof(bname),
				AC_LIST_BOARDS_ONLY);
		if (!*bname)
			return FULLUPDATE;

		if (!strcmp(bname, currboard)&&session.status != ST_RMAIL) {
			//% prints("\n\n对不起，本文就在您要转载的版面上，所以无需转载。\n");
			prints("\n\n\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xb1\xbe\xce\xc4\xbe\xcd\xd4\xda\xc4\xfa\xd2\xaa\xd7\xaa\xd4\xd8\xb5\xc4\xb0\xe6\xc3\xe6\xc9\xcf\xa3\xac\xcb\xf9\xd2\xd4\xce\xde\xd0\xe8\xd7\xaa\xd4\xd8\xa1\xa3\n");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		//% if (askyn("确定要转载吗", NA, NA)==NA)
		if (askyn("\xc8\xb7\xb6\xa8\xd2\xaa\xd7\xaa\xd4\xd8\xc2\xf0", NA, NA)==NA)
			return FULLUPDATE;
			case 4:
				break;
			case 5:
				break;
			default:
				break;
		}

	return DIRCHANGED;
}
#endif

extern int import_file(const char *title, const char *file, const char *path);

typedef struct {
	post_index_record_t *pir;
	const post_filter_t *filter;
	const char *path;
} import_posts_callback_t;

static record_callback_e import_posts_callback(void *r, void *args, int offset)
{
	const post_index_board_t *pib = r;
	import_posts_callback_t *ipc = args;

	if (match_filter(pib, ipc->pir, ipc->filter, offset)) {
		GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
		post_index_record_get_title(ipc->pir, pib->id,
				utf8_title, sizeof(utf8_title));
		convert_u2g(utf8_title, gbk_title);

		char file[HOMELEN];
		dump_content(pib->id, file, sizeof(file));
		import_file(gbk_title, file, ipc->path);
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static void import_posts(post_list_t *pl, post_filter_t *filter,
		const char *path)
{
	import_posts_callback_t ipc = {
		.pir = pl->pir, .filter = filter, .path = path,
	};
	record_foreach(pl->record, NULL, 0, import_posts_callback, &ipc);
}

static int tui_import_posts(post_list_t *pl, post_filter_t *filter)
{
	if (DEFINE(DEF_MULTANNPATH) && !!set_ann_path(NULL, NULL, ANNPATH_GETMODE))
		return FULLUPDATE;

	char path[256];
	sethomefile(path, currentuser.userid, ".announcepath");

	FILE *fp = fopen(path, "r");
	if (!fp) {
		//% 对不起, 您没有设定丝路. 请先用 f 设定丝路.
		presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xc3\xbb\xd3\xd0"
				"\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7. \xc7\xeb\xcf\xc8\xd3\xc3"
				" f \xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7.", -1);
		return MINIUPDATE;
	}

	fscanf(fp, "%s", path);
	fclose(fp);

	if (!dashd(path)) {
		//% 您设定的丝路已丢失, 请重新用 f 设定.
		presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7"
				"\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xd3\xc3"
				" f \xc9\xe8\xb6\xa8.", -1);
		return MINIUPDATE;
	}

	import_posts(pl, filter, path);
	return PARTUPDATE;
}

static int operate_posts_in_range(int choice, post_list_t *pl, post_id_t min,
		post_id_t max)
{
	post_filter_t filter = { .bid = pl->bid, .min = min, .max = max };
	int ret = PARTUPDATE;
	switch (choice) {
		case 0:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_MARKED,
					true, true);
			break;
		case 1:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_DIGEST,
					true, true);
			break;
		case 2:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_LOCKED,
					true, true);
			break;
		case 3:
			if (pl->type == POST_LIST_NORMAL) {
				post_index_board_delete(&filter, NULL, 0, true,
						!HAS_PERM(PERM_OBOARDS), false);
			}
			break;
		case 4:
			ret = tui_import_posts(pl, &filter);
			break;
		case 5:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_WATER,
					true, true);
			break;
		case 6:
			// TODO
			break;
		default:
			if (is_deleted(pl->type)) {
				post_index_board_undelete(&filter, NULL, 0,
						pl->type == POST_LIST_TRASH);
			} else {
				filter.flag |= POST_FLAG_WATER;
				post_index_board_delete(&filter, NULL, 0, true,
						!HAS_PERM(PERM_OBOARDS), false);
			}
			break;
	}

	switch (choice) {
		case 3:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEDEL, 1);
			break;
		case 4:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEANN, 1);
			break;
		default:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEOTHER, 1);
			break;
	}
	return ret;
}

static int tui_operate_posts_in_range(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;

	post_id_t min, max;
	int ret = tui_mark_range(tl, pi, &min, &max);
	if (!max)
		return ret;

	//% 恢复 : 删水文
	const char *option8 = is_deleted(pl->type)
			? "\xbb\xd6\xb8\xb4" : "\xc9\xbe\xcb\xae\xce\xc4";
	const char *options[] = {
		//% 保留 文摘 不可RE 删除 精华区 水文 转载
		"\xb1\xa3\xc1\xf4",  "\xce\xc4\xd5\xaa", "\xb2\xbb\xbf\xc9RE",
		"\xc9\xbe\xb3\xfd", "\xbe\xab\xbb\xaa\xc7\xf8", "\xcb\xae\xce\xc4",
		"\xd7\xaa\xd4\xd8", option8,
	};

	char prompt[120], ans[8];
	construct_prompt(prompt, sizeof(prompt), options, ARRAY_SIZE(options));
	getdata(-1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);

	int choice = *ans - '1';
	if (choice < 0 || choice >= ARRAY_SIZE(options))
		return MINIUPDATE;

	//% 区段[%s]操作，确定吗
	snprintf(prompt, sizeof(prompt), "\xc7\xf8\xb6\xce[%s]\xb2\xd9\xd7\xf7"
			"\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0", options[choice]);
	if (!askyn(prompt, NA, YEA))
		return MINIUPDATE;

	tl->valid = false;
	reopen_post_record(pl, pi);
	return operate_posts_in_range(choice, pl, min, max);
}

static void operate_posts_in_batch(post_list_t *pl, post_info_t *pi, int mode,
		int choice, bool first, post_id_t pid, bool quote, bool junk,
		const char *annpath, const char *utf8_keyword, const char *gbk_keyword)
{
	post_filter_t filter = { .bid = pl->bid };
	switch (mode) {
		case 1:
			filter.uid = pi->uid;
			break;
		case 2:
			strlcpy(filter.utf8_keyword, utf8_keyword,
					sizeof(filter.utf8_keyword));
			break;
		default:
			filter.tid = pi->tid;
			break;
	}
	if (!first)
		filter.min = pi->id;

	switch (choice) {
		case 0:
			if (pl->type == POST_LIST_NORMAL) {
				post_index_board_delete(&filter, NULL, 0, junk,
						!HAS_PERM(PERM_OBOARDS), false);
			}
			break;
		case 1:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_MARKED,
					true, true);
			break;
		case 2:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_DIGEST,
					true, true);
			break;
		case 3:
			import_posts(pl, &filter, annpath);
			break;
		case 4:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_WATER,
					true, true);
			break;
		case 5:
			set_post_flag(pl->record, pl->pir, &filter, POST_FLAG_LOCKED,
					true, true);
			break;
		case 6:
			// TODO: pack thread
			break;
		default:
			if (is_deleted(pl->type)) {
				post_index_board_undelete(&filter, NULL, 0,
						pl->type == POST_LIST_TRASH);
			} else {
				// TODO: merge thread
			}
			break;
	}

	switch (choice) {
		case 0:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEDEL, 1);
			break;
		case 3:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEANN, 1);
			break;
		case 6:
			bm_log(currentuser.userid, currboard, BMLOG_COMBINE, 1);
			break;
		default:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEOTHER, 1);
			break;
	}
}

static int tui_operate_posts_in_batch(tui_list_t *tl, post_info_t *pi)
{
	if (!pi || !am_curr_bm())
		return DONOTHING;

	post_list_t *pl = tl->data;
	bool deleted = is_deleted(pl->type);

	//% 相同主题 相同作者 相关主题
	const char *batch_modes[] = {
		"\xcf\xe0\xcd\xac\xd6\xf7\xcc\xe2",
		"\xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf",
		"\xcf\xe0\xb9\xd8\xd6\xf7\xcc\xe2"
	};
	//% 恢复 : 合并
	const char *option8 = deleted ? "\xbb\xd6\xb8\xb4" : "\xba\xcf\xb2\xa2";
	const char *options[] = {
		//% "删除", "保留", "文摘", "精华区", "水文", "不可RE", "合集", option8
		"\xc9\xbe\xb3\xfd", "\xb1\xa3\xc1\xf4", "\xce\xc4\xd5\xaa",
		"\xbe\xab\xbb\xaa\xc7\xf8", "\xcb\xae\xce\xc4", "\xb2\xbb\xbf\xc9RE",
		"\xba\xcf\xbc\xaf", option8
	};

	char ans[16];
	move(-1, 0);
	clrtoeol();
	ans[0] = '\0';
	//% 执行: 1) 相同主题  2) 相同作者 3) 相关主题 0) 取消
	getdata(-1, 0, "\xd6\xb4\xd0\xd0: "
			"1) \xcf\xe0\xcd\xac\xd6\xf7\xcc\xe2  "
			"2) \xcf\xe0\xcd\xac\xd7\xf7\xd5\xdf "
			"3) \xcf\xe0\xb9\xd8\xd6\xf7\xcc\xe2 "
			"0) \xc8\xa1\xcf\xfb [0]: ", ans, sizeof(ans), DOECHO, YEA);
	int mode = strtol(ans, NULL, 10) - 1;
	if (mode < 0 || mode >= ARRAY_SIZE(batch_modes))
		return MINIUPDATE;

	char prompt[120];
	construct_prompt(prompt, sizeof(prompt), options, ARRAY_SIZE(options));
	getdata(-1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);
	int choice = strtol(ans, NULL, 10) - 1;
	if (choice < 0 || choice >= ARRAY_SIZE(options))
		return MINIUPDATE;

	char buf[STRLEN];
	move(-1, 0);
	//% 确定要执行%s[%s]吗
	snprintf(buf, sizeof(buf), "\xc8\xb7\xb6\xa8\xd2\xaa\xd6\xb4\xd0\xd0"
			"%s[%s]\xc2\xf0", batch_modes[mode], options[choice]);
	if (!askyn(buf, NA, NA))
		return MINIUPDATE;

	post_id_t pid = 0;
	bool quote = true;
	if (choice == 6) {
		move(-1, 0);
		//% 制作的合集需要引言吗？
		quote = askyn("\xd6\xc6\xd7\xf7\xb5\xc4\xba\xcf\xbc\xaf"
				"\xd0\xe8\xd2\xaa\xd2\xfd\xd1\xd4\xc2\xf0\xa3\xbf", YEA, YEA);
	} else if (choice == 7) {
#if 0
		if (!deleted) {
			//% 本主题加至版面第几篇后？
			getdata(-1, 0, "\xb1\xbe\xd6\xf7\xcc\xe2\xbc\xd3\xd6\xc1"
					"\xb0\xe6\xc3\xe6\xb5\xda\xbc\xb8\xc6\xaa\xba\xf3\xa3\xbf",
					ans, sizeof(ans), DOECHO, YEA);
			pid = strtol(ans);
		}
#endif
	}

	GBK_UTF8_BUFFER(keyword, POST_TITLE_CCHARS);
	if (mode == 2) {
		//% 请输入主题关键字
		getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd6\xf7\xcc\xe2"
				"\xb9\xd8\xbc\xfc\xd7\xd6: ", gbk_keyword,
				sizeof(gbk_keyword), DOECHO, YEA);
		if (gbk_keyword[0] == '\0')
			return MINIUPDATE;
		convert_g2u(gbk_keyword, utf8_keyword);
	}

	bool junk = true;
	if (choice == 0) {
		//% 是否小d
		junk = askyn("\xca\xc7\xb7\xf1\xd0\xa1""d", YEA, YEA);
	}

	bool first = false;
	move(-1, 0);
	//% 是否从%s第一篇开始%s (Y)第一篇 (N)目前这一篇
	snprintf(buf, sizeof(buf), "\xca\xc7\xb7\xf1\xb4\xd3%s"
			"\xb5\xda\xd2\xbb\xc6\xaa\xbf\xaa\xca\xbc%s "
			"(Y)\xb5\xda\xd2\xbb\xc6\xaa "
			"(N)\xc4\xbf\xc7\xb0\xd5\xe2\xd2\xbb\xc6\xaa",
			//% 该作者 此主题
			(mode == 1) ? "\xb8\xc3\xd7\xf7\xd5\xdf"
				: "\xb4\xcb\xd6\xf7\xcc\xe2",
			options[choice]);
	first = askyn(buf, YEA, NA);

	char annpath[512];
	if (choice == 3) {
		if (DEFINE(DEF_MULTANNPATH) &&
				!set_ann_path(NULL, NULL, ANNPATH_GETMODE))
			return FULLUPDATE;

		sethomefile(annpath, currentuser.userid, ".announcepath");
		FILE *fp = fopen(annpath, "r");
		if (!fp) {
			//% 对不起, 您没有设定丝路. 请先用 f 设定丝路.
			presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xc3\xbb\xd3\xd0"
					"\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7. "
					"\xc7\xeb\xcf\xc8\xd3\xc3 f \xc9\xe8\xb6\xa8"
					"\xcb\xbf\xc2\xb7.", -1);
			return MINIUPDATE;
		}
		fscanf(fp, "%s", annpath);
		fclose(fp);
		if (!dashd(annpath)) {
			//% 您设定的丝路已丢失, 请重新用 f 设定.
			presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7"
					"\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2"
					"\xd3\xc3 f \xc9\xe8\xb6\xa8.", -1);
			return MINIUPDATE;
		}
	}

	operate_posts_in_batch(pl, pi, mode, choice, first,
			pid, quote, junk, annpath, utf8_keyword, gbk_keyword);
#if 0
	if (BMch == 7) {
		if (strneq(keyword, "Re: ", 4) || strneq(keyword, "RE: ", 4))
			//% snprintf(buf, sizeof(buf), "[合集]%s", keyword + 4);
			snprintf(buf, sizeof(buf), "[\xba\xcf\xbc\xaf]%s", keyword + 4);
		else
			//% snprintf(buf, sizeof(buf), "[合集]%s", keyword);
			snprintf(buf, sizeof(buf), "[\xba\xcf\xbc\xaf]%s", keyword);

		ansi_filter(keyword, buf);

		sprintf(buf, "tmp/%s.combine", currentuser.userid);

		Postfile(buf, currboard, keyword, 2);
		unlink(buf);
	}
#endif
	tl->valid = false;
	return PARTUPDATE;
}

extern int tui_select_board(int);

static int switch_board(tui_list_t *tl)
{
	post_list_t *pl = tl->data;
	if (pl->type != POST_LIST_NORMAL || !pl->bid)
		return DONOTHING;

	int bid = tui_select_board(pl->bid);
	if (bid) {
		tl->valid = false;
		pl->bid = bid;
		pl->plp = NULL;
		record_close(pl->record);
		post_index_board_open(bid, RECORD_READ, pl->record);
		record_close(pl->record_sticky);
		post_index_board_open_sticky(bid, RECORD_READ, pl->record_sticky);
	}
	return FULLUPDATE;
}
#if 0
static int switch_archive(post_list_t *l, bool upward)
{
	return READ_AGAIN;
}

static int tui_jump_to_id(slide_list_t *p)
{
	post_info_t *ip = get_post_info(p);
	if (!ip)
		return DONOTHING;

	char buf[16];
	getdata(-1, 0, "Post ID", buf, sizeof(buf), true, true);

	post_id_t pid = strtoll(buf, NULL, 10);
	if (!pid || pid == ip->id)
		return MINIUPDATE;

	post_list_t *l = p->data;
	post_filter_t filter = l->filter;
	bool upward = pid < ip->id;
	if (upward) {
		filter.min = 0;
		filter.max = pid;
	} else {
		filter.min = pid;
		filter.max = 0;
	}
	return relocate_to_filter(p, &filter, upward);
}

static int tui_jump(slide_list_t *p)
{
	post_list_t *l = p->data;
	if (l->filter.type == POST_LIST_THREAD)
		return DONOTHING;

	char buf[2];
	//% getdata(-1, 0, "跳转到 (P)文章 (A)存档 (C)取消？[C]",
	getdata(-1, 0, "\xcc\xf8\xd7\xaa\xb5\xbd (P)\xce\xc4\xd5\xc2 (A)\xb4\xe6\xb5\xb5 (C)\xc8\xa1\xcf\xfb\xa3\xbf[C]",
			buf, sizeof(buf), true, true);
	char c = tolower(buf[0]);
	if (c == 'p')
		return tui_jump_to_id(p);
	if (c == 'a') {
		l->filter.archive = true;
		l->abase = SLIDE_LIST_BOTTOMUP;
		l->reload = true;
		return FULLUPDATE;
	}
	return MINIUPDATE;
}
#endif

static int tui_reorder_sticky_posts(tui_list_t *tl, post_info_t *pi)
{
	post_list_t *pl = tl->data;
	if (pl->type != POST_LIST_NORMAL || !(pi->flag & POST_FLAG_STICKY)
			|| !am_curr_bm()) {
		return DONOTHING;
	}

	if (reorder_sticky_posts(pl->bid, pi->id))
		tl->valid = false;
	return PARTUPDATE;
}

extern int show_online(void);
extern int thesis_mode(void);
extern int deny_user(void);
extern int club_user(void);
extern int tui_send_msg(const char *);
extern int x_lockscreen(void);
extern int vote_results(const char *bname);
extern int b_vote(void);
extern int vote_maintain(const char *bname);
extern int b_notes_edit(void);
extern int b_notes_passwd(void);
extern int mainreadhelp(void);
extern int show_b_note(void);
extern int show_b_secnote(void);
extern int into_announce(void);
extern int show_hotspot(void);
extern int tui_follow_uname(const char *uname);
extern int tui_attachment_list(const board_t *board);

static tui_list_handler_t post_list_handler(tui_list_t *tl, int ch)
{
	post_list_t *pl = tl->data;
	post_info_t *pi = tl->cur >= tl->all ? NULL
			: pl->buf + tl->cur - tl->begin;

	switch (ch) {
		case Ctrl('P'):
			tl->valid = false;
			return tui_new_post(pl->bid, NULL);
		case '@':
			show_online();
			set_user_status(ST_READING);
			return FULLUPDATE;
		case '.':
			return post_list_deleted(tl, POST_INDEX_TRASH);
		case 'J':
			return post_list_deleted(tl, POST_INDEX_JUNK);
		case ',':
			return tui_attachment_list(currbp);
		case 't':
			return thesis_mode();
		case '!':
			return Goodbye();
		case 'S':
			s_msg();
			return FULLUPDATE;
		case 'o':
			show_online_followings();
			return FULLUPDATE;
		case 'u':
			t_query(NULL);
			return FULLUPDATE;
		case '|':
			return x_lockscreen();
		case 'R':
			return vote_results(currboard);
		case 'v':
			return b_vote();
		case 'V':
			return vote_maintain(currboard);
		case KEY_TAB:
			return show_b_note();
		case 'z':
			return show_b_secnote();
		case 'W':
			return b_notes_edit();
		case Ctrl('W'):
			return b_notes_passwd();
		case 'h':
			return mainreadhelp();
		case Ctrl('D'):
			return deny_user();
		case Ctrl('K'):
			return club_user();
		case 'x':
			if (into_announce() != DONOTHING)
				return FULLUPDATE;
		case 's':
			return switch_board(tl);
#if 0
		case '%':
			return tui_jump(p);
#endif
		case 'q': case 'e': case KEY_LEFT: case EOF:
			return READ_AGAIN;
		case Ctrl('L'):
			redoscr();
			return DONOTHING;
		case 'M':
			m_new();
			return FULLUPDATE;
		case 'H':
			return show_hotspot();
		case 'l':
			msg_more();
			return FULLUPDATE;
		default:
			if (!pi)
				return READ_AGAIN;
	}

	switch (ch) {
		case '\n': case '\r':
		case KEY_RIGHT: case 'r': case Ctrl('S'): case 'p':
			if (tl->jump) {
				tl->cur = tl->jump - 1;
				tl->jump = 0;
				tl->valid = false;
				return PARTUPDATE;
			}
			return read_posts(tl, pi, false, false);
		case Ctrl('U'):
			return read_posts(tl, pi, false, true);
		case '_':
			return toggle_post_lock(tl, pi);
		case '#':
			return toggle_post_stickiness(tl, pi);
		case ';':
			return tui_reorder_sticky_posts(tl, pi);
		case 'm':
			return toggle_post_flag(tl, pi, POST_FLAG_MARKED);
		case 'g':
			return toggle_post_flag(tl, pi, POST_FLAG_DIGEST);
		case 'w':
			return toggle_post_flag(tl, pi, POST_FLAG_WATER);
		case 'T':
			return tui_edit_post_title(tl, pi);
		case 'E':
			return tui_edit_post_content(pi);
		case 'i':
			return tui_save_post(pi);
		case 'I':
			return tui_import_post(pi);
		case 'D':
			return tui_delete_posts_in_range(tl, pi);
		case 'L':
			return tui_operate_posts_in_range(tl, pi);
		case 'b':
			return tui_operate_posts_in_batch(tl, pi);
#if 0
		case 'C':
			return tui_count_posts_in_range(p);
#endif
		case Ctrl('G'): case Ctrl('T'): case '`':
			return tui_post_list_selected(tl, pi);
		case 'a':
			return tui_search_author(tl, pi, false);
		case 'A':
			return tui_search_author(tl, pi, true);
		case '/':
			return tui_search_title(tl, false);
		case '?':
			return tui_search_title(tl, true);
		case '=':
			return jump_to_thread_first(tl, pi);
		case '[':
			return jump_to_thread_prev(tl, pi);
		case ']':
			return jump_to_thread_next(tl, pi);
		case 'n': case Ctrl('N'):
			return jump_to_thread_first_unread(tl, pi);
		case '\\':
			return jump_to_thread_last(tl, pi);
		case 'K':
			return skip_post(tl, pi);
		case 'c':
			brc_clear(pi->stamp);
			return PARTUPDATE;
		case 'f':
			brc_clear_all();
			return PARTUPDATE;
		case 'd':
			return tui_delete_single_post(tl, pi, pl->bid);
		case 'Y':
			return tui_undelete_single_post(tl, pi);
		case '*':
			return show_post_info(pi);
		case Ctrl('C'):
			return tui_cross_post(pi);
		case 'F':
			return forward_post(pi, false);
		case 'U':
			return forward_post(pi, true);
		case Ctrl('R'):
			return reply_with_mail(pi);
		case 'Z':
			clear();
			tui_send_msg(pi->owner);
			return FULLUPDATE;
		case Ctrl('A'):
			return pi ? t_query(pi->owner) : DONOTHING;
		case 'P': case Ctrl('B'): case KEY_PGUP:
			return tui_list_seek(tl, KEY_PGUP, true, true);
		case 'k': case KEY_UP:
			return tui_list_seek(tl, KEY_UP, true, true);
		case 'N': case Ctrl('F'): case KEY_PGDN: case ' ':
			return tui_list_seek(tl, KEY_PGDN, true, true);
		case 'j': case KEY_DOWN:
			return tui_list_seek(tl, KEY_DOWN, true, true);
		case '$': case KEY_END:
			if (pi->flag & POST_FLAG_STICKY)
				tl->cur = pl->record_count - 1;
			else
				tl->cur = tl->all - 1;
			if (tl->cur < 0)
				tl->cur = 0;
			if (tl->begin + tl->lines <= tl->cur) {
				tl->begin = tl->cur - tl->lines + 1;
				if (tl->begin < 0)
					tl->begin = 0;
			}
			tl->valid = false;
			return PARTUPDATE;
		case KEY_HOME:
			tl->begin = tl->cur = 0;
			tl->valid = false;
			return PARTUPDATE;
		case 'O':
			return pi->uid ? tui_follow_uname(pi->owner) : DONOTHING;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			{
				tl->jump = tl->jump * 10 + (ch - '0');
				move(-1, 69);
				clrtoeol();
				char buf[24];
				snprintf(buf, sizeof(buf), "\033[1;44;33m[%6d]\033[m",
						tl->jump);
				outs(buf);
			}
			return DONOTHING;
		default:
			return READ_AGAIN;
	}
}

static void open_post_record(const post_filter_t *filter, record_t *record)
{
	if (filter->bid) {
		switch (filter->type) {
			case POST_LIST_NORMAL:
				post_index_board_open(filter->bid, RECORD_READ, record);
				break;
			case POST_LIST_TRASH:
				post_index_trash_open(filter->bid, POST_INDEX_TRASH, record);
				break;
			case POST_LIST_JUNK:
				post_index_trash_open(filter->bid, POST_INDEX_JUNK, record);
				break;
			case POST_LIST_DIGEST: case POST_LIST_THREAD:
			case POST_LIST_MARKED: case POST_LIST_TOPIC:
			case POST_LIST_AUTHOR: case POST_LIST_KEYWORD: {
				char file[HOMELEN];
				filtered_record_name(file, sizeof(file));
				filtered_record_open(filter, RECORD_READ, file, sizeof(file),
						record);
				unlink(file);
				break;
			}
			default:
				break;
		}
	};
}

static int post_list_with_filter(const post_filter_t *filter)
{
	int lines = t_lines - 4;

	post_info_t *buf = malloc(lines * sizeof(*buf));

	record_t record, record_sticky;
	open_post_record(filter, &record);

	bool sticky = filter->type == POST_LIST_NORMAL;
	if (sticky)
		post_index_board_open_sticky(filter->bid, RECORD_READ, &record_sticky);

	post_index_record_t pir;
	post_index_record_open(&pir);

	post_list_t pl = {
		.record = &record,
		.record_sticky = sticky ? &record_sticky : NULL,
		.pir = &pir,
		.buf = buf,
		.type = filter->type,
		.bid = filter->bid,
	};

	tui_list_t tl = {
		.lines = lines,
		.data = &pl,
		.loader = post_list_loader,
		.title = post_list_title,
		.display = post_list_display,
		.handler = post_list_handler,
	};

	int status = session.status;
	set_user_status(ST_READING);
	tui_list(&tl);
	set_user_status(status);

	record_close(&record);
	if (sticky)
		record_close(&record_sticky);
	post_index_record_close(&pir);
	free(buf);
	return 0;
}

int post_list_board(int bid)
{
	post_filter_t filter = { .type = POST_LIST_NORMAL, .bid = bid };
	return post_list_with_filter(&filter);
}
