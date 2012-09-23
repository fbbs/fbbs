#include <stdio.h>
#include <stdbool.h>
#include "bbs.h"
#include "record.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"

const char *pid_to_base32(post_id_t pid, char *s, size_t size)
{
	if (!pid) {
		return "a";
	} else {
		char buf[PID_BUF_LEN];
		char *p = buf, *r = s;
		while (pid) {
			char c = pid & 0x1f;
			if (c < 26)
				c += 'a';
			else
				c += '2' - 26;

			*p++ = c;
			pid >>= 5;
		}

		--p;
		if (p > buf + size - 2)
			p = buf + size - 2;
		for (; p >= buf; --p) {
			*s++ = *p;
		}
		*s = '\0';
		return r;
	}
}

post_id_t base32_to_pid(const char *s)
{
	post_id_t pid = 0;
	for (char c = *s; (c = *s); ++s) {
		char d = 0;
		if (c >= 'a' && c <= 'z')
			d = c - 'a';
		else if (c >= '2' && c <= '7')
			d = c - '2' + 26;
		else
			return 0;

		pid <<= 5;
		pid += d;
	}
	return pid;
}

/**
 * Creates a new file in specific location.
 * @param[in] dir The directory.
 * @param[in] pfx Prefix of the file.
 * @param[in, out] fname The resulting filename.
 * @param[in] size The size of fname.
 * @return Filename and stream on success, NULL on error.
 * @see ::date_to_fname.
 */
static FILE *get_fname(const char *dir, const char *pfx,
		char *fname, size_t size)
{
	if (dir == NULL || pfx == NULL)
		return NULL;
	const char c[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA";
	int t = (int)time(NULL);
	int count = snprintf(fname, size, "%s%s%d. ", dir, pfx, t);
	if (count < 0 || count >= size)
		return NULL;
	int fd;
	for (int i = sizeof(c) - 2; i >= 0; ++i) {
		fname[count - 1] = c[i];
		if ((fd = open(fname, O_CREAT | O_RDWR | O_EXCL, 0644)) > 0) {
			FILE *fp = fdopen(fd, "w+");
			if (fp) {
				return fp;
			} else {
				close(fd);
				return NULL;
			}
		}
	}
	return NULL;
}

static char *convert_file_to_utf8_content(const char *file)
{
	char *utf8_content = NULL;
	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) == 0) {
		utf8_content = malloc(m.size * 2);
		convert_g2u(m.ptr, utf8_content);
		mmap_close(&m);
	}
	return utf8_content;
}

static char *generate_content(const post_request_t *pr, const char *uname,
		const char *nick, const char *ip, bool anony)
{
	char dir[HOMELEN];
	snprintf(dir, sizeof(dir), "boards/%s/", pr->board->name);
	const char *pfx = "M.";

	char fname[HOMELEN];
	FILE *fptr;
	if ((fptr = get_fname(dir, pfx, fname, sizeof(fname))) == NULL)
		return NULL;

	fprintf(fptr, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			uname, nick, pr->board->name, pr->title, BBSNAME,
			getdatestring(time(NULL), DATE_ZH));

	if (pr->cp)
		convert_to_file(pr->cp, pr->content, CONVERT_ALL, fptr);
	else
		fputs(pr->content, fptr);

	if (!anony && pr->sig > 0)
		add_signature(fptr, uname, pr->sig);
	else
		fputs("\n--", fptr);

	if (ip) {
		char buf[2];
		fseek(fptr, -1, SEEK_END);
		fread(buf, 1, 1, fptr);
		if (buf[0] != '\n')
			fputs("\n", fptr);

		fprintf(fptr, "\033[m\033[1;%2dm※ %s:·"BBSNAME" "BBSHOST
			"·HTTP [FROM: %s]\033[m\n", 31 + rand() % 7,
			pr->crosspost ? "转载" : "来源", ip);
	}

	fclose(fptr);

	return convert_file_to_utf8_content(fname);
}

static post_id_t insert_post(const post_request_t *pr, const char *uname,
		const char *content)
{
	fb_time_t now = time(NULL);
	user_id_t uid = get_user_id(uname);

	UTF8_BUFFER(title, POST_TITLE_CCHARS);
	convert_g2u(pr->title, utf8_title);

	post_id_t pid = 0, reid, tid;
	db_res_t *r = db_query("SELECT nextval('posts_base_id_seq')");
	if (r) {
		pid = reid = tid = db_get_post_id(r, 0, 0);
		db_clear(r);
	}

	if (pid) {
		reid = pr->reid ? pr->reid : pid;
		tid = pr->tid ? pr->tid : pid;

		r = db_cmd("INSERT INTO posts (id, reid, tid, owner, stamp, board,"
				" uname, title, content, locked, marked) VALUES (%"DBIdPID","
				" %"DBIdPID", %"DBIdPID", %"DBIdUID", %t, %d, %s, %s, %s, %b,"
				" %b)", pid, reid, tid, uid, now, pr->board->id, uname,
				utf8_title, content, pr->locked, pr->marked);
		if (!r || db_cmd_rows(r) != 1)
			pid = 0;
		db_clear(r);
	}
	return pid;
}

/**
 * Publish a post.
 * @param pr The post request.
 * @return file id on success, -1 on error.
 */
post_id_t publish_post(const post_request_t *pr)
{
	if (!pr || !pr->title || (!pr->content && !pr->gbk_file) || !pr->board)
		return 0;

	bool anony = pr->anony && (pr->board->flag & BOARD_ANONY_FLAG);
	const char *uname = NULL, *nick = NULL, *ip = pr->ip;
	if (anony) {
		uname = ANONYMOUS_ACCOUNT;
		nick = ANONYMOUS_NICK;
		ip = ANONYMOUS_SOURCE;
	} else if (pr->user) {
		uname = pr->user->userid;
		nick = pr->user->username;
	} else if (pr->autopost) {
		uname = pr->uname;
		nick = pr->nick;
	}
	if (!uname || !nick)
		return 0;

	char *content;
	if (pr->gbk_file)
		content = convert_file_to_utf8_content(pr->gbk_file);
	else
		generate_content(pr, uname, nick, ip, anony);

	post_id_t pid = insert_post(pr, uname, content);
	free(content);

	if (pid) {
		set_last_post_id(pr->board->id, pid);

		if (!pr->autopost) {
			brc_fcgi_init(uname, pr->board->name);
			brc_mark_as_read(pid);
			brc_update(uname, pr->board->name);
		}
	}
	return pid;
}

enum {
	MAX_QUOTED_LINES = 5,     ///< Maximum quoted lines (for QUOTE_AUTO).
	/** A line will be truncated at this width (78 for quoted line) */
	TRUNCATE_WIDTH = 76,
};

/**
 * Find newline in [begin, end), truncate at TRUNCATE_WIDTH.
 * @param begin The head pointer.
 * @param end The off-the-end pointer.
 * @return Off-the-end pointer to the first (truncated) line.
 */
static const char *get_truncated_line(const char *begin, const char *end)
{
	const char *code = "[0123456789;";
	bool ansi = false;

	int width = TRUNCATE_WIDTH;
	if (end - begin >= 2 && *begin == ':' && *(begin + 1) == ' ')
		width += 2;

	for (const char *s = begin; s < end; ++s) {
		if (*s == '\n')
			return s + 1;

		if (*s == '\033') {
			ansi = true;
			continue;
		}

		if (ansi) {
			if (!memchr(code, *s, sizeof(code) - 1))
				ansi = false;
			continue;
		}

		if (*s & 0x80) {
			width -= 2;
			if (width < 0)
				return s;
			++s;
			if (width == 0)
				return (s + 1 > end ? end : s + 1);
		} else {
			if (--width == 0)
				return s + 1;
		}
	}
	return end;
}

/**
 * Tell if a line is meaningless.
 * @param begin The beginning of the line.
 * @param end The off-the-end pointer.
 * @return True if str is quotation of a quotation or contains only white
           spaces, false otherwise.
 */
static bool qualify_quotation(const char *begin, const char *end)
{
	const char *s = begin;
	if (end - s > 2 && (*s == ':' || *s == '>') && *(s + 1) == ' ') {
		s += 2;
		if (end - s > 2 && (*s == ':' || *s == '>') && *(s + 1) == ' ')
			return false;
	}

	while (s < end && (*s == ' ' || *s == '\t' || *s == '\r'))
		++s;

	return (s < end && *s != '\n');
}

typedef size_t (*filter_t)(const char *, size_t, FILE *);

static size_t default_filter(const char *s, size_t size, FILE *fp)
{
	return fwrite(s, size, 1, fp);
}

static const char *get_newline(const char *begin, const char *end)
{
	while (begin < end) {
		if (*begin++ == '\n')
			return begin;
	}
	return begin;
}

#define PRINT_CONST_STRING(s)  (*filter)(s, sizeof(s) - 1, fp)

static void quote_author(const char *begin, const char *lend, bool mail,
		FILE *fp, filter_t filter)
{
	const char *quser = begin, *ptr = lend;
	while (quser < lend) {
		if (*quser++ == ' ')
			break;
	}
	while (--ptr >= begin) {
		if (*ptr == ')')
			break;
	}
	++ptr;

	PRINT_CONST_STRING("\n【 在 ");
	if (ptr > quser)
		(*filter)(quser, ptr - quser, fp);
	PRINT_CONST_STRING(" 的");
	if (mail)
		PRINT_CONST_STRING("来信");
	else
		PRINT_CONST_STRING("大作");
	PRINT_CONST_STRING("中提到: 】\n");
}

/**
 * Make quotation from a string.
 * @param str String to be quoted.
 * @param size Size of the string.
 * @param output Output file. If NULL, will output to stdout (web).
 * @param mode Quotation mode. See QUOTE_* enums.
 * @param mail Whether the referenced post is a mail.
 * @param filter Output filter function.
 */
void quote_string(const char *str, size_t size, const char *output, int mode,
		bool mail, filter_t filter)
{
	FILE *fp = NULL;
	if (output) {
		if (!(fp = fopen(output, "w")))
			return;
	}

	if (!filter)
		filter = default_filter;

	const char *begin = str, *end = str + size;
	const char *lend = get_newline(begin, end);
	quote_author(begin, lend, mail, fp, filter);

	bool header = true, tail = false;
	size_t lines = 0;
	const char *ptr;
	while (1) {
		ptr = lend;
		if (ptr >= end)
			break;

		lend = get_truncated_line(ptr, end);
		if (header && *ptr == '\n') {
			header = false;
			continue;
		}

		if (lend - ptr == 3 && !memcmp(ptr, "--\n", 3)) {
			tail = true;
			if (mode == QUOTE_LONG || mode == QUOTE_AUTO)
				break;
		}

		if (!header || mode == QUOTE_ALL) {
			if ((mode == QUOTE_LONG || mode == QUOTE_AUTO)
					&& !qualify_quotation(ptr, lend)) {
				if (*(lend - 1) != '\n')
					lend = get_newline(lend, end);
				continue;
			}

			if (mode == QUOTE_SOURCE && lend - ptr > 10 + sizeof("※ 来源:·")
					&& !memcmp(ptr + 10, "※ 来源:·", sizeof("※ 来源:·"))) {
				break;
			}

			if (mode == QUOTE_AUTO) {
				if (++lines > MAX_QUOTED_LINES) {
					PRINT_CONST_STRING(": .................（以下省略）");
					break;
				}
			}

			if (mode != QUOTE_SOURCE)
				PRINT_CONST_STRING(": ");
			(*filter)(ptr, lend - ptr, fp);
			if (*(lend - 1) != '\n')
				PRINT_CONST_STRING("\n");
		}
	}
	if (fp)
		fclose(fp);
}

void quote_file_(const char *orig, const char *output, int mode, bool mail,
		filter_t filter)
{
	if (mode != QUOTE_NOTHING) {
		mmap_t m = { .oflag = O_RDONLY };
		if (mmap_open(orig, &m) == 0) {
			quote_string(m.ptr, m.size, output, mode, mail, filter);
			mmap_close(&m);
		}
	}
}

bool set_post_flag_unchecked(int bid, post_id_t pid, const char *field,
		bool on)
{
	char query[80];
	snprintf(query, sizeof(query), "UPDATE posts SET %s = %%b"
			" WHERE board = %%d AND id = %%"DBIdPID, field);
	db_res_t *res = db_cmd(query, on, bid, pid);
	int rows = res ? db_cmd_rows(res) : 0;
	db_clear(res);
	return rows;
}

int count_sticky_posts(int bid)
{
	db_res_t *r = db_query("SELECT count(*) FROM posts"
			" WHERE board = %d AND sticky", bid);
	int rows = r ? db_get_bigint(r, 0, 0) : 0;
	db_clear(r);
	return rows;
}

bool sticky_post_unchecked(int bid, post_id_t pid, bool sticky)
{
	if (sticky) {
		int count = count_sticky_posts(bid);
		if (count >= MAX_NOTICE)
			return false;
	}
	return set_post_flag_unchecked(bid, pid, "sticky", sticky);
}

void res_to_post_info(db_res_t *r, int i, post_info_t *p)
{
	p->id = db_get_post_id(r, i, 0);
	p->reid = db_get_post_id(r, i, 1);
	p->tid = db_get_post_id(r, i, 2);
	p->uid = db_get_is_null(r, i, 3) ? 0 : db_get_user_id(r, i, 3);
	strlcpy(p->owner, db_get_value(r, i, 4), sizeof(p->owner));
	p->stamp = db_get_time(r, i, 5);
	p->flag = (db_get_bool(r, i, 6) ? POST_FLAG_DIGEST : 0)
			| (db_get_bool(r, i, 7) ? POST_FLAG_MARKED : 0)
			| (db_get_bool(r, i, 8) ? POST_FLAG_WATER : 0)
			| (db_get_bool(r, i, 9) ? POST_FLAG_LOCKED : 0)
			| (db_get_bool(r, i, 10) ? POST_FLAG_IMPORT : 0);
	p->replies = db_get_integer(r, i, 11);
	p->comments = db_get_integer(r, i, 12);
	p->score = db_get_integer(r, i, 13);
	strlcpy(p->utf8_title, db_get_value(r, i, 14), sizeof(p->utf8_title));
}

void set_post_flag(post_info_t *ip, post_flag_e flag, bool set)
{
	if (set)
		ip->flag |= flag;
	else
		ip->flag &= ~flag;
}

int _load_sticky_posts(int bid, post_info_t **posts)
{
	if (!*posts)
		*posts = malloc(sizeof(**posts) * MAX_NOTICE);

	db_res_t *r = db_query("SELECT " POST_LIST_FIELDS " FROM posts"
			" WHERE board = %d AND sticky ORDER BY id DESC", bid);
	if (r) {
		int count = db_res_rows(r);
		for (int i = 0; i < count; ++i) {
			res_to_post_info(r, i, *posts + i);
			set_post_flag(*posts + i, POST_FLAG_STICKY, true);
		}
		db_clear(r);
		return count;
	}
	return 0;
}

const char *post_table_name(post_list_type_e type)
{
	switch (type) {
		case POST_LIST_TRASH:
		case POST_LIST_JUNK:
			return "posts_deleted";
		default:
			return "posts";
	}
}

static const char *post_filter(post_list_type_e type)
{
	switch (type) {
		case POST_LIST_MARKED:
			return "marked";
		case POST_LIST_DIGEST:
			return "digest";
		case POST_LIST_AUTHOR:
			return "p.owner = %%"DBIdPID;
		case POST_LIST_KEYWORD:
			return "p.title LIKE %%s";
		case POST_LIST_TRASH:
			return "bm_visible";
		case POST_LIST_JUNK:
			return "NOT bm_visible";
		default:
			return "TRUE";
	}
}

void build_post_filter(query_builder_t *b, post_filter_t *f)
{
	query_builder_append(b, "WHERE TRUE");
	if (f->bid)
		query_builder_append_and(b, "board = %d", f->bid);
	if (f->flag & POST_FLAG_DIGEST)
		query_builder_append_and(b, "digest");
	if (f->flag & POST_FLAG_MARKED)
		query_builder_append_and(b, "marked");
	if (f->flag & POST_FLAG_WATER)
		query_builder_append_and(b, "water");
	if (f->uid)
		query_builder_append_and(b, "owner = %"DBIdUID, f->uid);
	if (f->min)
		query_builder_append_and(b, "id >= %"DBIdPID, f->min);
	if (f->max)
		query_builder_append_and(b, "id <= %"DBIdPID, f->max);
	if (f->tid)
		query_builder_append_and(b, "tid = %"DBIdPID, f->tid);
	if (*f->utf8_keyword)
		query_builder_append_and(b, "title ILIKE '%%%s%%'", f->utf8_keyword);
}

query_builder_t *build_post_query(post_list_type_e type, post_filter_t *filter,
		bool asc, int limit)
{
	query_builder_t *b = query_builder_new(0);
	query_builder_append(b, "SELECT " POST_LIST_FIELDS " FROM");
	query_builder_append(b, post_table_name(type));
	build_post_filter(b, filter);
	query_builder_append(b, "ORDER BY id");
	query_builder_append(b, asc ? "ASC" : "DESC");
	int64_t l = limit;
	query_builder_append(b, "LIMIT %l", l);
	return b;
}

void res_to_post_info_full(db_res_t *res, int row, post_info_full_t *p)
{
	res_to_post_info(res, row, &p->p);
	p->res = res;
	p->content = db_get_value(res, row, 15);
	p->length = db_get_length(res, row, 15);
}

void free_post_info_full(post_info_full_t *p)
{
	db_clear(p->res);
}

int dump_content_to_gbk_file(const char *utf8_str, size_t length, char *file,
		size_t size)
{
	snprintf(file, size, "tmp/gbk_dump.%d", getpid());
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	convert_to_file(env_u2g, utf8_str, length, fp);
	fclose(fp);
	return 0;
}

#define LAST_POST_KEY  "last_post"

bool set_last_post_id(int bid, post_id_t pid)
{
	mdb_res_t *res = mdb_cmd("HSET", LAST_POST_KEY " %d %"PRIdPID, bid, pid);
	mdb_clear(res);
	return res;
}

post_id_t get_last_post_id(int bid)
{
	return mdb_integer(0, "HGET", LAST_POST_KEY " %d", bid);
}

static void adjust_user_post_count(const char *uname, int delta)
{
	struct userec urec;
	int unum = searchuser(uname);
	getuserbyuid(&urec, unum);
	urec.numposts += delta;
	substitut_record(NULL, &urec, sizeof(urec), unum);
}

int delete_posts(post_filter_t *filter, bool junk, bool bm_visible, bool force)
{
	fb_time_t now = time(NULL);

	bool decrease = true;
	board_t board;
	if (filter->bid && get_board_by_bid(filter->bid, &board)
			&& is_junk_board(&board)) {
		decrease = false;
	}

	query_builder_t *b = query_builder_new(0);
	query_builder_append(b, "WITH rows AS ( DELETE FROM posts");
	build_post_filter(b, filter);
	if (!force)
		query_builder_append_and(b, "NOT marked");
	query_builder_append_and(b, "NOT sticky");
	query_builder_append(b, "RETURNING " POST_BASE_FIELDS_FULL ")");
	query_builder_append(b, "INSERT INTO posts_deleted ("
			POST_BASE_FIELDS_FULL ",eraser,deleted,junk,bm_visible,ename)");
	query_builder_append(b, "SELECT " POST_BASE_FIELDS_FULL ","
			" %"DBIdUID", %t, %b AND (water OR %b),"" %b, %s FROM rows",
			session.uid, now, decrease, junk, bm_visible, currentuser.userid);
	query_builder_append(b, "RETURNING owner, uname, junk");

	db_res_t *res = query_builder_query(b);
	query_builder_free(b);

	int rows = 0;
	if (res) {
		rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			user_id_t uid = db_get_user_id(res, i, 0);
			if (uid && db_get_bool(res, i, 2)) {
				const char *uname = db_get_value(res, i, 1);
				adjust_user_post_count(uname, -1);
			}
		}
		db_clear(res);
	}
	return rows;
}

int undelete_posts(post_filter_t *filter, bool bm_visible)
{
	query_builder_t *b = query_builder_new(0);
	query_builder_append(b, "SELECT owner, uname, junk FROM posts_deleted");
	build_post_filter(b, filter);
	query_builder_append_and(b, "bm_visible = %b", bm_visible);

	db_res_t *res = query_builder_query(b);
	if (res) {
		for (int i = db_res_rows(res) - 1; i >= 0; --i) {
			user_id_t uid = db_get_user_id(res, i, 0);
			if (uid && db_get_bool(res, i, 2)) {
				const char *uname = db_get_value(res, i, 1);
				adjust_user_post_count(uname, 1);
			}
		}
		db_clear(res);
	}
	query_builder_free(b);

	b = query_builder_new(0);
	query_builder_append(b, "WITH rows AS ( DELETE FROM posts_deleted");
	build_post_filter(b, filter);
	query_builder_append_and(b, "bm_visible = %b", bm_visible);
	query_builder_append(b, "RETURNING " POST_BASE_FIELDS_FULL ")");
	query_builder_append(b, "INSERT INTO posts (" POST_BASE_FIELDS_FULL ")");
	query_builder_append(b, "SELECT " POST_BASE_FIELDS_FULL " FROM rows");

	res = query_builder_cmd(b);
	int rows = res ? db_cmd_rows(res) : 0;

	db_clear(res);
	query_builder_free(b);
	return rows;
}

db_res_t *query_post_by_pid(post_list_type_e type, post_id_t pid,
		const char *fields)
{
	query_builder_t *b = query_builder_new(0);
	query_builder_append(b, "SELECT");
	query_builder_append(b, fields);
	query_builder_append(b, "FROM");
	query_builder_append(b, post_table_name(type));
	query_builder_append(b, "WHERE id = %"DBIdPID, pid);

	db_res_t *res = query_builder_query(b);
	query_builder_free(b);
	return res;
}

static char *replace_content_title(const char *content, size_t len,
		const char *title)
{
	const char *end = content + len;
	const char *l1_end = get_line_end(content, end);
	const char *l2_end = get_line_end(l1_end, end);

	// sizeof("标  题: ") in UTF-8 is 10
	const char *begin = l1_end + 10;
	int orig_title_len = l2_end - begin - 1; // exclude '\n'
	if (orig_title_len < 0)
		return NULL;

	int new_title_len = strlen(title);
	char *s = malloc(len + new_title_len - orig_title_len);
	char *p = s;
	size_t l = begin - content;
	memcpy(p, content, l);
	p += l;
	memcpy(p, title, new_title_len);
	p += new_title_len;
	*p++ = '\n';
	l = end - l2_end;
	memcpy(p, l2_end, end - l2_end);
	return s;
}

bool alter_title(post_list_type_e type, post_id_t pid, const char *title)
{
	db_res_t *res = query_post_by_pid(type, pid, "content");
	if (res && db_res_rows(res) == 1) {
		char *content = replace_content_title(db_get_value(res, 0, 0),
				db_get_length(res, 0, 0), title);
		db_clear(res);
		if (!content)
			return false;

		query_builder_t *b = query_builder_new(0);
		query_builder_append(b, "UPDATE");
		query_builder_append(b, post_table_name(type));
		query_builder_append(b, "SET title = %s, content = %s",
				title, content);
		query_builder_append(b, "WHERE id = %"DBIdPID, pid);

		res = query_builder_cmd(b);
		bool success = res;

		db_clear(res);
		query_builder_free(b);
		free(content);
		return success;
	}
	return false;
}
