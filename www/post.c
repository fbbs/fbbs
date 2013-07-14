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
};

typedef struct {
	char flags;
	char action;
	bool found;
	int offset;
	post_id_t pid;
	post_id_t tid;
	post_info_t *pi;
	post_index_record_t *pir;
	post_index_board_t pib;
} bbscon_search_callback_t;

static int save_result(const post_index_board_t *pib,
		bbscon_search_callback_t *bsc, int off)
{
	bsc->found = true;
	bsc->offset = off;
	bsc->tid = pib->id - pib->tid_delta;
	memcpy(&bsc->pib, pib, sizeof(bsc->pib));
	return post_index_board_to_info(bsc->pir, pib, bsc->pi, 1);
}

static record_callback_e search_pid_callback(void *ptr, void *args, int off)
{
	const post_index_board_t *pib = ptr;
	bbscon_search_callback_t *bsc = args;

	if (!bsc->action) {
		if (pib->id <= bsc->pid) {
			if (pib->id == bsc->pid)
				save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
	} else if (bsc->action == 'p') {
		if (bsc->offset >= 0) {
			save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pib->id == bsc->pid)
			bsc->offset = off;
	} else if (bsc->action == 'b') {
		if (pib->id <= bsc->tid) {
			if (pib->id == bsc->tid)
				save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pib->id == bsc->pid)
			bsc->tid = pib->id - pib->tid_delta;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static record_callback_e search_next(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	bbscon_search_callback_t *bsc = args;

	if (bsc->action == 'n') {
		if (offset > bsc->offset) {
			save_result(pib, bsc, offset);
			return RECORD_CALLBACK_BREAK;
		}
	} else if (bsc->action == 'a') {
		if (pib->id > bsc->pid && pib->id - pib->tid_delta == bsc->tid) {
			save_result(pib, bsc, offset);
			return RECORD_CALLBACK_BREAK;
		}
	} else {
		if (pib->id > bsc->pid && pib->id - pib->tid_delta == bsc->tid) {
			bsc->flags |= NOT_THREAD_LAST_POST;
			return RECORD_CALLBACK_BREAK;
		}
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
		if (action == 'n' || action == 'a' || extra) {
			bsc.found = false;
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
		if (bsc.flags & NOT_THREAD_LAST_POST)
			bsc.flags |= THREAD_LAST_POST;
	}
	post_index_record_close(&pir);
	record_close(&record);
	return bsc.flags;
}

int search_pid(int bid, post_id_t pid, post_info_t *pi)
{
	return search(bid, pid, 0, false, pi);
}

int bbscon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = strtol(get_param("f"), NULL, 10);
	char action = *get_param("a");
//	int archive = strtol(get_param("archive"), NULL, 10);
	bool sticky = *get_param("s");

	int ret = 0;
	post_info_t pi;
	if (sticky) {
		record_t record;
		if (post_index_board_open_sticky(board.id, RECORD_READ, &record) < 0)
			return BBS_ENOFILE;
		// TODO
		record_close(&record);
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
	bool self = (session.uid == pi.uid);
	printf("<po fid='%"PRIdPID"'%s%s%s%s%s%s", pi.id,
			(pi.flag & POST_FLAG_STICKY) ? " sticky='1'" : "",
			ret & POST_FIRST ? " first='1'" : "",
			ret & POST_LAST ? " last='1'" : "",
			ret & THREAD_LAST_POST ? " tlast='1'" : "",
			(pi.flag & POST_FLAG_LOCKED) ? " nore='1'" : "",
			self || isbm ? " edit='1'" : "");
	printf(" reid='%"PRIdPID"' gid='%"PRIdPID"'>", pi.reid, pi.tid);

	char buffer[4096];
	char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));
	size_t len = strlen(utf8_content);

	char *gbk_content = malloc(len + 1);
	convert(env_u2g, utf8_content, len, gbk_content, len, NULL, NULL);

	xml_print_post_wrapper(gbk_content, strlen(gbk_content));

	free(gbk_content);
	if (utf8_content != buffer)
		free(utf8_content);

	printf("</po></bbscon>");

	brc_initialize(currentuser.userid, board.name);
	brc_mark_as_read(pi.stamp);
	brc_update(currentuser.userid, board.name);

	return 0;
}

int bbsgcon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

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

int xml_print_post_wrapper(const char *str, size_t size)
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
		xml_fputs2(str, size, stdout);
		return 0;
	}
	return xml_print_post(str, size, PARSE_NOSIG);
}

int bbsdel_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;
	int bid = strtol(get_param("bid"), NULL, 10);
	unsigned int fid = strtoul(get_param("f"), NULL, 10);
	if (fid == 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_bid(bid, &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	char file[HOMELEN];
	setwbdir(file, board.name);
	struct fileheader fh;
//	bool self = !strcmp(ptr->owner, currentuser.userid);
	bool self = false;
#if 0
	record_t r;
	record_open(file, O_RDWR, &r);
	fh.id = fid;
	struct fileheader *ptr =
			record_search(&r, &fh, sizeof(fh), bsearch, cmp_fid);
	if (ptr == NULL) {
		record_close(&r);
		return BBS_ENOFILE;
	}
	if (!self && !am_bm(&board)) {
		record_close(&r);
		return BBS_EACCES;
	}
	memcpy(&fh, ptr, sizeof(fh));
	record_delete(&r, ptr, sizeof(*ptr));
	record_close(&r);
#endif
	if (!(board.flag & BOARD_JUNK_FLAG)) {
		struct userec user;
		getuserec(fh.owner, &user);
		if (user.numposts > 0)
			user.numposts--;
		save_user_data(&user);
	}

	char buf[STRLEN];
	sprintf(buf, "deleted[www] '%u' on %s\n", fid, board.name);
	report(buf, currentuser.userid);
	strlcpy(fh.szEraser, currentuser.userid, sizeof(fh.szEraser));
	fh.timeDeleted = time(NULL);
	const char *trash = JUNK_DIR;
	if (!self && !HAS_PERM(PERM_OBOARDS)) {
		trash = TRASH_DIR;
	}
	fh.accessed[1] |= FILE_SUBDEL;
	setbfile(file, board.name, trash);
	append_record(file, &fh, sizeof(fh));
	updatelastpost(&board);

	printf("Location: doc?bid=%d\n\n", bid);
	return 0;
}

static post_id_t find_next_tid(int bid, post_id_t tid, char action)
{
	post_filter_t filter = { .bid = bid, .type = POST_LIST_TOPIC };
	if (action == 'a')
		filter.min = tid + 1;
	else
		filter.max = tid - 1;

	query_t *q = build_post_query(&filter, action == 'a', 1);
	db_res_t *res = query_exec(q);

	if (res && db_res_rows(res) >= 1) {
		post_info_t info;
		res_to_post_info(res, 0, 0, &info);
		tid = info.tid;
	}
	db_clear(res);
	return tid;
}

// action 'a' next thread
// 'b' previous thread
// 'n' next page
// 'p' previous page
// (none): first page
// TODO: THREAD_FIRST, THREAD_LAST
static post_info_full_t *bbstcon_search(int bid, post_id_t pid, post_id_t *tid,
		char action, int *count, int *flag)
{
	if (action == 'a' || action == 'b') {
		*tid = find_next_tid(bid, *tid, action);
	}

	post_filter_t filter = { .bid = bid, .tid = *tid };
	if (action == 'p')
		filter.max = pid - 1;
	else if (action == 'n')
		filter.min = pid + 1;
	bool asc = !(action == 'b' || action == 'p');

	query_t *q = query_new(0);
	query_select(q, POST_LIST_FIELDS_FULL);
	query_from(q, post_table_name(&filter));
	build_post_filter(q, &filter, &asc);
	query_limit(q, *count + 1);

	db_res_t *res = query_exec(q);
	int rows = db_res_rows(res);
	if (!rows)
		return NULL;

	if (rows <= *count) {
		if (asc)
			*flag |= THREAD_LAST_POST;
		*count = rows;
	}

	post_info_full_t *p = malloc(sizeof(*p) * rows);
	for (int i = 0; i < rows; ++i) {
		res_to_post_info_full(res, i, 0, p + i);
	}
	return p;
}

int bbstcon_main(void)
{
	board_t board;
	int bid = strtol(get_param("bid"), NULL, 10);
	if (bid <= 0) {
		get_board(get_param("board"), &board);
	} else {
		get_board_by_bid(bid, &board);
	}
	if (!board.id || !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t tid = strtoll(get_param("g"), NULL, 10);
	post_id_t pid = strtoll(get_param("f"), NULL, 10);
	char action = *(get_param("a"));
	if (!tid)
		tid = pid;

	int count = POSTS_PER_PAGE;
	int c = count, flag = 0;
	post_info_full_t *p = bbstcon_search(board.id, pid, &tid, action, &c,
			&flag);
	if (!p)
		return BBS_ENOFILE;

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();
	bool isbm = am_bm(&board);

	xml_header(NULL);
	printf("<bbstcon bid='%d' gid='%"PRIdPID"' anony='%d' page='%d'"
			" attach='%d'%s%s%s%s%s>",
			bid, tid, anony, count, maxlen(board.name),
			flag & THREAD_LAST_POST ? " last='1'" : "",
			flag & THREAD_LAST ? " tlast='1'" : "",
			flag & THREAD_FIRST ? " tfirst='1'" : "",
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");
	print_session();

	brc_initialize(currentuser.userid, board.name);

	bool asc = action != 'p';
	if (c > count)
		c = count;
	for (post_info_full_t *ip = asc ? p : p + c - 1;
			asc ? ip < p + c : ip >= p;
			ip += asc ? 1 : -1) {
		printf("<po fid='%"PRIdPID"' owner='%s'%s>", ip->p.id, ip->p.owner,
				!isbm && (ip->p.flag & POST_FLAG_LOCKED) ? " nore='1'" : "");
		xml_print_post_wrapper(ip->content, ip->length);
		puts("</po>");
		brc_mark_as_read(ip->p.stamp);
	}
	puts("</bbstcon>");
	free_post_info_full(p);

	brc_update(currentuser.userid, board.name);
	return 0;
}

int web_sigopt(void)
{
	if (!session.id)
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

static int edit_article(const char *file, const char *content, const char *ip)
{
	if (file == NULL || content == NULL || ip == NULL)
		return BBS_EINTNL;
	int fd = open(file, O_RDWR);
	if (fd < 0)
		return BBS_EINTNL;
	fb_flock(fd, LOCK_EX);
	char buf[4096];
	ssize_t bytes = read(fd, buf, sizeof(buf));
	if (bytes >= 0) {
		// skip header.
		char *ptr = buf, *e = buf + bytes;
		int n = 3;
		while (ptr != e && n >= 0) {
			if (*ptr == '\n')
				--n;
			++ptr;
		}
		int begin = ptr - buf;

		if (bytes == sizeof(buf)) {
			lseek(fd, -sizeof(buf), SEEK_END);
			bytes = read(fd, buf, sizeof(buf));
			if (bytes < sizeof(buf)) {
				fb_flock(fd, LOCK_UN);
				file_close(fd);
				return BBS_EINTNL;
			}
			e = buf + bytes;
		}
		ptr = e - 2; // skip last '\n'
		while (ptr >= buf && *ptr != '\n')
			--ptr;
		if (ptr >= buf) {
			//% if (!strncmp(ptr + 1, "\033[m\033[1;36m※ 修改", 17)) {
			if (!strncmp(ptr + 1, "\033[m\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4", 17)) {
				e = ptr + 1;
				--ptr;
				while (ptr >= buf && *ptr != '\n')
					--ptr;
			}
		}

		lseek(fd, begin, SEEK_SET);
		size_t len = strlen(content);
		size_t size = begin + len;
		int ret = file_write(fd, content, len);
		if (ret == 0 && ptr != e)
			ret = file_write(fd, ptr, e - ptr);
		//% len = snprintf(buf, sizeof(buf), "\033[m\033[1;36m※ 修改:·%s 于 "
		len = snprintf(buf, sizeof(buf), "\033[m\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4:\xa1\xa4%s \xd3\xda "
				//% "%22.22s·HTTP [FROM: %s]\033[m\n", currentuser.userid,
				"%22.22s\xa1\xa4HTTP [FROM: %s]\033[m\n", currentuser.userid,
				format_time(fb_time(), TIME_FORMAT_ZH), mask_host(ip));
		if (ret == 0)
			ret = file_write(fd, buf, len);
		size += (e - ptr) + len;
		ret = ftruncate(fd, size);
		fb_flock(fd, LOCK_UN);
		file_close(fd);
		if (ret == 0)
			return 0;
		return BBS_EINTNL;
	}
	return BBS_EINTNL;	
}

static char *_check_character(char *text)
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
	return text;
}

extern int search_pid(int bid, post_id_t pid, post_info_t *pi);

int bbssnd_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;
	if (parse_post_data() < 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_post_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	bool isedit = (*(get_param("e")) == '1');

	GBK_BUFFER(title, POST_TITLE_CCHARS);
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

		if (!am_bm(&board) && session.uid != pi.uid) {
			if (!isedit && (pi.flag & POST_FLAG_LOCKED))
				return BBS_EPST;
			if (isedit)
				return BBS_EACCES;
		}
	}

	char *text = (char *)get_param("text");
	_check_character(text);

	if (isedit) {
		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));

		char file[HOMELEN];
		dump_content_to_gbk_file(utf8_content, CONVERT_ALL, file,
				sizeof(file));

		if (utf8_content != buffer)
			free(utf8_content);

		int ret = edit_article(file, text, mask_host(fromhost));
		unlink(file);

		if (ret < 0)
			return BBS_EINTNL;
	} else {
		post_request_t pr = {
			.autopost = false, .crosspost = false, .uname = NULL, .nick = NULL,
			.user = &currentuser, .board = &board, .title = gbk_title,
			.content = text, .sig = strtol(get_param("sig"), NULL, 0),
			.ip = mask_host(fromhost), .reid = reply ? pi.id : 0,
			.tid = reply ? pi.tid : 0, .marked = false,
			.locked = reply && (pi.flag & POST_FLAG_LOCKED),
			.anony = strtol(get_param("anony"), NULL, 0),
			.cp = request_type(REQUEST_UTF8) ? env_u2g : NULL,
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

	snprintf(buf, sizeof(buf), "%sdoc?board=%s", get_doc_mode_str(),
			board.name);
	http_header();
	refreshto(1, buf);
	//% printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>发表</a>"
	printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>\xb7\xa2\xb1\xed</a>"
			//% "成功，1秒钟后自动转到<a href='%s'>版面</a>\n</body>\n</html>\n",
			"\xb3\xc9\xb9\xa6\xa3\xac""1\xc3\xeb\xd6\xd3\xba\xf3\xd7\xd4\xb6\xaf\xd7\xaa\xb5\xbd<a href='%s'>\xb0\xe6\xc3\xe6</a>\n</body>\n</html>\n",
			board.id, pid, buf);
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
	//% if (!strncmp(ptr + 1, "\033[m\033[1;36m※ 修改", 17)) {
	if (!strncmp(ptr + 1, "\033[m\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4", 17)) {
		--ptr;
		while (ptr >= *begin && *ptr != '\n')
			--ptr;
		*end = (ptr >= *begin) ? ptr : *begin;
	} else {
		*end = ptr;
	}
}

extern int search_pid(int bid, post_id_t pid, post_info_t *pi);

static int do_bbspst(bool isedit)
{
	if (!session.id)
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_post_perm(&currentuser, &board))
		return BBS_EPST;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = 0;
	post_info_t pi;

	const char *f = get_param("f");
	bool reply = !(*f == '\0');

	if (isedit && !reply)
		return BBS_EINVAL;

	if (reply) {
		pid = strtol(f, NULL, 10);
		if (!pid || !search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;
		if (!isedit && (pi.flag & POST_FLAG_LOCKED))
			return BBS_EPST;
		if (isedit && !am_bm(&board) && session.uid != pi.uid)
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

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(pi.utf8_title, gbk_title);

		ansi_filter(gbk_title, gbk_title);
		xml_fputs2(gbk_title, 0, stdout);

		printf("</t><po f='%lu'>", pid);

		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));
		size_t len = strlen(utf8_content);

		char *gbk_content = malloc(len + 1);
		convert(env_u2g, utf8_content, len, gbk_content, len, NULL, NULL);

		if (isedit) {
			const char *begin = gbk_content;
			const char *end = begin + strlen(gbk_content);
			get_post_body(&begin, &end);
			if (end > begin)
				xml_fputs2(begin, end - begin, stdout);
		} else {
			quote_string(gbk_content, strlen(gbk_content), NULL, QUOTE_AUTO,
					false, xml_fputs3);
		}

		free(gbk_content);
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

/** UTF-8 "[转载]" */
#define CP_MARK_STRING  "[\xe8\xbd\xac\xe8\xbd\xbd]"

int bbsccc_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	parse_post_data();

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = strtol(get_param("f"), NULL, 10);

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
		if (!has_post_perm(&currentuser, &to))
			return BBS_EPST;

		GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
		if (strneq(pi.utf8_title, CP_MARK_STRING,
					sizeof(CP_MARK_STRING) - 1)) {
			strlcpy(utf8_title, pi.utf8_title, sizeof(utf8_title));
		} else {
			snprintf(utf8_title, sizeof(utf8_title), CP_MARK_STRING"%s",
					pi.utf8_title);
		}
		convert_u2g(utf8_title, gbk_title);

		char buffer[4096];
		char *content = post_content_get(pi.id, buffer, sizeof(buffer));

		post_request_t pr = {
			.autopost = false, .crosspost = true, .uname = NULL, .nick = NULL,
			.user = &currentuser, .board = &to, .title = gbk_title,
			.content = content, .sig = 0, .ip = mask_host(fromhost),
			.reid = 0, .tid = 0, .locked = false, .marked = false,
			.anony = false,
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
		xml_fputs(gbk_title, stdout);

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
	if (!session.id)
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
		if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
				|| !has_read_perm(&currentuser, &board))
			return BBS_ENOBRD;
		if (board.flag & BOARD_DIR_FLAG)
			return BBS_EINVAL;

		post_id_t pid = strtoul(get_param("f"), NULL, 10);
		post_info_t pi;
		if (!search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		GBK_BUFFER(title2, POST_TITLE_CCHARS);
		convert_u2g(pi.utf8_title, gbk_title);
		//% snprintf(gbk_title2, sizeof(gbk_title2), "[转寄]%s", gbk_title);
		snprintf(gbk_title2, sizeof(gbk_title2), "[\xd7\xaa\xbc\xc4]%s", gbk_title);

		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));

		char file[HOMELEN];
		int ret = dump_content_to_gbk_file(utf8_content, CONVERT_ALL, file,
				sizeof(file));

		if (utf8_content != buffer)
			free(utf8_content);

		if (ret == 0) {
			ret = mail_file(file, reci, gbk_title2);
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
