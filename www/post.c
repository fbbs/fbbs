#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
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
	post_index_record_t *pir;
	post_index_board_t pib;
} bbscon_search_callback_t;

static inline post_id_t get_tid(const post_index_board_t *pib)
{
	return pib->id - pib->tid_delta;
}

static int save_result(const post_index_board_t *pib,
		bbscon_search_callback_t *bsc, int off)
{
	bsc->found = true;
	bsc->offset = off;
	bsc->tid = get_tid(pib);
	memcpy(&bsc->pib, pib, sizeof(bsc->pib));
	return post_index_board_to_info(bsc->pir, pib, bsc->pi, 1);
}

static record_callback_e search_pid_callback(void *ptr, void *args, int off)
{
	const post_index_board_t *pib = ptr;
	bbscon_search_callback_t *bsc = args;

	if (bsc->action == POST_OLDER) {
		if (bsc->offset >= 0) {
			save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pib->id == bsc->pid)
			bsc->offset = off;
	} else if (bsc->action == THREAD_PREV_POST) {
		if (pib->id <= bsc->tid) {
			if (pib->id == bsc->tid)
				save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pib->id == bsc->pid)
			bsc->tid = get_tid(pib);
	} else {
		if (pib->id <= bsc->pid) {
			if (pib->id == bsc->pid)
				save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
	}
	return RECORD_CALLBACK_CONTINUE;
}

static record_callback_e search_next(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	bbscon_search_callback_t *bsc = args;

	if (bsc->action == POST_NEWER && offset > bsc->offset)
		save_result(pib, bsc, offset);

	if (pib->id > bsc->pid && get_tid(pib) == bsc->tid) {
		if (bsc->action == THREAD_NEXT_POST)
			save_result(pib, bsc, offset);
		bsc->flags |= NOT_THREAD_LAST_POST;
		return RECORD_CALLBACK_BREAK;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int search(int bid, post_id_t pid, int action, bool extra,
		post_info_t *pi)
{
	record_t record;
	if (post_index_board_open(bid, RECORD_READ, &record) < 0)
		return -1;

	post_index_record_t pir;
	post_index_record_open(&pir);

	bbscon_search_callback_t bsc = {
		.flags = 0, .action = action, .offset = -1, .pi = pi, .pid = pid,
		.pir = &pir, .tid = 0, .found = false,
	};
	record_reverse_foreach(&record, search_pid_callback, &bsc);

	if (bsc.found) {
		if (action == POST_NEWER || action == THREAD_NEXT_POST || extra) {
			bsc.found = !(action == POST_NEWER || action == THREAD_NEXT_POST);
			record_foreach(&record, &bsc.pib, bsc.offset, search_next, &bsc);
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
		if (pi->id == pi->tid)
			bsc.flags |= THREAD_FIRST_POST;
		if (!(bsc.flags & NOT_THREAD_LAST_POST))
			bsc.flags |= THREAD_LAST_POST;
	}
	post_index_record_close(&pir);
	record_close(&record);
	return bsc.flags;
}

static int search_pid(int bid, post_id_t pid, post_info_t *pi)
{
	return search(bid, pid, 0, false, pi);
}

static record_callback_e search_sticky_callback(void *ptr, void *args, int off)
{
	const post_index_board_t *pib = ptr;
	post_id_t *pid = args;
	return pib->id == *pid ? RECORD_CALLBACK_MATCH : RECORD_CALLBACK_CONTINUE;
}

static bool search_sticky(int bid, post_id_t pid, post_info_t *pi)
{
	record_t record;
	if (post_index_board_open_sticky(bid, RECORD_READ, &record) < 0)
		return false;

	post_index_board_t pib;
	bool found = (record_search_copy(&record, search_sticky_callback, &pid,
			-1, false, &pib) >= 0);
	record_close(&record);

	if (found) {
		post_index_record_t pir;
		post_index_record_open(&pir);
		post_index_board_to_info(&pir, &pib, pi, 1);
		post_index_record_close(&pir);
	}
	return found;
}

extern bool get_board_by_param(board_t *bp);

static int xml_print_post_wrapper(const char *str, size_t size)
{
	if (request_type(REQUEST_PARSED)) {
		int opt = PARSE_NOQUOTEIMG;
		int flag = get_user_flag();
		if (flag & PREF_NOSIG)
			opt |= PARSE_NOSIG;
		if (flag & PREF_NOSIGIMG)
			opt |= PARSE_NOSIGIMG;
		return xml_print_post(str, size, opt);
	}
	if (!request_type(REQUEST_MOBILE)) {
		xml_fputs2(str, size);
		return 0;
	}
	return xml_print_post(str, size, PARSE_NOSIG);
}

int bbscon_main(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	set_current_board(board.id);

	post_id_t pid = strtol(get_param("f"), NULL, 10);
	char action = *get_param("a");
//	int archive = strtol(get_param("archive"), NULL, 10);
	bool sticky = *get_param("s");

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

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();
	printf("<bbscon link='con' bid='%d' anony='%d' attach='%d'%s%s>",
			board.id, anony, maxlen(board.name),
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");

	print_session();

	bool isbm = am_bm(&board);
	bool self = (session_uid() == pi.uid);
	printf("<po fid='%"PRIdPID"'%s%s%s%s%s%s", pi.id,
			sticky ? " sticky='1'" : "",
			ret & POST_FIRST ? " first='1'" : "",
			ret & POST_LAST ? " last='1'" : "",
			ret & THREAD_LAST_POST ? " tlast='1'" : "",
			(pi.flag & POST_FLAG_LOCKED) ? " nore='1'" : "",
			self || isbm ? " edit='1'" : "");
	if (pi.reid != pi.id)
		printf(" reid='%"PRIdPID"' gid='%"PRIdPID"'>", pi.reid, pi.tid);
	else
		putchar('>');

	char buffer[4096];
	char *content = post_content_get(pi.id, buffer, sizeof(buffer));
	xml_print_post_wrapper(content, strlen(content));
	if (content != buffer)
		free(content);

	printf("</po></bbscon>");

	brc_initialize(currentuser.userid, board.name);
	brc_mark_as_read(pi.stamp);
	brc_update(currentuser.userid, board.name);

	return 0;
}

int bbsgcon_main(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	set_current_board(board.id);

	const char *f = get_param("f");
	if (strstr(f, "..") || strstr(f, "/") || strncmp(f, "G.", 2))
		return BBS_EINVAL;
	xml_header(NULL);
	printf("<bbscon link='gcon' bid='%d'>", board.id);
	print_session();
	printf("<po>");

	char file[HOMELEN];
	setbfile(file, board.name, f);
	xml_print_post_wrapper(file, 0);

	printf("</po></bbscon>");
	brc_initialize(currentuser.userid, board.name);
	brc_addlist_legacy(f);
	brc_update(currentuser.userid, board.name);
	return 0;
}

int bbsdel_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	post_id_t pid = strtoll(get_param("f"), NULL, 10);
	if (pid <= 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	set_current_board(board.id);

	record_t record;
	if (post_index_board_open(board.id, RECORD_WRITE, &record) <= 0)
		return BBS_EINTNL;

	post_filter_t filter = {
		.bid = board.id, .min = pid, .max = pid,
		.uid = am_bm(&board) ? 0 : session_uid(),
	};
	int deleted = post_index_board_delete(&filter, NULL, 0, true, false, true);
	record_close(&record);

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
	post_index_board_t *pib;
	int size;
	int capacity;
} search_topic_callback_t;

static void save_offset(const post_index_board_t *pib,
		search_topic_callback_t *stc, int offset)
{
	stc->offset = offset;
	memcpy(stc->pib, pib, sizeof(*stc->pib));
}

static record_callback_e search_topic_callback(void *ptr, void *args, int off)
{
	const post_index_board_t *pib = ptr;
	search_topic_callback_t *stc = args;

	if (stc->action == THREAD_PREV_PAGE) {
		if (stc->size >= stc->capacity || pib->id < stc->tid) {
			return RECORD_CALLBACK_BREAK;
		} else if (pib->id < stc->pid && get_tid(pib) == stc->tid) {
			memcpy(stc->pib + stc->size++, pib, sizeof(*stc->pib));
		}
	} else if (stc->action == THREAD_OLDER) {
		if (pib->id < stc->tid && !pib->tid_delta) {
			save_offset(pib, stc, off);
			stc->tid = stc->pid = pib->id;
			return RECORD_CALLBACK_BREAK;
		}
	} else if (stc->action == THREAD_NEWER) {
		if (pib->id <= stc->tid) {
			return RECORD_CALLBACK_BREAK;
		} else {
			if (!pib->tid_delta) {
				save_offset(pib, stc, off);
				stc->tid = stc->pid = pib->id;
			}
		}
	} else if (stc->action == THREAD_NEXT_PAGE) {
		if (pib->id <= stc->pid)
			return RECORD_CALLBACK_BREAK;
		else if (get_tid(pib) == stc->tid)
			save_offset(pib, stc, off);
	} else {
		if (pib->id <= stc->pid) {
			if (pib->id == stc->pid)
				save_offset(pib, stc, off);
			return RECORD_CALLBACK_BREAK;
		}
	}
	return RECORD_CALLBACK_CONTINUE;
}

static record_callback_e search_topic_posts(void *ptr, void *args, int off)
{
	const post_index_board_t *pib = ptr;
	search_topic_callback_t *stc = args;

	if (get_tid(pib) == stc->tid && pib->id >= stc->pid) {
		if (stc->size < stc->capacity)
			memcpy(stc->pib + stc->size++, pib, sizeof(*stc->pib));
		else
			stc->flags |= NOT_THREAD_LAST_POST;
	}
	if (!pib->tid_delta && pib->id >= stc->tid)
		stc->flags |= NOT_THREAD_LAST;
	return RECORD_CALLBACK_CONTINUE;
}

static post_index_board_t *search_topic(int bid, post_id_t pid, post_id_t *tid,
		int action, int *count, int *flags)
{
	record_t record;
	if (post_index_board_open(bid, RECORD_READ, &record) < 0)
		return NULL;

	post_index_board_t *pib = malloc(sizeof(*pib) * *count);
	if (!pib) {
		record_close(&record);
		return NULL;
	}

	search_topic_callback_t stc = {
		.action = action, .flags = 0, .pid = pid, .tid = *tid,
		.offset = -1, .pib = pib, .size = 0, .capacity = *count,
	};
	record_reverse_foreach(&record, search_topic_callback, &stc);

	if (action != THREAD_PREV_PAGE && stc.offset >= 0) {
		record_foreach(&record, stc.pib, stc.offset,
				search_topic_posts, &stc);
	}

	record_close(&record);
	if (!stc.size) {
		free(pib);
		return NULL;
	}
	*tid = stc.tid;
	*count = stc.size;
	*flags = stc.flags;
	return pib;
}

enum {
	POSTS_PER_PAGE = 20,
};

int bbstcon_main(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	set_current_board(board.id);

	post_id_t tid = strtoll(get_param("g"), NULL, 10);
	post_id_t pid = strtoll(get_param("f"), NULL, 10);
	char action = *(get_param("a"));
	if (!tid)
		tid = pid;

	int count = POSTS_PER_PAGE;
	int c = count, flags = 0;
	post_index_board_t *pib =
			search_topic(board.id, pid, &tid, action, &c, &flags);
	if (!pib)
		return BBS_ENOFILE;

	bool anony = board.flag & BOARD_ANONY_FLAG;
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

	brc_initialize(currentuser.userid, board.name);

	bool asc = action != THREAD_PREV_PAGE;
	if (c > count)
		c = count;

	post_index_record_t pir;
	post_index_record_open(&pir);

	for (post_index_board_t *ptr = asc ? pib : pib + c - 1;
			asc ? ptr < pib + c : ptr >= pib;
			ptr += asc ? 1 : -1) {
		post_info_t pi;
		post_index_board_to_info(&pir, ptr, &pi, 1);
		printf("<po fid='%"PRIdPID"' owner='%s'%s>", pi.id, pi.owner,
				!isbm && (pi.flag & POST_FLAG_LOCKED) ? " nore='1'" : "");

		char buffer[4096];
		char *content = post_content_get(pi.id, buffer, sizeof(buffer));
		xml_print_post_wrapper(content, strlen(content));
		if (content != buffer)
			free(content);

		puts("</po>");
		brc_mark_as_read(pi.stamp);
	}
	puts("</bbstcon>");
	post_index_record_close(&pir);

	brc_update(currentuser.userid, board.name);
	return 0;
}

int web_sigopt(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	bool hidesig = streq(get_param("hidesig"), "on");
	bool hideimg = streq(get_param("hideimg"), "on");

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
	convert(env_g2u, text, CONVERT_ALL, dst, size - header_len, NULL, NULL);
	size_t left = size - strlen(out);
	if (left > (end - ptr) + mark_len) {
		dst = out + size - left;
		memcpy(dst, ptr, end - ptr);
		dst += end - ptr;
		memcpy(dst, buf, mark_len);
		dst += mark_len;
		*dst = '\0';
	}

	int ret = post_content_write(pid, out, dst - out);
	free(out);
	return ret > 0 ? 0 : BBS_EINTNL;
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
	if (parse_post_data() < 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (!has_post_perm(&board))
		return BBS_EPST;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	bool isedit = (*(get_param("e")) == '1');

	GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
	if (!isedit) {
		if (request_type(REQUEST_UTF8)) {
			convert_u2g(get_param("title"), gbk_title);
		} else {
			strlcpy(gbk_title, get_param("title"), sizeof(gbk_title));
		}
		printable_filter(gbk_title);
		valid_title(gbk_title);
		if (*gbk_title == '\0')
			return BBS_EINVAL;
	}
	convert_g2u(gbk_title, utf8_title);

	time_t now = fb_time();
	int diff = now - get_my_last_post_time();
	set_my_last_post_time(now);
	if (diff < 6)
		return BBS_EPFREQ;

	post_id_t pid = 0;
	post_info_t pi;

	const char *f = get_param("f");
	bool reply = !(*f == '\0');

	if (isedit || reply) {
		pid = strtol(f, NULL, 10);
		if (!pid || !search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		if (!am_bm(&board) && session_uid() != pi.uid) {
			if (!isedit && (pi.flag & POST_FLAG_LOCKED))
				return BBS_EPST;
			if (isedit)
				return BBS_EACCES;
		}
	}

	char *text = (char *) get_param("text");
	check_character(text);

	if (isedit) {
		char buffer[4096];
		char *content = post_content_get(pi.id, buffer, sizeof(buffer));

		int ret = edit_article(pid, content, text, fromhost);

		if (content != buffer)
			free(content);
		if (ret < 0)
			return BBS_EINTNL;
	} else {
		post_request_t pr = {
			.autopost = false,
			.crosspost = false,
			.uname = NULL,
			.nick = NULL,
			.user = &currentuser,
			.board = &board,
			.title = utf8_title,
			.content = text,
			.sig = strtol(get_param("sig"), NULL, 0),
			.ip = mask_host(fromhost),
			.reid = reply ? pi.id : 0,
			.tid = reply ? pi.tid : 0,
			.marked = false,
			.locked = reply && (pi.flag & POST_FLAG_LOCKED),
			.anony = strtol(get_param("anony"), NULL, 0),
			.web = true,
			.cp = request_type(REQUEST_UTF8) ? NULL : env_g2u,
		};
		pid = publish_post(&pr);
		if (!pid)
			return BBS_EINTNL;
	}

	if (!isedit && !(board.flag & BOARD_JUNK_FLAG)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[128];
	snprintf(buf, sizeof(buf), "%sed '%s' on %s", isedit ? "edit" : "post",
			gbk_title, board.name);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "%sdoc?board=%s", get_post_list_type_string(),
			board.name);
	if (request_type(REQUEST_MOBILE)) {
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
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	const char *f = get_param("f");
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
		if (isedit && !am_bm(&board) && session_uid() != pi.uid)
			return BBS_EACCES;
	}
	
	xml_header(NULL);
	char path[HOMELEN];
	snprintf(path, sizeof(path), BBSHOME"/upload/%s", board.name);
	bool anony = board.flag & BOARD_ANONY_FLAG;
	printf("<bbspst brd='%s' bid='%d' edit='%d' att='%d' anony='%d'>",
			board.name, board.id, isedit, dashd(path), anony);
	print_session();

	if (reply) {
		printf("<t>");

		bool utf8 = request_type(REQUEST_UTF8);

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		char *title = utf8 ? pi.utf8_title : gbk_title;
		if (!utf8)
			convert_u2g(pi.utf8_title, gbk_title);
		ansi_filter(title, title);
		xml_fputs(title);

		printf("</t><po f='%lu'>", pid);

		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));
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
					convert(env_u2g, begin, end - begin, gbk_content, len,
							NULL, NULL);
					xml_fputs(gbk_content);
					free(gbk_content);
				}
			}
		} else {
			if (utf8) {
				quote_string(utf8_content, len, NULL, QUOTE_AUTO, false,
						true, xml_fputs3);
			} else {
				char *gbk_content = malloc(len + 1);
				convert(env_u2g, utf8_content, len, gbk_content, len,
						NULL, NULL);
				quote_string(gbk_content, strlen(gbk_content), NULL,
						QUOTE_AUTO, false, false, xml_fputs3);
				free(gbk_content);
			}
		}

		if (utf8_content != buffer)
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

	parse_post_data();

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = strtoll(get_param("f"), NULL, 10);

	post_info_t pi;
	if (!search_pid(board.id, pid, &pi))
		return BBS_ENOFILE;

	const char *target = get_param("t");
	if (*target != '\0') {
		board_t to;
		if (!get_board(target, &to))
			return BBS_ENOBRD;
		if ((to.flag & BOARD_DIR_FLAG) || to.id == board.id)
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

		char buffer[4096];
		char *content = post_content_get(pi.id, buffer, sizeof(buffer));

		post_request_t pr = {
			.autopost = false,
			.crosspost = true,
			.uname = NULL,
			.nick = NULL,
			.user = &currentuser,
			.board = &to,
			.title = utf8_title,
			.content = content,
			.sig = 0,
			.ip = mask_host(fromhost),
			.reid = 0,
			.tid = 0,
			.locked = false,
			.marked = false,
			.anony = false,
			.web = true,
			.cp = NULL,
		};
		int ret = publish_post(&pr);

		if (content != buffer)
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
				pi.owner, board.name, board.id, pid);

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
	parse_post_data();
	const char *reci = get_param("u");
	if (*reci == '\0') {
		xml_header(NULL);
		printf("<bbsfwd bid='%s' f='%s'>",
				get_param("bid"), get_param("f"));
		print_session();
		printf("</bbsfwd>");
	} else {
		if (!HAS_PERM(PERM_MAIL))
			return BBS_EACCES;

		board_t board;
		if (!get_board_by_param(&board))
			return BBS_ENOBRD;
		if (board.flag & BOARD_DIR_FLAG)
			return BBS_EINVAL;

		post_id_t pid = strtoll(get_param("f"), NULL, 10);
		post_info_t pi;
		if (!search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
		snprintf(utf8_title, sizeof(utf8_title), "[转寄]%s", pi.utf8_title);
		convert_u2g(utf8_title, gbk_title);

		char buffer[4096];
		char *content = post_content_get(pi.id, buffer, sizeof(buffer));

		char file[HOMELEN];
		int ret = dump_content_to_gbk_file(content, CONVERT_ALL, file,
				sizeof(file));

		if (content != buffer)
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
