#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

enum {
	POST_FIRST = 1 << 1,
	POST_LAST = 1 << 2,
	THREAD_FIRST_POST = 1 << 3,
	THREAD_LAST_POST = 1 << 4,
	THREAD_LAST = 1 << 5,
	THREAD_FIRST = 1 << 6,
	NOT_THREAD_LAST_POST = 1 << 7,
	NOT_THREAD_LAST = 1 << 8,
};

enum {
	POST_OLDER = 'p', ///< 上一篇
	POST_NEWER = 'n', ///< 下一篇
	THREAD_PREV_POST = 'b', ///< 同主题上一篇
	THREAD_NEXT_POST = 'a', ///< 同主题下一篇
	THREAD_PREV_PAGE = 'p', ///< 同主题上一页
	THREAD_NEXT_PAGE = 'n', ///< 同主题下一页
	THREAD_OLDER = 'b', ///< 上一主题
	THREAD_NEWER = 'a', ///< 下一主题
};

typedef struct {
	char action;
	bool found;
	int flags;
	int offset;
	post_id_t pid;
	post_id_t tid;
	post_info_t *pi;
	post_record_t pr;
} bbscon_search_callback_t;

static void save_result(const post_record_t *pr, bbscon_search_callback_t *bsc,
		int off)
{
	bsc->found = true;
	bsc->offset = off;
	bsc->tid = pr->thread_id;
	memcpy(&bsc->pr, pr, sizeof(bsc->pr));
	post_record_to_info(pr, bsc->pi, 1);
}

static record_callback_e search_pid_callback(void *ptr, void *args, int off)
{
	const post_record_t *pr = ptr;
	bbscon_search_callback_t *bsc = args;

	if (bsc->action == POST_OLDER) {
		if (bsc->offset >= 0) {
			save_result(pr, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pr->id == bsc->pid)
			bsc->offset = off;
	} else if (bsc->action == THREAD_PREV_POST) {
		if (pr->id <= bsc->tid) {
			if (pr->id == bsc->tid)
				save_result(pr, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pr->id == bsc->pid)
			bsc->tid = pr->thread_id;
	} else {
		if (pr->id <= bsc->pid) {
			if (pr->id == bsc->pid)
				save_result(pr, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
	}
	return RECORD_CALLBACK_CONTINUE;
}

static record_callback_e search_next(void *ptr, void *args, int offset)
{
	const post_record_t *pr = ptr;
	bbscon_search_callback_t *bsc = args;

	if (bsc->action == POST_NEWER && offset > bsc->offset)
		save_result(pr, bsc, offset);

	if (pr->id > bsc->pid && pr->thread_id == bsc->tid) {
		if (bsc->action == THREAD_NEXT_POST)
			save_result(pr, bsc, offset);
		bsc->flags |= NOT_THREAD_LAST_POST;
		return RECORD_CALLBACK_BREAK;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int search(int bid, post_id_t pid, int action, bool extra,
		post_info_t *pi)
{
	record_t record;
	if (post_record_open(bid, &record) < 0)
		return -1;

	bbscon_search_callback_t bsc = {
		.action = action,
		.offset = -1,
		.pi = pi,
		.pid = pid,
	};
	record_reverse_foreach(&record, search_pid_callback, &bsc);

	if (bsc.found) {
		if (action == POST_NEWER || action == THREAD_NEXT_POST || extra) {
			bsc.found = !(action == POST_NEWER || action == THREAD_NEXT_POST);
			record_foreach(&record, &bsc.pr, bsc.offset, search_next, &bsc);
		}
	} else {
		memset(pi, 0, sizeof(*pi));
	}

	bsc.flags |= bsc.found;
	if (bsc.found && extra) {
		if (!bsc.offset)
			bsc.flags |= POST_FIRST;
		if (bsc.offset == record_count(&record) - 1)
			bsc.flags |= POST_LAST;
		if (pi->id == pi->thread_id)
			bsc.flags |= THREAD_FIRST_POST;
		if (!(bsc.flags & NOT_THREAD_LAST_POST))
			bsc.flags |= THREAD_LAST_POST;
	}
	record_close(&record);
	return bsc.flags;
}

static int search_pid(int bid, post_id_t pid, post_info_t *pi)
{
	return search(bid, pid, 0, false, pi);
}

static record_callback_e search_sticky_callback(void *ptr, void *args, int off)
{
	const post_record_t *pr = ptr;
	post_id_t *pid = args;
	return pr->id == *pid ? RECORD_CALLBACK_MATCH : RECORD_CALLBACK_CONTINUE;
}

static bool search_sticky(int bid, post_id_t pid, post_info_t *pi)
{
	record_t record;
	if (post_record_open_sticky(bid, &record) < 0)
		return false;

	post_record_t pr;
	bool found = (record_search_copy(&record, search_sticky_callback, &pid,
			-1, false, &pr) >= 0);
	record_close(&record);

	if (found) {
		post_record_to_info(&pr, pi, 1);
	}
	return found;
}

extern bool get_board_by_param(board_t *bp);

static int xml_print_post_wrapper(const char *str, size_t size)
{
	if (web_request_type(PARSED)) {
		int opt = PARSE_NOQUOTEIMG;
		int flag = get_user_flag();
		if (flag & PREF_NOSIG)
			opt |= PARSE_NOSIG;
		if (flag & PREF_NOSIGIMG)
			opt |= PARSE_NOSIGIMG;
		return xml_print_post(str, size, opt);
	}
	if (!web_request_type(MOBILE)) {
		xml_fputs2(str, size);
		return 0;
	}
	return xml_print_post(str, size, PARSE_NOSIG);
}

int bbscon(const char *link)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;
	session_set_board(board.id);

	post_id_t pid = strtol(web_get_param("f"), NULL, 10);
	char action = *web_get_param("a");
//	int archive = strtol(web_get_param("archive"), NULL, 10);
	bool sticky = *web_get_param("s");

	int ret = 0;
	post_info_t pi;
	if (sticky) {
		ret = search_sticky(board.id, pid, &pi);
		if (!ret)
			return BBS_ENOFILE;
	} else {
		ret = search(board.id, pid, action, true, &pi);
		if (ret <= 0)
			return BBS_ENOFILE;
	}

	xml_header(NULL);

	bool anony = board.flag & BOARD_FLAG_ANONY;
	int opt = get_user_flag();
	printf("<bbscon link='%s' bid='%d' anony='%d' attach='%d'%s%s>",
			link, board.id, anony, maxlen(board.name),
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");

	print_session();

	bool isbm = am_bm(&board);
	bool self = (session_uid() == pi.user_id);
	printf("<po fid='%"PRIdPID"'%s%s%s%s%s%s", pi.id,
			sticky ? " sticky='1'" : "",
			ret & POST_FIRST ? " first='1'" : "",
			ret & POST_LAST ? " last='1'" : "",
			ret & THREAD_LAST_POST ? " tlast='1'" : "",
			(pi.flag & POST_FLAG_LOCKED) ? " nore='1'" : "",
			self || isbm ? " edit='1'" : "");
	if (pi.reply_id != pi.id) {
		printf(" reid='%"PRIdPID"' gid='%"PRIdPID"'>", pi.reply_id,
				pi.thread_id);
	} else {
		putchar('>');
	}

	char *content = post_content_get(pi.id, false);
	if (content)
		xml_print_post_wrapper(content, strlen(content));

	brc_init(currentuser.userid, board.name);
	post_mark_as_read(&pi, content);
	brc_sync(currentuser.userid);

	free(content);

	printf("</po></bbscon>");
	return 0;
}

int bbscon_main(void)
{
	return bbscon("con");
}

int bbsgcon_main(void)
{
	return bbscon("gcon");
}

int bbsdel_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	post_id_t pid = strtoll(web_get_param("f"), NULL, 10);
	if (pid <= 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;
	session_set_board(board.id);

	post_filter_t filter = {
		.bid = board.id, .min = pid, .max = pid,
		.uid = am_bm(&board) ? 0 : session_uid(),
	};
	int deleted = post_delete(&filter, true, false, true);

	if (deleted) {
		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "deleted[www] '%"PRIdPID"' on %s\n",
				pid, board.name);
		report(buf, currentuser.userid);
	}

	printf("Location: doc?bid=%d\n\n", board.id);
	return 0;
}

typedef struct {
	const char action;
	int flags;
	post_id_t pid;
	post_id_t tid;
	int offset;
	post_record_t *prs;
	int size;
	int capacity;
} search_topic_callback_t;

static void save_offset(const post_record_t *pr, search_topic_callback_t *stc,
		int offset)
{
	stc->offset = offset;
	memcpy(stc->prs, pr, sizeof(*stc->prs));
}

static record_callback_e search_topic_callback(void *ptr, void *args, int off)
{
	const post_record_t *pr = ptr;
	search_topic_callback_t *stc = args;

	if (stc->action == THREAD_PREV_PAGE) {
		if (stc->size >= stc->capacity || pr->id < stc->tid) {
			return RECORD_CALLBACK_BREAK;
		} else if (pr->id < stc->pid && pr->thread_id == stc->tid) {
			memcpy(stc->prs + stc->size++, pr, sizeof(*stc->prs));
		}
	} else if (stc->action == THREAD_OLDER) {
		if (pr->id < stc->tid && pr->id == pr->thread_id) {
			save_offset(pr, stc, off);
			stc->tid = stc->pid = pr->id;
			return RECORD_CALLBACK_BREAK;
		}
	} else if (stc->action == THREAD_NEWER) {
		if (pr->id <= stc->tid) {
			return RECORD_CALLBACK_BREAK;
		} else {
			if (pr->id == pr->thread_id) {
				save_offset(pr, stc, off);
				stc->tid = stc->pid = pr->id;
			}
		}
	} else if (stc->action == THREAD_NEXT_PAGE) {
		if (pr->id <= stc->pid)
			return RECORD_CALLBACK_BREAK;
		else if (pr->thread_id == stc->tid)
			save_offset(pr, stc, off);
	} else {
		if (pr->id <= stc->pid) {
			if (pr->id == stc->pid)
				save_offset(pr, stc, off);
			return RECORD_CALLBACK_BREAK;
		}
	}
	return RECORD_CALLBACK_CONTINUE;
}

static record_callback_e search_topic_posts(void *ptr, void *args, int off)
{
	const post_record_t *pr = ptr;
	search_topic_callback_t *stc = args;

	if (pr->thread_id == stc->tid && pr->id >= stc->pid) {
		if (stc->size < stc->capacity)
			memcpy(stc->prs + stc->size++, pr, sizeof(*stc->prs));
		else
			stc->flags |= NOT_THREAD_LAST_POST;
	}
	if (pr->id == pr->thread_id && pr->id >= stc->tid)
		stc->flags |= NOT_THREAD_LAST;
	return RECORD_CALLBACK_CONTINUE;
}

static post_record_t *search_topic(int bid, post_id_t pid, post_id_t *tid,
		int action, int *count, int *flags)
{
	record_t record;
	if (post_record_open(bid, &record) < 0)
		return NULL;

	post_record_t *prs = malloc(sizeof(*prs) * *count);
	if (!prs) {
		record_close(&record);
		return NULL;
	}

	search_topic_callback_t stc = {
		.action = action,
		.pid = pid,
		.tid = *tid,
		.offset = -1,
		.prs = prs,
		.capacity = *count,
	};
	record_reverse_foreach(&record, search_topic_callback, &stc);

	if (action != THREAD_PREV_PAGE && stc.offset >= 0) {
		record_foreach(&record, stc.prs, stc.offset, search_topic_posts, &stc);
	}

	record_close(&record);
	if (!stc.size) {
		free(prs);
		return NULL;
	}
	*tid = stc.tid;
	*count = stc.size;
	*flags = stc.flags;
	return prs;
}

enum {
	POSTS_PER_PAGE = 20,
};

int bbstcon_main(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;
	session_set_board(board.id);

	post_id_t tid = strtoll(web_get_param("g"), NULL, 10);
	post_id_t pid = strtoll(web_get_param("f"), NULL, 10);
	char action = *(web_get_param("a"));
	if (!tid)
		tid = pid;

	int count = POSTS_PER_PAGE;
	int c = count, flags = 0;
	post_record_t *pr = search_topic(board.id, pid, &tid, action, &c, &flags);
	if (!pr)
		return BBS_ENOFILE;

	bool anony = board.flag & BOARD_FLAG_ANONY;
	int opt = get_user_flag();
	bool isbm = am_bm(&board);

	xml_header(NULL);
	printf("<bbstcon bid='%d' gid='%"PRIdPID"' anony='%d' page='%d'"
			" attach='%d'%s%s%s%s%s>",
			board.id, tid, anony, count, maxlen(board.name),
			flags & NOT_THREAD_LAST_POST ? "" : " last='1'",
			flags & NOT_THREAD_LAST ? "" : " tlast='1'",
			flags & THREAD_FIRST ? " tfirst='1'" : "",
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");
	print_session();

	brc_init(currentuser.userid, board.name);

	bool asc = action != THREAD_PREV_PAGE;
	if (c > count)
		c = count;

	for (post_record_t *ptr = asc ? pr : pr + c - 1;
			asc ? ptr < pr + c : ptr >= pr;
			ptr += asc ? 1 : -1) {
		post_info_t pi;
		post_record_to_info(ptr, &pi, 1);
		printf("<po fid='%"PRIdPID"' owner='%s'%s>", pi.id, pi.user_name,
				!isbm && (pi.flag & POST_FLAG_LOCKED) ? " nore='1'" : "");

		char *content = post_content_get(pi.id, false);
		if (content)
			xml_print_post_wrapper(content, strlen(content));
		post_mark_as_read(&pi, content);
		free(content);

		puts("</po>");
	}
	puts("</bbstcon>");

	brc_sync(currentuser.userid);
	return 0;
}

int web_sigopt(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	bool hidesig = streq(web_get_param("hidesig"), "on");
	bool hideimg = streq(web_get_param("hideimg"), "on");

	int flag = get_user_flag();

	if (hidesig)
		flag |= PREF_NOSIG;
	else
		flag &= ~PREF_NOSIG;
	if (hideimg)
		flag |= PREF_NOSIGIMG;
	else
		flag &= ~PREF_NOSIGIMG;

	set_user_flag(flag);

	html_header();
	printf("<title>success</title></head><body></body></html>");
	return 0;
}

static int edit_article(post_id_t pid, const char *content, const char *text,
		const char *ip)
{
	if (!content || !text || !ip)
		return BBS_EINTNL;

	size_t len = strlen(content), header_len = 0;
	const char *ptr = content, *end = ptr + len;
	if (len > 0) {
		// 跳过文章头部
		int n = 3;
		while (ptr != end && n >= 0) {
			if (*ptr == '\n')
				--n;
			++ptr;
		}
		header_len = ptr - content;

		ptr = end - 2; // 跳过最后的换行符
		while (ptr >= content && *ptr != '\n')
			--ptr;
		if (ptr >= content) {
			//% ※ 修改
			if (strneq2(ptr + 1, "\033[m\033[1;36m※ 修改")) {
				// 如果已经修改过，应该覆盖修改标记
				end = ptr + 1;
				--ptr;
				while (ptr >= content && *ptr != '\n')
					--ptr;
			}
		}
	}

	char buf[256];
	int mark_len = snprintf(buf, sizeof(buf), "\033[m\033[1;36m※ 修改:·%s 于 "
			"%.25s·HTTP [FROM: %s]\033[m\n", currentuser.userid,
			format_time(fb_time(), TIME_FORMAT_UTF8_ZH), mask_host(ip));
	if (mark_len < 0)
		return BBS_EINTNL;

	size_t size = header_len + strlen(text) * 2 + (end - ptr) + mark_len + 1;
	char *out = malloc(size), *dst = out;
	if (!out)
		return BBS_EINTNL;
	memcpy(dst, content, header_len);
	dst += header_len;
	convert(CONVERT_G2U, text, CONVERT_ALL, dst, size - header_len,
			NULL, NULL);
	size_t left = size - strlen(out);
	if (left > (end - ptr) + mark_len) {
		dst = out + size - left;
		memcpy(dst, ptr, end - ptr);
		dst += end - ptr;
		memcpy(dst, buf, mark_len);
		dst += mark_len;
		*dst = '\0';
	}

	bool ok = post_content_set(pid, out);
	free(out);
	return ok ? 0 : BBS_EINTNL;
}

static void check_character(char *text)
{
	char *dst = text, *src;
	for (src = text; *src != '\0'; ++src) {
		switch (*src) {
			case '\x1': case '\x2': case '\x3': case '\x4': case '\x5':
			case '\x6': case '\x7': case '\x8': case '\xb': case '\xc':
			case '\xe': case '\xf': case '\x10': case '\x11': case '\x12':
			case '\x13': case '\x14': case '\x15': case '\x16': case '\x17':
			case '\x18': case '\x19': case '\x1a': case '\x1c': case '\x1d':
			case '\x1e': case '\x1f': case '\r':
				break;
			default:
				*dst++ = *src;
				break;
		}
	}
	*dst = '\0';
}

extern const char *get_post_list_type_string(void);

int bbssnd_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	if (web_parse_post_data() < 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (!has_post_perm(&board))
		return BBS_EPST;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;

	bool isedit = (*(web_get_param("e")) == '1');

	UTF8_BUFFER(title, POST_TITLE_CCHARS);
	if (!isedit) {
		if (web_request_type(UTF8)) {
			strlcpy(utf8_title, web_get_param("title"), sizeof(utf8_title));
		} else {
			convert_g2u(web_get_param("title"), utf8_title);
		}
		string_remove_ansi_control_code(utf8_title, utf8_title);
		string_remove_non_printable(utf8_title);
		string_check_tail(utf8_title, NULL);
		if (*utf8_title == '\0')
			return BBS_EINVAL;
	}

	time_t now = fb_time();
	int diff = now - get_my_last_post_time();
	set_my_last_post_time(now);
	if (diff < 6)
		return BBS_EPFREQ;

	post_id_t pid = 0;
	post_info_t pi;

	const char *f = web_get_param("f");
	bool reply = !(*f == '\0');

	if (isedit || reply) {
		pid = strtol(f, NULL, 10);
		if (!pid || !search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		if (!am_bm(&board) && session_uid() != pi.user_id) {
			if (!isedit && (pi.flag & POST_FLAG_LOCKED))
				return BBS_EPST;
			if (isedit)
				return BBS_EACCES;
		}
	}

	char *text = (char *) web_get_param("text");
	check_character(text);
	if (web_request_type(UTF8)
			&& string_validate_utf8(text, POST_CONTENT_CCHARS, true) < 0)
		return BBS_EINVAL;

	if (isedit) {
		int ret = -1;
		char *content = post_content_get(pi.id, false);
		if (content)
			ret = edit_article(pid, content, text, fromhost);
		free(content);

		if (ret < 0)
			return BBS_EINTNL;
	} else {
		post_request_t pr = {
			.user = &currentuser,
			.board = &board,
			.title = utf8_title,
			.content = text,
			.sig = strtol(web_get_param("sig"), NULL, 0),
			.ip = mask_host(fromhost),
			.reid = reply ? pi.id : 0,
			.tid = reply ? pi.thread_id : 0,
			.uid_replied = reply ? pi.user_id : 0,
			.locked = reply && (pi.flag & POST_FLAG_LOCKED),
			.anony = strtol(web_get_param("anony"), NULL, 0),
			.web = true,
			.convert_type = web_request_type(UTF8)
					? CONVERT_NONE : CONVERT_G2U,
		};
		pid = post_new(&pr);
		if (!pid)
			return BBS_EINTNL;
	}

	if (!isedit && !(board.flag & BOARD_FLAG_JUNK)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[128];
	snprintf(buf, sizeof(buf), "%sed '%s' on %s", isedit ? "edit" : "post",
			utf8_title, board.name);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "%sdoc?board=%s", get_post_list_type_string(),
			board.name);
	if (web_request_type(MOBILE)) {
		printf("Status: 302\nLocation: %s\n\n", buf);
	} else {
		http_header();
		refreshto(1, buf);
		//% printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>发表</a>"
		printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>\xb7\xa2\xb1\xed</a>"
		//% "成功，1秒钟后自动转到<a href='%s'>版面</a>\n</body>\n</html>\n",
		"\xb3\xc9\xb9\xa6\xa3\xac""1\xc3\xeb\xd6\xd3\xba\xf3\xd7\xd4\xb6\xaf\xd7\xaa\xb5\xbd<a href='%s'>\xb0\xe6\xc3\xe6</a>\n</body>\n</html>\n",
		board.id, pid, buf);
	}
	return 0;
}

static void get_post_body(const char **begin, const char **end)
{
	const char *ptr = *begin, *e = *end;
	// skip header.
	int n = 3;
	while (ptr != e && n >= 0) {
		if (*ptr == '\n')
			--n;
		++ptr;
	}
	*begin = ptr;

	ptr = e - 2; // skip last '\n'
	while (ptr >= *begin && *ptr != '\n')
		--ptr;
	if (ptr < *begin)
		return;
	if (strneq2(ptr + 1, "\033[m\033[1;36m※ 修改")) {
		--ptr;
		while (ptr >= *begin && *ptr != '\n')
			--ptr;
		*end = (ptr >= *begin) ? ptr : *begin;
	} else {
		*end = ptr;
	}
}

static int do_bbspst(bool isedit)
{
	if (!session_id())
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (!has_post_perm(&board))
		return BBS_EPST;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;

	const char *f = web_get_param("f");
	bool reply = !(*f == '\0');

	if (isedit && !reply)
		return BBS_EINVAL;

	post_id_t pid = 0;
	post_info_t pi;

	if (reply) {
		pid = strtoll(f, NULL, 10);
		if (!pid || !search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;
		if (!isedit && (pi.flag & POST_FLAG_LOCKED))
			return BBS_EPST;
		if (isedit && !am_bm(&board) && session_uid() != pi.user_id)
			return BBS_EACCES;
	}
	
	xml_header(NULL);
	char path[HOMELEN];
	snprintf(path, sizeof(path), BBSHOME"/upload/%s", board.name);
	bool anony = board.flag & BOARD_FLAG_ANONY;
	printf("<bbspst brd='%s' bid='%d' edit='%d' att='%d' anony='%d'>",
			board.name, board.id, isedit, dashd(path), anony);
	print_session();

	if (reply) {
		printf("<t>");

		bool utf8 = web_request_type(UTF8);

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		char *title = utf8 ? pi.utf8_title : gbk_title;
		if (!utf8)
			convert_u2g(pi.utf8_title, gbk_title);
		string_remove_ansi_control_code(title, title);
		if (web_request_type(MOBILE) && !strneq2(title, "Re: "))
			fputs("Re: ", stdout);
		xml_fputs(title);

		printf("</t><po f='%lu'>", pid);

		char *utf8_content = post_content_get(pi.id, false);
		size_t len = strlen(utf8_content);

		if (isedit) {
			const char *begin = utf8_content;
			const char *end = begin + len;
			get_post_body(&begin, &end);
			if (end > begin) {
				if (utf8) {
					xml_fputs2(begin, end - begin);
				} else {
					char *gbk_content = malloc(len + 1);
					convert(CONVERT_U2G, begin, end - begin, gbk_content, len,
							NULL, NULL);
					xml_fputs(gbk_content);
					free(gbk_content);
				}
			}
		} else {
			if (utf8) {
				post_quote_string(utf8_content, len, NULL, POST_QUOTE_AUTO,
						false, true, xml_fputs3);
			} else {
				char *gbk_content = malloc(len + 1);
				convert(CONVERT_U2G, utf8_content, len, gbk_content, len,
						NULL, NULL);
				post_quote_string(gbk_content, strlen(gbk_content), NULL,
						POST_QUOTE_AUTO, false, false, xml_fputs3);
				free(gbk_content);
			}
		}

		free(utf8_content);

		fputs("</po>", stdout);
	}
	printf("</bbspst>");
	return 0;
}

int bbspst_main(void)
{
	return do_bbspst(false);
}

int bbsedit_main(void)
{
	return do_bbspst(true);
}

#define CP_MARK_STRING  "[转载]"

int bbsccc_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	web_parse_post_data();

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;

	post_id_t pid = strtoll(web_get_param("f"), NULL, 10);

	post_info_t pi;
	if (!search_pid(board.id, pid, &pi))
		return BBS_ENOFILE;

	const char *target = web_get_param("t");
	if (*target != '\0') {
		board_t to;
		if (!get_board(target, &to))
			return BBS_ENOBRD;
		if ((to.flag & BOARD_FLAG_DIR) || to.id == board.id)
			return BBS_EINVAL;
		if (!has_post_perm(&to))
			return BBS_EPST;

		UTF8_BUFFER(title, POST_TITLE_CCHARS);
		if (strneq2(pi.utf8_title, CP_MARK_STRING)) {
			strlcpy(utf8_title, pi.utf8_title, sizeof(utf8_title));
		} else {
			snprintf(utf8_title, sizeof(utf8_title), CP_MARK_STRING"%s",
					pi.utf8_title);
		}
		string_remove_non_printable(utf8_title);

		char *content = post_content_get(pi.id, false);

		post_request_t pr = {
			.crosspost = true,
			.user = &currentuser,
			.board = &to,
			.title = utf8_title,
			.content = content,
			.ip = mask_host(fromhost),
			.web = true,
		};
		int ret = post_new(&pr);

		free(content);
		if (!ret)
			return BBS_EINTNL;

		xml_header(NULL);
		printf("<bbsccc t='%ld' b='%ld' f='%u'>",
				to.id, board.id, ret);
		print_session();
		printf("</bbsccc>");
	} else {
		xml_header(NULL);
		printf("<bbsccc owner='%s' brd='%s' bid='%ld' fid='%u'>",
				pi.user_name, board.name, board.id, pid);

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(pi.utf8_title, gbk_title);
		xml_fputs(gbk_title);

		print_session();
		printf("</bbsccc>");
	}
	return 0;
}

/**
 * Forward post.
 * @return 0 on success, bbserrno on error.
 */
// fwd?bid=[bid]&f=[fid]&u=[recipient]
int bbsfwd_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	web_parse_post_data();
	const char *reci = web_get_param("u");
	if (*reci == '\0') {
		xml_header(NULL);
		printf("<bbsfwd bid='%s' f='%s'>",
				web_get_param("bid"), web_get_param("f"));
		print_session();
		printf("</bbsfwd>");
	} else {
		if (!HAS_PERM(PERM_MAIL))
			return BBS_EACCES;

		board_t board;
		if (!get_board_by_param(&board))
			return BBS_ENOBRD;
		if (board.flag & BOARD_FLAG_DIR)
			return BBS_EINVAL;

		post_id_t pid = strtoll(web_get_param("f"), NULL, 10);
		post_info_t pi;
		if (!search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
		snprintf(utf8_title, sizeof(utf8_title), "[转寄]%s", pi.utf8_title);
		convert_u2g(utf8_title, gbk_title);

		char *content = post_content_get(pi.id, false);

		char file[HOMELEN];
		int ret = post_dump_gbk_file(content, CONVERT_ALL, file, sizeof(file));

		free(content);

		if (ret == 0) {
			ret = mail_file(file, reci, gbk_title);
			unlink(file);

			if (ret)
				return ret;
			http_header();
			//% printf("</head><body><p>文章转寄成功</p>"
			printf("</head><body><p>\xce\xc4\xd5\xc2\xd7\xaa\xbc\xc4\xb3\xc9\xb9\xa6</p>"
					//% "<a href='javascript:history.go(-2)'>返回</a>"
					"<a href='javascript:history.go(-2)'>\xb7\xb5\xbb\xd8</a>"
					"</body></html>");
		}
	}
	return 0;
}

enum {
	API_POST_DEFAULT_COUNT = 20,
};

int api_post_content(void)
{
	post_id_t thread_id = strtoll(web_get_param("thread_id"), NULL, 10);
	if (thread_id <= 0)
		return WEB_ERROR_BAD_REQUEST;
	post_id_t since_id = strtoll(web_get_param("since_id"), NULL, 10);

	board_t board;
	int board_id = strtol(web_get_param("board_id"), NULL, 10);
	if (board_id <= 0 || get_board_by_bid(board_id, &board) <= 0
			|| !has_read_perm(&board)) {
		return WEB_ERROR_BOARD_NOT_FOUND;
	}

	int count = strtol(web_get_param("count"), NULL, 10);
	if (count > API_POST_DEFAULT_COUNT || count <= 0)
		count = API_POST_DEFAULT_COUNT;

	json_object_t *object = json_object_new();
	web_set_response(object, JSON_OBJECT);

	json_object_integer(object, "board_id", board.id);
	json_object_string(object, "board_name", board.name);

	json_array_t *posts = json_array_new();
	json_object_append(object, "posts", posts, JSON_ARRAY);

	query_t *q = query_new(0);
	query_select(q, POST_TABLE_FIELDS);
	query_from(q, "post.recent");
	query_where(q, "board_id = %d", board_id);
	query_and(q, "thread_id = %"DBIdPID, thread_id);
	if (since_id)
		query_and(q, "id > %"DBIdPID, since_id);
	query_orderby(q, "id", true);
	query_limit(q, count);

	db_res_t *res = query_exec(q);
	count = res ? db_res_rows(res) : 0;
	for (int i = 0; i < count; ++i) {
		post_record_t pr;
		post_record_from_query(res, i, &pr, false);
		char *content = post_content_get(pr.id, false);
		if (content) {
			json_object_t *post = json_object_new();
			json_object_string(post, "content", content);
			free(content);
			json_object_bigint(post, "id", pr.id);
			json_object_bigint(post, "reply_id", pr.reply_id);
			json_object_bigint(post, "thread_id", pr.thread_id);
			json_object_integer(post, "user_id", pr.user_id);
			json_object_string(post, "user_name", pr.user_name);
			json_object_integer(post, "flags", pr.flag);
			json_object_string(post, "title", pr.utf8_title);
			json_array_append(posts, post, JSON_OBJECT);
		}
	}
	db_clear(res);
	return WEB_OK;
}
