#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct {
	post_filter_t filter;

	bool relocate;
	bool reload;
	bool sreload;
	int last_query_rows;

	post_info_t **index;
	post_info_t *posts;
	post_info_t *sposts;
	int icount;
	int count;
	int scount;
} post_list_t;

static bool is_asc(slide_list_base_e base)
{
	return (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_NEXT);
}

static void res_to_array(db_res_t *r, post_list_t *l, slide_list_base_e base,
		int size)
{
	int rows = db_res_rows(r);
	if (rows < 1)
		return;

	bool asc = is_asc(base);

	if (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_BOTTOMUP
			|| rows >= size) {
		rows = rows > size ? size : rows;
		for (int i = 0; i < rows; ++i)
			res_to_post_info(r, asc ? i : rows - i - 1, l->posts + i);
		l->count = rows;
	} else {
		int extra = l->count + rows - size;
		int left = l->count - (extra > 0 ? extra : 0);
		if (asc) {
			if (extra > 0) {
				memmove(l->posts, l->posts + extra,
						sizeof(*l->posts) * (l->count - extra));
			}
			for (int i = 0; i < rows; ++i)
				res_to_post_info(r, i, l->posts + left + i);
		} else {
			memmove(l->posts + rows, l->posts,
					sizeof(*l->posts) * (l->count - left));
			for (int i = 0; i < rows; ++i)
				res_to_post_info(r, rows - i - 1, l->posts + i);
		}
		l->count = left + rows;
	}
}

static void load_sticky_posts(post_list_t *l)
{
	if (l->filter.type == POST_LIST_NORMAL && !l->sposts && l->sreload) {
		l->scount = _load_sticky_posts(l->filter.bid, &l->sposts);
		l->sreload = false;
	}
}

static void index_posts(post_list_t *l, int limit, slide_list_base_e base)
{
	if (base != SLIDE_LIST_CURRENT) {
		int remain = limit, start = 0;
		if (base == SLIDE_LIST_BOTTOMUP)
			start = l->scount;
		if (base == SLIDE_LIST_NEXT)
			start = l->count - l->last_query_rows;
		if (start < 0)
			start = 0;

		l->icount = 0;
		for (int i = start; i < l->count && i < limit; ++i) {
			l->index[l->icount++] = l->posts + i;
			--remain;
		}

		if (l->sposts) {
			for (int i = 0; i < remain && i < l->scount; ++i) {
				l->index[l->icount++] = l->sposts + i;
			}
		}
	}
}

static slide_list_loader_t post_list_loader(slide_list_t *p)
{
	post_list_t *l = p->data;

	if (l->reload)
		p->base = SLIDE_LIST_NEXT;
	if (p->base == SLIDE_LIST_CURRENT)
		return 0;

	int page = t_lines - 4;
	if (!l->index) {
		l->index = malloc(sizeof(*l->index) * page);
		l->posts = malloc(sizeof(*l->posts) * page);
		l->sposts = NULL;
		l->icount = l->count = l->scount = 0;
	}

	bool asc = is_asc(p->base);
	query_builder_t *b = build_post_query(&l->filter, asc, page);
	db_res_t *res = b->query(b);
	query_builder_free(b);
	res_to_array(res, l, p->base, page);
	l->last_query_rows = db_res_rows(res);

	if ((p->base == SLIDE_LIST_NEXT && l->last_query_rows < page)
			|| p->base == SLIDE_LIST_BOTTOMUP) {
		load_sticky_posts(l);
	}
	db_clear(res);

	if (l->last_query_rows) {
		if (p->update != FULLUPDATE)
			p->update = PARTUPDATE;
	} else {
		p->cur = p->base == SLIDE_LIST_PREV ? 0 : page - 1;
	}

	index_posts(l, page, p->base);
	l->reload = false;
	return 0;
}

static slide_list_title_t post_list_title(slide_list_t *p)
{
	return;
}

static void post_list_display_entry(post_info_t *p)
{
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(p->utf8_title, gbk_title);

	char id_str[24];
	if (p->flag & POST_FLAG_STICKY)
		strlcpy(id_str, " \033[1;31m[∞]\033[m ", sizeof(id_str));
	else
		pid_to_base32(p->id, id_str, 7);

	prints(" %s %s %s\n", id_str, p->owner, gbk_title);
}

static slide_list_display_t post_list_display(slide_list_t *p)
{
	post_list_t *l = p->data;
	for (int i = 0; i < l->icount; ++i) {
		post_list_display_entry(l->index[i]);
	}
	return 0;
}

static int toggle_post_lock(int bid, post_info_t *ip)
{
	bool deleted = ip->flag & POST_FLAG_DELETED;
	bool locked = ip->flag & POST_FLAG_LOCKED;
	if (am_curr_bm() || (session.id == ip->uid && !locked)) {
		post_filter_t filter = { .bid = bid, .min = ip->id, .max = ip->id };
		if (set_post_flag(&filter, "locked", deleted, !locked, false)) {
			set_post_flag_local(ip, POST_FLAG_LOCKED, !locked);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int toggle_post_stickiness(int bid, post_info_t *ip, post_list_t *l)
{
	if (is_deleted(l->filter.type))
		return DONOTHING;

	bool sticky = ip->flag & POST_FLAG_STICKY;
	if (am_curr_bm() && sticky_post_unchecked(bid, ip->id, !sticky)) {
		l->sreload = l->reload = true;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int toggle_post_flag(int bid, post_info_t *ip, post_flag_e flag,
		const char *field)
{
	bool deleted = ip->flag & POST_FLAG_DELETED;
	bool set = ip->flag & flag;
	if (am_curr_bm()) {
		post_filter_t filter = { .bid = bid, .min = ip->id, .max = ip->id };
		if (set_post_flag(&filter, field, deleted, !set, false)) {
			set_post_flag_local(ip, flag, !set);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, user_id_t uid, const char *keyword);

static int post_list_deleted(int bid, post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !am_curr_bm())
		return DONOTHING;
	post_list(bid, POST_LIST_TRASH, 0, SLIDE_LIST_BOTTOMUP, 0, NULL);
	return FULLUPDATE;
}

static int post_list_admin_deleted(int bid, post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !HAS_PERM(PERM_OBOARDS))
		return DONOTHING;
	post_list(bid, POST_LIST_JUNK, 0, SLIDE_LIST_BOTTOMUP, 0, NULL);
	return FULLUPDATE;
}

static int post_list_with_filter(slide_list_base_e base,
		post_filter_t *filter);

static int tui_post_list_selected(post_list_t *l, post_info_t *ip)
{
	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	char ans[3];
	getdata(t_lines - 1, 0, "切换模式到: 1)文摘 2)同主题 3)被 m 文章 4)原作"
			" 5)同作者 6)标题关键字 [1]: ", ans, sizeof(ans), DOECHO, YEA);

	int c = ans[0];
	if (!c)
		c = '1';
	c -= '1';

	const post_list_type_e types[] = {
		POST_LIST_DIGEST, POST_LIST_THREAD, POST_LIST_MARKED,
		POST_LIST_TOPIC, POST_LIST_AUTHOR, POST_LIST_KEYWORD,
	};
	if (c < 0 || c >= NELEMS(types))
		return MINIUPDATE;

	post_filter_t filter = { .bid = l->filter.bid, .type = types[c] };
	switch (filter.type) {
		case POST_LIST_DIGEST:
			filter.flag &= POST_FLAG_DIGEST;
			break;
		case POST_LIST_MARKED:
			filter.flag &= POST_FLAG_MARKED;
		case POST_LIST_AUTHOR: {
				char uname[IDLEN + 1];
				strlcpy(uname, ip->owner, sizeof(uname));
				getdata(t_lines - 1, 0, "您想查找哪位网友的文章? ", uname,
						sizeof(uname), DOECHO, YEA);
				user_id_t uid = get_user_id(uname);
				if (!uid)
					return MINIUPDATE;
				filter.uid = uid;
			}
			break;
		case POST_LIST_KEYWORD: {
				GBK_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
				getdata(t_lines - 1, 0, "您想查找的文章标题关键字: ",
						gbk_keyword, sizeof(gbk_keyword), DOECHO, YEA);
				convert_g2u(gbk_keyword, filter.utf8_keyword);
				if (!filter.utf8_keyword[0])
					return MINIUPDATE;
			}
			break;
		default:
			break;
	}

	post_list_with_filter(SLIDE_LIST_BOTTOMUP, &filter);
	l->reload = true;
	return FULLUPDATE;
}

static bool match_filter(post_filter_t *filter, post_info_t *p)
{
	bool match = true;
	if (filter->uid)
		match &= p->uid == filter->uid;
	if (filter->min)
		match &= p->id >= filter->min;
	if (filter->max)
		match &= p->id <= filter->max;
	if (filter->tid)
		match &= p->id == filter->tid;
	return match;
}

static int relocate_to_filter(slide_list_t *p, post_filter_t *filter,
		bool upward)
{
	post_list_t *l = p->data;

	int found = -1;
	if (upward) {
		for (int i = p->cur - 1; i >= 0; --i) {
			if (match_filter(filter, l->index[i])) {
				found = i;
				break;
			}
		}
	} else {
		for (int i = p->cur + 1; i < l->icount; ++i) {
			if (match_filter(filter, l->index[i])) {
				found = i;
				break;
			}
		}
	}

	if (found >= 0) {
		p->cur = found;
		return MINIUPDATE;
	} else {
		// TODO
		return PARTUPDATE;
	}
}

static int relocate_to_author(slide_list_t *p, user_id_t uid, bool upward)
{
	post_list_t *l = p->data;
	post_filter_t filter = { .bid = l->filter.bid, .uid = uid };
	return relocate_to_filter(p, &filter, upward);
}

static int tui_search_author(slide_list_t *p, bool upward)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	char prompt[80];
	snprintf(prompt, sizeof(prompt), "向%s搜索作者 [%s]: ",
			upward ? "上" : "下", ip->owner);
	char ans[IDLEN + 1];
	getdata(t_lines - 1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);

	user_id_t uid = ip->uid;
	if (*ans && !streq(ans, ip->owner))
		uid = get_user_id(ans);

	return relocate_to_author(p, uid, upward);
}

static int tui_search_title(slide_list_t *p, bool upward)
{
	post_list_t *l = p->data;

	char prompt[80];
	static GBK_BUFFER(title, POST_TITLE_CCHARS) = "";
	snprintf(prompt, sizeof(prompt), "向%s搜索标题[%s]: ",
			upward ? "上" : "下", gbk_title);

	GBK_BUFFER(ans, POST_TITLE_CCHARS);
	getdata(t_lines - 1, 0, prompt, gbk_ans, sizeof(gbk_ans), DOECHO, YEA);

	if (*gbk_ans != '\0')
		strlcpy(gbk_title, gbk_ans, sizeof(gbk_title));

	if (!*gbk_title != '\0')
		return MINIUPDATE;

	post_filter_t filter = { .type = l->filter.type, .bid = l->filter.bid };
	convert_g2u(gbk_title, filter.utf8_keyword);
	return relocate_to_filter(p, &filter, upward);
}

static int jump_to_thread_first(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];
	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .max = ip->tid
	};
	return relocate_to_filter(p, &filter, true);
}

static int jump_to_thread_prev(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	if (ip->id == ip->tid)
		return DONOTHING;

	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .max = ip->id - 1,
	};
	return relocate_to_filter(p, &filter, true);
}

static int jump_to_thread_next(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];
	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .min = ip->id + 1,
	};
	return relocate_to_filter(p, &filter, false);
}

static int read_posts(post_list_t *l, post_info_t *ip, bool thread, bool user);

static int jump_to_thread_first_unread(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .min = brc_first_unread()
	};

	const int limit = 40;
	bool end = false;
	while (!end) {
		query_builder_t *b = build_post_query(&filter, true, limit);
		db_res_t *res = b->query(b);
		query_builder_free(b);

		int rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			post_id_t id = db_get_post_id(res, i, 0);
			if (brc_unread(id)) {
				post_info_t info;
				res_to_post_info(res, i, &info);
				read_posts(l, &info, true, false);
				db_clear(res);
				return FULLUPDATE;
			}
		}
		if (rows < limit)
			end = true;
		if (rows > 0)
			filter.min = db_get_post_id(res, rows - 1, 0) + 1;
		db_clear(res);
	}
	return PARTUPDATE;
}

static int jump_to_thread_last(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .min = ip->id + 1,
	};

	query_builder_t *b = build_post_query(&filter, false, 1);
	db_res_t *res = b->query(b);
	if (res && db_res_rows(res) > 0) {
		post_info_t info;
		res_to_post_info(res, 0, &info);
		read_posts(l, &info, true, false);
		db_clear(res);
		return FULLUPDATE;
	}
	return DONOTHING;
}

static int skip_post(slide_list_t *p, post_id_t pid)
{
	brc_mark_as_read(pid);
	if (++p->cur >= t_lines - 4) {
		p->base = SLIDE_LIST_NEXT;
		p->cur = 0;
	}
	return DONOTHING;
}

static int tui_delete_single_post(post_list_t *p, post_info_t *ip)
{
	if (ip->uid == session.uid || am_curr_bm()) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (delete_posts(&f, true, false, true)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int tui_undelete_single_post(post_list_t *p, post_info_t *ip)
{
	if (is_deleted(p->filter.type)) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (undelete_posts(&f, p->filter.type == POST_LIST_TRASH)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static bool dump_content(const post_filter_t *fp, const post_info_t *ip,
		char *file, size_t size)
{
	post_filter_t filter = *fp;
	filter.min = ip->id;

	db_res_t *res = query_post_by_pid(&filter, "content");
	if (!res || db_res_rows(res) < 1)
		return false;

	int ret = dump_content_to_gbk_file(db_get_value(res, 0, 0),
			db_get_length(res, 0, 0), file, sizeof(file));

	db_clear(res);
	return ret == 0;
}

static int forward_post(const post_filter_t *fp, const post_info_t *ip,
		bool uuencode)
{
	char file[HOMELEN];
	if (!dump_content(fp, ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	tui_forward(file, gbk_title, uuencode);

	unlink(file);
	return FULLUPDATE;
}

static int reply_with_mail(const post_filter_t *fp, const post_info_t *ip)
{
	char file[HOMELEN];
	if (!dump_content(fp, ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	post_reply(ip->owner, gbk_title, file);

	unlink(file);
	return FULLUPDATE;
}

static int tui_edit_post_title(post_info_t *ip)
{
	if (ip->uid != session.uid && !am_curr_bm())
		return DONOTHING;

	GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);

	ansi_filter(utf8_title, ip->utf8_title);
	convert_u2g(utf8_title, gbk_title);

	getdata(t_lines - 1, 0, "新文章标题: ", gbk_title, sizeof(gbk_title),
			DOECHO, NA);

	check_title(gbk_title, sizeof(gbk_title));
	convert_g2u(gbk_title, utf8_title);

	if (!*utf8_title || streq(utf8_title, ip->utf8_title))
		return MINIUPDATE;

	if (alter_title(ip->id, ip->flag & POST_FLAG_DELETED, utf8_title)) {
		strlcpy(ip->utf8_title, utf8_title, sizeof(ip->utf8_title));
		return PARTUPDATE;
	}
	return MINIUPDATE;
}

static int tui_edit_post_content(const post_filter_t *fp, post_info_t *ip)
{
	if (ip->uid != session.uid && !am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(fp, ip, file, sizeof(file)))
		return DONOTHING;

	set_user_status(ST_EDIT);

	clear();
	if (vedit(file, NA, NA) != -1) {
		char *content = convert_file_to_utf8_content(file);
		if (content) {
			if (alter_content(ip->id, ip->flag & POST_FLAG_DELETED, content)) {
				char buf[STRLEN];
				snprintf(buf, sizeof(buf), "edited post #%"PRIdPID, ip->id);
				report(buf, currentuser.userid);
			}
			free(content);
		}
	}

	unlink(file);
	return FULLUPDATE;
}

extern int show_board_notes(const char *bname, int command);
extern int noreply;

static int tui_new_post(post_filter_t *fp, post_info_t *ip)
{
	time_t now = time(NULL);
	if (now - get_last_post_time() < 3) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("您太辛苦了，先喝杯咖啡歇会儿，3 秒钟后再发表文章。\n");
		pressreturn();
		return MINIUPDATE;
	}

	board_t board;
	if (!get_board_by_bid(fp->bid, &board) ||
			!has_post_perm(&currentuser, &board)) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("此讨论区是唯读的, 或是您尚无权限在此发表文章。");
		pressreturn();
		return FULLUPDATE;
	}

	set_user_status(ST_POSTING);

	clear();
	show_board_notes(board.name, 1);

	struct postheader header;
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (ip) {
		header.reply = true;
		if (strncaseeq(ip->utf8_title, "Re: ", 4)) {
			convert_u2g(ip->utf8_title, header.title);
		} else {
			convert_u2g(ip->utf8_title, gbk_title);
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
			ansi_filter(header.title, header.title);
//			strlcpy(postfile.title, header.title, sizeof(postfile.title));
		}
	} else {
		return FULLUPDATE;
	}

	now = time(NULL);
	set_last_post_time(now);

	in_mail = NA;

	char file[HOMELEN];
	snprintf(file, sizeof(file), "tmp/editbuf.%d", getpid());
	if (ip) {
		char orig[HOMELEN];
		dump_content(fp, ip, orig, sizeof(orig));
		do_quote(orig, file, header.include_mode);
		unlink(orig);
	}

	if (vedit(file, YEA, YEA) == -1) {
		unlink(file);
		clear();
		return FULLUPDATE;
	}

	// header.chk_anony
	// save_title
	// valid_title()

	if (header.mail_owner) {
		if (header.anonymous) {
			prints("对不起，您不能在匿名版使用寄信给原作者功能。");
		} else {
			if (!is_blocked(ip->owner)
					&& !mail_file(file, ip->owner, gbk_title)) {
				prints("信件已成功地寄给原作者 %s", ip->owner);
			}
			else {
				prints("信件邮寄失败，%s 无法收信。", ip->owner);
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
		.title = gbk_title,
		.content = NULL,
		.gbk_file = file,
		.sig = 0,
		.ip = NULL,
		.reid = ip ? ip->id : 0,
		.tid = ip ? ip->tid : 0,
		.locked = header.locked,
		.marked = false,
		.anony = header.anonymous,
		.cp = NULL,
	};

	post_id_t pid = publish_post(&req);
	if (pid) {
		brc_mark_as_read(pid);

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
	return FULLUPDATE;
}

static int tui_save_post(const post_filter_t *fp, const post_info_t *ip)
{
	if (!am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(fp, ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	a_Save(gbk_title, file, false, true);

	return MINIUPDATE;
}

static int tui_import_post(const post_filter_t *fp, const post_info_t *ip)
{
	if (!HAS_PERM(PERM_BOARDS))
		return DONOTHING;

	if (DEFINE(DEF_MULTANNPATH)
			&& set_ann_path(NULL, NULL, ANNPATH_GETMODE) == 0)
		return FULLUPDATE;

	char file[HOMELEN];
	if (dump_content(fp, ip, file, sizeof(file))) {
		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(ip->utf8_title, gbk_title);

		a_Import(gbk_title, file, NA);
	}

	if (DEFINE(DEF_MULTANNPATH))
		return FULLUPDATE;
	return DONOTHING;
}

static bool tui_input_range(post_id_t *min, post_id_t *max)
{
	char num[8];
	getdata(t_lines - 1, 0, "首篇文章编号: ", num, sizeof(num), DOECHO, YEA);
	*min = base32_to_pid(num);
	if (!*min) {
		move(t_lines - 1, 50);
		prints("错误编号...");
		egetch();
		return false;
	}

	getdata(t_lines - 1, 25, "末篇文章编号: ", num, sizeof(num), DOECHO, YEA);
	*max = base32_to_pid(num);
	if (*max < *min) {
		move(t_lines - 1, 50);
		prints("错误区间...");
		egetch();
		return false;
	}
	return true;
}

static int tui_delete_posts_in_range(slide_list_t *p)
{
	if (!am_curr_bm())
		return DONOTHING;

	post_list_t *l = p->data;
	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	post_id_t min = 0, max = 0;
	if (!tui_input_range(&min, &max))
		return MINIUPDATE;

	move(t_lines - 1, 50);
	if (askyn("确定删除", NA, NA)) {
		post_filter_t filter = {
			.bid = l->filter.bid, .min = min, .max = max
		};
		if (delete_posts(&filter, false, !HAS_PERM(PERM_OBOARDS), false)) {
			bm_log(currentuser.userid, currboard, BMLOG_DELETE, 1);
			l->reload = true;
		}
		return PARTUPDATE;
	}
	move(t_lines - 1, 50);
	clrtoeol();
	prints("放弃删除...");
	egetch();
	return MINIUPDATE;
}

static bool count_posts_in_range(post_id_t min, post_id_t max, bool asc,
		int sort, int least, char *file, size_t size)
{
	query_builder_t *b = query_builder_new(0);
	b->append(b, "SELECT");
	b->append(b, "uname, COUNT(*) AS a, SUM(marked::integer) AS m");
	b->append(b, ", SUM(digest::integer) AS g, SUM(water::integer) AS w, ");
	b->append(b, "SUM((NOT marked AND NOT digest AND NOT water)::integer) as n");
	b->sappend(b, "FROM", "posts");
	b->sappend(b, "WHERE", "id >= %"DBIdPID, min);
	b->sappend(b, "AND", "id <= %"DBIdPID, max);
	b->sappend(b, "GROUP BY", "uname");

	const char *field[] = { "a", "m", "g", "w", "n" };
	b->sappend(b, "ORDER BY", field[sort]);
	b->append(b, asc ? "ASC" : "DESC");

	db_res_t *res = b->query(b);
	query_builder_free(b);

	if (res) {
		snprintf(file, sizeof(file), "tmp/count.%d", getpid());
		FILE *fp = fopen(file, "w");
		if (fp) {
			fprintf(fp, "版    面: \033[1;33m%s\033[m\n", currboard);

			int count = 0;
			for (int i = db_res_rows(res) - 1; i >=0; --i) {
				count += db_get_bigint(res, i, 1);
			}
			char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
			fprintf(fp, "有效篇数: \033[1;33m%d\033[m 篇"
					" [\033[1;33m%s-%s\033[m]\n", count,
					pid_to_base32(min, min_str, sizeof(min_str)),
					pid_to_base32(max, max_str, sizeof(max_str)));
			
			const char *descr[] = {
				"总数", "被m的", "被g的", "被w的", "无标记"
			};
			fprintf(fp, "排序方式: \033[1;33m按%s%s\033[m\n", descr[sort],
					asc ? "升序" : "降序");
			fprintf(fp, "文章数下限: \033[1;33m%d\033[m\n\n", least);
			fprintf(fp, "\033[1;44;37m 使用者代号  │总  数│ 被M的│ 被G的"
					"│ 被w的│无标记 \033[m\n");

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

static int tui_count_posts_in_range(post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !am_curr_bm())
		return DONOTHING;

	post_id_t min = 0, max = 0;
	if (!tui_input_range(&min, &max))
		return MINIUPDATE;

	char num[8];
	getdata(t_lines - 1, 0, "排序方式 (0)降序 (1)升序 ? : ", num, 2, DOECHO, YEA);
	bool asc = (num[0] == '1');

	getdata(t_lines - 1, 0, "排序选项 (0)总数 (1)被m (2)被g (3)被w (4)无标记 ? : ",
			num, 2, DOECHO, YEA);
	int sort = strtol(num, NULL, 10);
	if (sort < 0 || sort > 4)
		sort = 0;

	getdata(t_lines - 1, 0, "文章数下限(默认0): ", num, 6, DOECHO, YEA);
	int least = strtol(num, NULL, 10);

	char file[HOMELEN];
	if (!count_posts_in_range(min, max, asc, sort, least, file, sizeof(file)))
		return MINIUPDATE;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
	snprintf(gbk_title, sizeof(gbk_title), "[%s]统计文章数(%s-%s)",
			currboard, pid_to_base32(min, min_str, sizeof(min_str)),
			pid_to_base32(max, max_str, sizeof(max_str)));
	mail_file(file, currentuser.userid, gbk_title);
	unlink(file);

	return MINIUPDATE;
}

#if 0
		struct stat filestat;
		int i, len;
		char tmp[1024];

		sprintf(genbuf, "upload/%s/%s", currboard, fileinfo->filename);
		if (stat(genbuf, &filestat) < 0) {
			clear();
			move(10, 30);
			prints("对不起，%s 不存在！\n", genbuf);
			pressanykey();
			clear();
			return FULLUPDATE;
		}

		clear();
		prints("文件详细信息\n\n");
		prints("版    名:     %s\n", currboard);
		prints("序    号:     第 %d 篇\n", ent);
		prints("文 件 名:     %s\n", fileinfo->filename);
		prints("上 传 者:     %s\n", fileinfo->owner);
		prints("上传日期:     %s\n", getdatestring(fileinfo->timeDeleted, DATE_ZH));
		prints("文件大小:     %d 字节\n", filestat.st_size);
		prints("文件说明:     %s\n", fileinfo->title);
		prints("URL 地址:\n");
		sprintf(tmp, "http://%s/upload/%s/%s", BBSHOST, currboard,
				fileinfo->filename);
		strtourl(genbuf, tmp);
		len = strlen(genbuf);
		clrtoeol();
		for (i = 0; i < len; i += 78) {
			strlcpy(tmp, genbuf + i, 78);
			tmp[78] = '\n';
			tmp[79] = '\0';
			outs(tmp);
		}
		if (!(ch == KEY_UP || ch == KEY_PGUP))
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
				if (DEFINE(DEF_THESIS)) {
					sread(0, 0, fileinfo);
					break;
				} else
					return READ_NEXT;
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
#endif

#if 0
	switch (ch) {
		case '*':
			show_file_info(ent, fileinfo, direct);
			break;
		case ' ':
		case 'j':
		case KEY_RIGHT:
			if (DEFINE(DEF_THESIS)) {
				sread(0, 0, fileinfo);
				break;
			} else
				return READ_NEXT;
		case KEY_DOWN:
		case KEY_PGDN:
			return READ_NEXT;
		case KEY_UP:
		case KEY_PGUP:
			return READ_PREV;
		case 'Y':
		case 'R':
		case 'y':
		case 'r': {
			board_t board;
			get_board(currboard, &board);
			noreply = (fileinfo->accessed[0] & FILE_NOREPLY)
					|| (board.flag & BOARD_NOREPLY_FLAG);
			local_article = true;
			if (!noreply || am_curr_bm()) {
				do_reply(fileinfo);
			} else {
				clear();
				prints("\n\n    对不起, 该文章有不可 RE 属性, 您不能回复(RE) 这篇文章.    ");
				pressreturn();
				clear();
			}
		}
			break;
		case Ctrl('R'):
			post_reply(ent, fileinfo, direct);
			break;
		case 'g':
			digest_post(ent, fileinfo, direct);
			break;
		case Ctrl('U'):
			sread(1, 1, fileinfo);
			break;
		case Ctrl('N'):
			locate_the_post(fileinfo, fileinfo->title, 5, 0, 1);
			sread(1, 0, fileinfo);
			break;
		case Ctrl('S'):
		case 'p':
			sread(0, 0, fileinfo);
			break;
		case Ctrl('A'):
			clear();
			show_author(0, fileinfo, '\0');
			return READ_NEXT;
			break;
		case 'S':
			if (!HAS_PERM(PERM_TALK))
				break;
			clear();
			s_msg();
			break;
		default:
			break;
	}
#endif

static int load_full_post(const post_filter_t *fp, post_info_full_t *ip)
{
	query_builder_t *b = query_builder_new(0);
	b->sappend(b, "SELECT", POST_LIST_FIELDS_FULL);
	b->sappend(b, "FROM", post_table_name(fp));
	build_post_filter(b, fp);

	db_res_t *res = b->query(b);
	query_builder_free(b);

	int rows = db_res_rows(res);
	if (rows > 0)
		res_to_post_info_full(res, 0, ip);
	else
		db_clear(res);
	return rows;
}

static int read_posts(post_list_t *l, post_info_t *ip, bool thread, bool user)
{
	post_info_full_t info = { .p = *ip };
	post_filter_t filter = l->filter;

	char file[HOMELEN];
	if (!dump_content(&filter, ip, file, sizeof(file)))
		return DONOTHING;

	bool end = false, upward = false;
	while (!end) {
		brc_mark_as_read(info.p.id);

		int ch = ansimore(file, false);

		move(t_lines - 1, 0);
		clrtoeol();
		prints("\033[0;1;44;31m[阅读文章]  \033[33m回信 R │ 结束 Q,← │上一封 ↑"
				"│下一封 <Space>,↓│主题阅读 ^s或p \033[m");
		refresh();

		if (!(ch == KEY_UP || ch == KEY_PGUP))
			ch = egetch();
		switch (ch) {
			case 'N': case 'Q': case 'n': case 'q': case KEY_LEFT:
				end = true;
				break;
			case 'Y': case 'R': case 'y': case 'r':
				// TODO
				tui_new_post(&filter, &info.p);
				break;
			case ' ': case '\n': case KEY_DOWN: case KEY_RIGHT:
				upward = false;
				break;
			case KEY_UP: case 'u': case 'U':
				upward = true;
				break;
			case Ctrl('A'):
				t_query(info.p.owner);
				break;
			case Ctrl('R'):
				reply_with_mail(&filter, &info.p);
				break;
			case 'g':
				toggle_post_flag(filter.bid, &info.p, POST_FLAG_DIGEST,
						"digest");
				break;
			default:
				break;
		}

		unlink(file);
		free_post_info_full(&info);

		if (!end) {
			if (upward)
				filter.max = info.p.id - 1;
			else
				filter.min = info.p.id + 1;
			if (load_full_post(&filter, &info)) {
				dump_content_to_gbk_file(info.content, info.length,
						file, sizeof(file));
			} else {
				end = true;
			}
		}
	}
	return FULLUPDATE;
}

static void construct_prompt(char *s, size_t size, const char **options,
		size_t len)
{
	char *p = s;
	strappend(&p, &size, "区段:");
	for (int i = 0; i < len; ++i) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d)%s", i + 1, options[i]);
		strappend(&p, &size, buf);
	}
	strappend(&p, &size, "[0]:");
}

#if 0
		clear();
		prints("\n\n您将进行区段转载。转载范围是：[%d -- %d]\n", num1, num2);
		prints("当前版面是：[ %s ] \n", currboard);
		board_complete(6, "请输入要转贴的讨论区名称: ", bname, sizeof(bname),
				AC_LIST_BOARDS_ONLY);
		if (!*bname)
			return FULLUPDATE;

		if (!strcmp(bname, currboard)&&session.status != ST_RMAIL) {
			prints("\n\n对不起，本文就在您要转载的版面上，所以无需转载。\n");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (askyn("确定要转载吗", NA, NA)==NA)
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

static int import_posts(bool deleted, post_filter_t *filter, const char *path)
{
	query_builder_t *b = query_builder_new(0);
	b->sappend(b, "UPDATE", post_table_name(filter));
	b->sappend(b, "SET", "imported = TRUE");
	build_post_filter(b, filter);
	b->sappend(b, "RETURNING", "title, content");

	db_res_t *res = b->query(b);
	query_builder_free(b);

	if (res) {
		int rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			GBK_BUFFER(title, POST_TITLE_CCHARS);
			convert_u2g(db_get_value(res, i, 0), gbk_title);

			char file[HOMELEN];
			dump_content_to_gbk_file(db_get_value(res, i, 1),
					db_get_length(res, i, 1), file, sizeof(file));

			import_file(gbk_title, file, path);
		}
		db_clear(res);
		return rows;
	}
	return 0;
}

static int tui_import_posts(bool deleted, post_filter_t *filter)
{
	if (DEFINE(DEF_MULTANNPATH) && !!set_ann_path(NULL, NULL, ANNPATH_GETMODE))
		return FULLUPDATE;

	char annpath[256];
	sethomefile(annpath, currentuser.userid, ".announcepath");

	FILE *fp = fopen(annpath, "r");
	if (!fp) {
		presskeyfor("对不起, 您没有设定丝路. 请先用 f 设定丝路.",
				t_lines - 1);
		return MINIUPDATE;
	}

	fscanf(fp, "%s", annpath);
	fclose(fp);

	if (!dashd(annpath)) {
		presskeyfor("您设定的丝路已丢失, 请重新用 f 设定.", t_lines - 1);
		return MINIUPDATE;
	}

	import_posts(deleted, filter, annpath);
	return PARTUPDATE;
}

static int operate_posts_in_range(int choice, post_list_t *l, post_id_t min,
		post_id_t max)
{
	bool deleted = is_deleted(l->filter.type);
	post_filter_t filter = { .bid = l->filter.bid, .min = min, .max = max };
	int ret = PARTUPDATE;
	switch (choice) {
		case 0:
			set_post_flag(&filter, "marked", deleted, true, true);
			break;
		case 1:
			set_post_flag(&filter, "digest", deleted, true, true);
			break;
		case 2:
			set_post_flag(&filter, "locked", deleted, true, true);
			break;
		case 3:
			delete_posts(&filter, true, !HAS_PERM(PERM_OBOARDS), false);
			break;
		case 4:
			ret = tui_import_posts(is_deleted(l->filter.type), &filter);
			break;
		case 5:
			set_post_flag(&filter, "water", deleted, true, true);
			break;
		case 6:
			break;
		default:
			if (deleted) {
				undelete_posts(&filter, l->filter.type == POST_LIST_TRASH);
			} else {
				filter.flag |= POST_FLAG_WATER;
				delete_posts(&filter, true, !HAS_PERM(PERM_OBOARDS), false);
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
	l->reload = true;
	return ret;
}

static int tui_operate_posts_in_range(slide_list_t *p)
{
	post_list_t *l = p->data;

	if (!am_curr_bm())
		return DONOTHING;

	const char *option8 = is_deleted(l->filter.type) ? "恢复" : "删水文";
	const char *options[] = {
		"保留",  "文摘", "不可RE", "删除",
		"精华区", "水文", "转载", option8,
	};

	char prompt[120], ans[8];
	construct_prompt(prompt, sizeof(prompt), options, NELEMS(options));
	getdata(t_lines - 1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);

	int choice = *ans - '1';
	if (choice < 0 || choice >= NELEMS(options))
		return MINIUPDATE;

	post_id_t min, max;
	if (!tui_input_range(&min, &max))
		return MINIUPDATE;

	char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
	snprintf(prompt, sizeof(prompt), "区段[%s]操作范围 [ %s -- %s ]，确定吗",
			options[choice], pid_to_base32(min, min_str, sizeof(min_str)),
			pid_to_base32(min, max_str, sizeof(max_str)));
	if (!askyn(prompt, NA, YEA))
		return MINIUPDATE;

	return operate_posts_in_range(choice, l, min, max);
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

static slide_list_handler_t post_list_handler(slide_list_t *p, int ch)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	switch (ch) {
		case '\n': case '\r': case KEY_RIGHT: case 'r':
			return read_posts(l, ip, false, false);
		case '_':
			return toggle_post_lock(l->filter.bid, ip);
		case '@':
			return show_online();
		case '#':
			return toggle_post_stickiness(l->filter.bid, ip, l);
		case 'm':
			return toggle_post_flag(l->filter.bid, ip,
					POST_FLAG_MARKED, "marked");
		case 'g':
			return toggle_post_flag(l->filter.bid, ip,
					POST_FLAG_DIGEST, "digest");
		case 'w':
			return toggle_post_flag(l->filter.bid, ip,
					POST_FLAG_WATER, "water");
		case 'T':
			return tui_edit_post_title(ip);
		case 'E':
			return tui_edit_post_content(&l->filter, ip);
		case Ctrl('P'):
			return tui_new_post(&l->filter, NULL);
		case 'i':
			return tui_save_post(&l->filter, ip);
		case 'I':
			return tui_import_post(&l->filter, ip);
		case 'D':
			return tui_delete_posts_in_range(p);
		case 'L':
			return tui_operate_posts_in_range(p);
		case 'C':
			return tui_count_posts_in_range(l->filter.type);
		case '.':
			return post_list_deleted(l->filter.bid, l->filter.type);
		case 'J':
			return post_list_admin_deleted(l->filter.bid, l->filter.type);
		case Ctrl('G'): case Ctrl('T'): case '`':
			return tui_post_list_selected(l, ip);
		case 'a':
			return tui_search_author(p, false);
		case 'A':
			return tui_search_author(p, true);
		case '/':
			return tui_search_title(p, false);
		case '?':
			return tui_search_title(p, true);
		case '=':
			return jump_to_thread_first(p);
		case '[':
			return jump_to_thread_prev(p);
		case ']':
			return jump_to_thread_next(p);
		case 'n': case Ctrl('N'):
			return jump_to_thread_first_unread(p);
		case '\\':
			return jump_to_thread_last(p);
		case 'K':
			return skip_post(p, ip->id);
		case 'c':
			brc_clear(ip->id);
			return PARTUPDATE;
		case 'f':
			brc_clear_all();
			return PARTUPDATE;
		case 'd':
			return tui_delete_single_post(l, ip);
		case 'Y':
			return tui_undelete_single_post(l, ip);
		case 'F':
			return forward_post(&l->filter, ip, false);
		case 'U':
			return forward_post(&l->filter, ip, true);
		case Ctrl('R'):
			return reply_with_mail(&l->filter, ip);
		case 't':
			return thesis_mode();
		case '!':
			return Q_Goodbye();
		case 'S':
			s_msg();
			return FULLUPDATE;
		case 'o':
			show_online_followings();
			return FULLUPDATE;
		case 'Z':
			tui_send_msg(ip->owner);
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
		case Ctrl('A'):
			return t_query(ip->owner);
		case Ctrl('D'):
			return deny_user();
		case Ctrl('K'):
			return club_user();
		case 'x':
			if (into_announce() != DONOTHING)
				return FULLUPDATE;
		default:
			return DONOTHING;
	}
}

static int post_list_with_filter(slide_list_base_e base, post_filter_t *filter)
{
	post_list_t p = { .relocate = true, .filter = *filter };

	slide_list_t s = {
		.base = base,
		.data = &p,
		.update = FULLUPDATE,
		.loader = post_list_loader,
		.title = post_list_title,
		.display = post_list_display,
		.handler = post_list_handler,
	};

	slide_list(&s);

	free(p.index);
	free(p.posts);
	free(p.sposts);
	return 0;
}

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, user_id_t uid, const char *keyword)
{
	post_filter_t filter = { .type = type, .bid = bid, .uid = uid };

	if (keyword)
		strlcpy(filter.utf8_keyword, keyword, sizeof(filter.utf8_keyword));
	if (is_asc(base)) {
		filter.min = pid;
		filter.max = 0;
	} else {
		filter.min = 0;
		filter.max = pid;
	}

	return post_list_with_filter(base, &filter);
}

int post_list_normal_range(int bid, post_id_t pid, slide_list_base_e base)
{
	return post_list(bid, POST_LIST_NORMAL, pid, base, 0, NULL);
}

int post_list_normal(int bid)
{
	return 0;
}
