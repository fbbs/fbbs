#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/friend.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct {
	post_list_type_e type;
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
	if (l->type == POST_LIST_NORMAL && !l->sposts && l->sreload) {
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
	bool locked = ip->flag & POST_FLAG_LOCKED;
	if (am_curr_bm() || (session.id == ip->uid && !locked)) {
		if (set_post_flag_unchecked(bid, ip->id, "locked", !locked)) {
			set_post_flag(ip, POST_FLAG_LOCKED, !locked);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int toggle_post_stickiness(int bid, post_info_t *ip, post_list_t *l)
{
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
	bool set = ip->flag & flag;
	if (am_curr_bm()) {
		if (set_post_flag_unchecked(bid, ip->id, field, !set)) {
			set_post_flag(ip, flag, !set);
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
	if (p->type == POST_LIST_JUNK || p->type == POST_LIST_TRASH) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (undelete_posts(&f, p->type == POST_LIST_TRASH)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static bool dump_content(const post_info_t *ip, char *file, size_t size,
		char *title, size_t tsize)
{
	db_res_t *res = query_post_by_pid(ip->id, ip->flag & POST_FLAG_DELETED,
			"title, content");
	if (!res || db_res_rows(res) < 1)
		return false;

	int ret = dump_content_to_gbk_file(db_get_value(res, 0, 1),
			db_get_length(res, 0, 1), file, sizeof(file));

	if (title) {
		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(db_get_value(res, 0, 0), gbk_title);
		strlcpy(title, gbk_title, tsize);
	}

	db_clear(res);
	return ret == 0;
}

static int forward_post(post_info_t *ip, bool uuencode)
{
	char file[HOMELEN];
	GBK_BUFFER(title, POST_TITLE_CCHARS);

	if (!dump_content(ip, file, sizeof(file), gbk_title, sizeof(gbk_title)))
		return DONOTHING;

	tui_forward(file, gbk_title, uuencode);

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

static int tui_edit_post_content(post_info_t *ip)
{
	if (ip->uid != session.uid && !am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file), NULL, 0))
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
extern int mailtoauther;

static int tui_new_post(int bid, post_info_t *ip)
{
	time_t now = time(NULL);
	if (now - get_last_post_time() < 3) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("您太辛苦了，先喝杯咖啡歇会儿，3 秒钟后再发表文章。\n");
		pressreturn();
		return MINIUPDATE;
	}

	set_user_status(ST_POSTING);

	board_t board;
	if (!get_board_by_bid(bid, &board) ||
			!has_post_perm(&currentuser, &board)) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("此讨论区是唯读的, 或是您尚无权限在此发表文章。");
		pressreturn();
		return FULLUPDATE;
	}

	clear();
	show_board_notes(board.name, 1);

	struct postheader header;
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (ip) {
		header.reply_mode = 1;
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
		if (!header.reply_mode && header.prefix[0]) {
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
		dump_content(ip, orig, sizeof(orig), NULL, 0);
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

	if (noreply) {
		noreply = 0;
	}

	if (mailtoauther) {
		if (header.chk_anony) {
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
		mailtoauther = 0;
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
		.locked = noreply,
		.marked = false,
		.anony = header.chk_anony,
		.cp = NULL,
	};

//	save_gid = postfile.gid;
	post_id_t pid = publish_post(&req);
	if (pid) {
		brc_mark_as_read(pid);

		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "posted '%s' on %s",
				gbk_title, board.name);
		report(buf, currentuser.userid);
	}

	if (!is_junk_board(&board) && !header.chk_anony) {
		set_safe_record();
		currentuser.numposts++;
		substitut_record(PASSFILE, &currentuser, sizeof (currentuser),
				usernum);
	}
	bm_log(currentuser.userid, currboard, BMLOG_POST, 1);
	return FULLUPDATE;
}

static int tui_save_post(const post_info_t *ip)
{
	if (!am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (!dump_content(ip, file, sizeof(file), gbk_title, sizeof(gbk_title)))
		return DONOTHING;

	a_Save(gbk_title, file, false, true);

	return MINIUPDATE;
}

static int tui_import_post(const post_info_t *ip)
{
	if (!HAS_PERM(PERM_BOARDS))
		return DONOTHING;

	if (DEFINE(DEF_MULTANNPATH)
			&& set_ann_path(NULL, NULL, ANNPATH_GETMODE) == 0)
		return FULLUPDATE;

	char file[HOMELEN];
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (dump_content(ip, file, sizeof(file), gbk_title, sizeof(gbk_title)))
		a_Import(gbk_title, file, NA);

	if (DEFINE(DEF_MULTANNPATH))
		return FULLUPDATE;
	return DONOTHING;
}

static int tui_delete_posts_in_range(slide_list_t *p)
{
	if (!am_curr_bm())
		return DONOTHING;

	post_list_t *l = p->data;
	if (l->type != POST_LIST_NORMAL)
		return DONOTHING;

	char num[8];
	getdata(t_lines - 1, 0, "首篇文章编号: ", num, sizeof(num), DOECHO, YEA);
	post_id_t min = base32_to_pid(num);
	if (!min) {
		move(t_lines - 1, 50);
		prints("错误编号...");
		egetch();
		return MINIUPDATE;
	}

	getdata(t_lines - 1, 25, "末篇文章编号: ", num, sizeof(num), DOECHO, YEA);
	post_id_t max = base32_to_pid(num);
	if (max < min) {
		move(t_lines - 1, 50);
		prints("错误区间...");
		egetch();
		return MINIUPDATE;
	}

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
		case 'N':
		case 'Q':
		case 'n':
		case 'q':
		case KEY_LEFT:
			break;
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

static int read_post(post_list_t *l, post_info_t *ip)
{
	brc_mark_as_read(ip->id);

	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file), NULL, 0))
		return DONOTHING;

	int ch = ansimore(file, false);

	move(t_lines - 1, 0);
	clrtoeol();
	prints("\033[0;1;44;31m[阅读文章]  \033[33m回信 R │ 结束 Q,← │上一封 ↑"
			"│下一封 <Space>,↓│主题阅读 ^s或p \033[m");
	refresh();

	if (!(ch == KEY_UP || ch == KEY_PGUP))
		ch = egetch();

	unlink(file);
	return FULLUPDATE;
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
			return read_post(l, ip);
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
			return tui_edit_post_content(ip);
		case Ctrl('P'):
			return tui_new_post(l->filter.bid, ip);
		case 'i':
			return tui_save_post(ip);
		case 'I':
			return tui_import_post(ip);
		case 'D':
			return tui_delete_posts_in_range(p);
		case '.':
			return post_list_deleted(l->filter.bid, l->type);
		case 'J':
			return post_list_admin_deleted(l->filter.bid, l->type);
		case 'a':
			return tui_search_author(p, false);
		case 'A':
			return tui_search_author(p, true);
		case '=':
			return jump_to_thread_first(p);
		case '[':
			return jump_to_thread_prev(p);
		case ']':
			return jump_to_thread_next(p);
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
			return forward_post(ip, false);
		case 'U':
			return forward_post(ip, true);
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

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, user_id_t uid, const char *keyword)
{
	post_list_t p = {
		.type = type,
		.filter = { .deleted = is_deleted(type), .bid = bid, .uid = uid },
		.relocate = true, .reload = false, .last_query_rows = 0,
		.index = NULL, .posts = NULL, .sposts = NULL,
		.icount = 0, .count = 0, .scount = 0, .sreload = false,
	};
	if (keyword)
		strlcpy(p.filter.utf8_keyword, keyword, sizeof(p.filter.utf8_keyword));
	if (is_asc(base)) {
		p.filter.min = pid;
		p.filter.max = 0;
	} else {
		p.filter.min = 0;
		p.filter.max = pid;
	}

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

int post_list_normal_range(int bid, post_id_t pid, slide_list_base_e base)
{
	return post_list(bid, POST_LIST_NORMAL, pid, base, 0, NULL);
}

int post_list_normal(int bid)
{
	return 0;
}
