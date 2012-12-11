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

char *convert_file_to_utf8_content(const char *file)
{
	char *utf8_content = NULL;
	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) == 0) {
		utf8_content = malloc(m.size * 2);
		convert(env_g2u, m.ptr, CONVERT_ALL, utf8_content, m.size * 2,
				NULL, NULL);
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

	//% "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n"
	fprintf(fptr, "\xb7\xa2\xd0\xc5\xc8\xcb: %s (%s), \xd0\xc5\xc7\xf8: %s\n\xb1\xea  \xcc\xe2: %s\n\xb7\xa2\xd0\xc5\xd5\xbe: %s (%s)\n\n",
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

		//% "\033[m\033[1;%2dm※ %s:·"
		fprintf(fptr, "\033[m\033[1;%2dm\xa1\xf9 %s:\xa1\xa4"BBSNAME" "BBSHOST
			//% "·HTTP [FROM: %s]\033[m\n"
			"\xa1\xa4HTTP [FROM: %s]\033[m\n", 31 + rand() % 7,
			//% "转载" "来源"
			pr->crosspost ? "\xd7\xaa\xd4\xd8" : "\xc0\xb4\xd4\xb4", ip);
	}

	fclose(fptr);

	return convert_file_to_utf8_content(fname);
}

#define LAST_FAKE_ID_KEY "last_fake_id"

int get_last_fake_pid(int bid)
{
	return mdb_integer(0, "HGET", LAST_FAKE_ID_KEY " %d", bid);
}

int incr_last_fake_pid(int bid, int delta)
{
	return mdb_integer(0, "HINCRBY", LAST_FAKE_ID_KEY " %d %d", bid, delta);
}

static void update_fake_pid(int bid, int delta, post_id_t min)
{
	int last = incr_last_fake_pid(bid, delta);

	post_filter_t filter = { .bid = bid, .min = min };

	query_t *q = query_new(0);
	query_append(q, "WITH rank AS ( SELECT id, %d::INTEGER - rank()"
			" OVER (ORDER BY id DESC) AS r", last + 1);
	query_from(q, post_table_name(&filter));
	build_post_filter(q, &filter, NULL);
	query_update(q, post_table_name(&filter));
	query_set(q, "fake_id = rank.r");
	query_from(q, "rank");
	query_where(q, "rank.id = p.id");

	db_res_t *res = query_cmd(q);
	db_clear(res);
}

const char *post_recent_table(int bid)
{
	static char table[24];
	if (bid) {
		snprintf(table, sizeof(table), "posts.recent_%d", bid);
		return table;
	}
	return "posts.recent";
}

static post_id_t insert_post(const post_request_t *pr, const char *uname,
		const char *content)
{
	fb_time_t now = time(NULL);
	user_id_t uid = get_user_id(uname);

	UTF8_BUFFER(title, POST_TITLE_CCHARS);
	convert_g2u(pr->title, utf8_title);

	post_id_t pid = 0, reid, tid;
	db_res_t *r = db_query("SELECT nextval('posts.base_id_seq')");
	if (r) {
		pid = reid = tid = db_get_post_id(r, 0, 0);
		db_clear(r);
	}

	if (pid) {
		reid = pr->reid ? pr->reid : pid;
		tid = pr->tid ? pr->tid : pid;
		int fake_pid = incr_last_fake_pid(pr->board->id, 1);

		query_t *q = query_new(0);
		query_append(q, "INSERT INTO");
		query_append(q, post_recent_table(pr->board->id));
		query_append(q, "(id, reid, tid, owner, stamp, board, uname, title,"
				" content, locked, marked, fake_id)");
		query_append(q, "VALUES (%"DBIdPID", %"DBIdPID", %"DBIdPID","
				" %"DBIdUID", %t, %d, %s, %s, %s, %b, %b, %d)",
				pid, reid, tid, uid, now, pr->board->id, uname,
				utf8_title, content, pr->locked, pr->marked, fake_pid);
		db_res_t *r = query_cmd(q);
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
		content = generate_content(pr, uname, nick, ip, anony);

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

	//% "\n【 在 "
	PRINT_CONST_STRING("\n\xa1\xbe \xd4\xda ");
	if (ptr > quser)
		(*filter)(quser, ptr - quser, fp);
	//% " 的"
	PRINT_CONST_STRING(" \xb5\xc4");
	if (mail)
		//% "来信"
		PRINT_CONST_STRING("\xc0\xb4\xd0\xc5");
	else
		//% "大作"
		PRINT_CONST_STRING("\xb4\xf3\xd7\xf7");
	//% "中提到: 】\n"
	PRINT_CONST_STRING("\xd6\xd0\xcc\xe1\xb5\xbd: \xa1\xbf\n");
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

			//% "※ 来源:·"
			if (mode == QUOTE_SOURCE && lend - ptr > 10 + sizeof("\xa1\xf9 \xc0\xb4\xd4\xb4:\xa1\xa4")
					//% "※ 来源:·" "※ 来源:·"
					&& !memcmp(ptr + 10, "\xa1\xf9 \xc0\xb4\xd4\xb4:\xa1\xa4", sizeof("\xa1\xf9 \xc0\xb4\xd4\xb4:\xa1\xa4"))) {
				break;
			}

			if (mode == QUOTE_AUTO) {
				if (++lines > MAX_QUOTED_LINES) {
					//% ": .................（以下省略）"
					PRINT_CONST_STRING(": .................\xa3\xa8\xd2\xd4\xcf\xc2\xca\xa1\xc2\xd4\xa3\xa9");
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

int set_post_flag(post_filter_t *filter, const char *field, bool set,
		bool toggle)
{
	query_t *q = query_new(0);
	query_update(q, post_table_name(filter));
	query_sappend(q, "SET", field);
	if (toggle)
		query_sappend(q, "= NOT", field);
	else
		query_append(q, "= %b", set);
	build_post_filter(q, filter, NULL);

	db_res_t *res = query_cmd(q);

	int rows = res ? db_cmd_rows(res) : 0;
	db_clear(res);
	return rows;
}

int count_sticky_posts(int bid)
{
	db_res_t *r = db_query("SELECT count(*) FROM posts.recent"
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

	post_filter_t filter = { .bid = bid, .min = pid, .max = pid };
	return set_post_flag(&filter, "sticky", sticky, false);
}

void res_to_post_info(db_res_t *r, int i, bool archive, post_info_t *p)
{
	bool deleted = streq(db_field_name(r, 0), "did");
	p->id = db_get_post_id(r, i, 0);
	p->reid = db_get_post_id(r, i, 1);
	p->tid = db_get_post_id(r, i, 2);
	p->fake_id = db_get_is_null(r, i, 3) ? 0 : db_get_integer(r, i, 3);
	p->bid = db_get_integer(r, i, 4);
	p->uid = db_get_is_null(r, i, 5) ? 0 : db_get_user_id(r, i, 5);
	strlcpy(p->owner, db_get_value(r, i, 6), sizeof(p->owner));
	p->stamp = db_get_time(r, i, 7);
	p->flag = (db_get_bool(r, i, 8) ? POST_FLAG_DIGEST : 0)
			| (db_get_bool(r, i, 9) ? POST_FLAG_MARKED : 0)
			| (db_get_bool(r, i, 10) ? POST_FLAG_WATER : 0)
			| (db_get_bool(r, i, 11) ? POST_FLAG_LOCKED : 0)
			| (db_get_bool(r, i, 12) ? POST_FLAG_IMPORT : 0)
			| (deleted ? POST_FLAG_DELETED : 0)
			| (archive ? POST_FLAG_ARCHIVE : 0);
	p->replies = db_get_integer(r, i, 13);
	p->comments = db_get_integer(r, i, 14);
	p->score = db_get_integer(r, i, 15);
	strlcpy(p->utf8_title, db_get_value(r, i, 16), sizeof(p->utf8_title));

	if (deleted) {
		strlcpy(p->ename, db_get_value(r, i, 17), sizeof(p->ename));
		p->estamp = db_get_time(r, i, 18);
		p->flag |= db_get_bool(r, i, 19) ? POST_FLAG_JUNK : 0;
	}
}

void set_post_flag_local(post_info_t *ip, post_flag_e flag, bool set)
{
	if (set)
		ip->flag |= flag;
	else
		ip->flag &= ~flag;
}

int load_sticky_posts(int bid, post_info_t **posts)
{
	if (!*posts)
		*posts = malloc(sizeof(**posts) * MAX_NOTICE);

	db_res_t *r = db_query("SELECT " POST_LIST_FIELDS " FROM posts.recent"
			" WHERE board = %d AND sticky ORDER BY id DESC", bid);
	if (r) {
		int count = db_res_rows(r);
		for (int i = 0; i < count; ++i) {
			res_to_post_info(r, i, 0, *posts + i);
			set_post_flag_local(*posts + i, POST_FLAG_STICKY, true);
		}
		db_clear(r);
		return count;
	}
	return 0;
}

bool is_deleted(post_list_type_e type)
{
	return type == POST_LIST_TRASH || type == POST_LIST_JUNK;
}

post_list_type_e post_list_type(const post_info_t *ip)
{
	return (ip->flag & POST_FLAG_DELETED) ? POST_LIST_TRASH : POST_LIST_NORMAL;
}

const char *post_archive_table(const post_filter_t *filter)
{
	static char table[24];
	if (filter->bid) {
		snprintf(table, sizeof(table), "posts.archive_%d", filter->bid);
		return table;
	}
	return "posts.archives";
}

/**
 * Get name of the database table by filter.
 * @param filter The post filter.
 * @return The table name.
 * @note This function will directly return partition table to the caller
 *       instead of parent table because of a 2x performance gain (as of
 *       PostgreSQL 9.2).
 */
const char *post_table_name(const post_filter_t *filter)
{
	if (filter->archive)
		return post_archive_table(filter);

	if (is_deleted(filter->type))
		return "posts.deleted";
	else
		return post_recent_table(filter->bid);
}

const char *post_table_index(const post_filter_t *filter)
{
	if (is_deleted(filter->type))
		return "did";
	else
		return "id";
}

/**
 * Generate post query for a given filter.
 * @param[out] q The query.
 * @param[in] f The post filter.
 * @param[in] asc Generate proper ORDER BY clause, or nothing if left NULL.
 */
void build_post_filter(query_t *q, const post_filter_t *f, const bool *asc)
{
	query_where(q, "TRUE");
	if (f->bid && is_deleted(f->type))
		query_and(q, "board = %d", f->bid);
	if (f->flag & POST_FLAG_DIGEST)
		query_and(q, "digest");
	if (f->flag & POST_FLAG_MARKED)
		query_and(q, "marked");
	if (f->flag & POST_FLAG_WATER)
		query_and(q, "water");
	if (f->uid)
		query_and(q, "owner = %"DBIdUID, f->uid);
	if (*f->utf8_keyword)
		query_and(q, "title ILIKE '%%' || %s || '%%'", f->utf8_keyword);
	if (f->type == POST_LIST_TOPIC)
		query_and(q, "id = tid");

	if (f->type == POST_LIST_THREAD) {
		if (f->min && f->tid) {
			query_and(q, "(tid = %"DBIdPID" AND id >= %"DBIdPID
					" OR tid > %"DBIdPID")", f->tid, f->min, f->tid);
		}
		if (f->max && f->tid) {
			query_and(q, "(tid = %"DBIdPID" AND id <= %"DBIdPID
					" OR tid < %"DBIdPID")", f->tid, f->max, f->tid);
		}
	} else {
		if (f->min) {
			query_and(q, post_table_index(f));
			query_append(q, ">= %"DBIdPID, f->min);
		}
		if (f->max) {
			query_and(q, post_table_index(f));
			query_append(q, "<= %"DBIdPID, f->max);
		}
		if (f->tid)
			query_and(q, "tid = %"DBIdPID, f->tid);
		if (f->fake_id_min)
			query_and(q, "fake_id >= %d", f->fake_id_min);
		if (f->fake_id_max)
			query_and(q, "fake_id <= %d", f->fake_id_max);
	}

	if (f->type == POST_LIST_TRASH)
		query_and(q, "AND", "bm_visible");
	if (f->type == POST_LIST_JUNK)
		query_and(q, "NOT bm_visible");

	if (asc) {
		query_append(q, "ORDER BY");
		if (f->type == POST_LIST_THREAD) {
			if (*asc)
				query_append(q, "tid,");
			else
				query_append(q, "tid DESC,");
		}
		query_append(q, post_table_index(f));
		if (!*asc)
			query_append(q, "DESC");
	}
}

static const char *post_list_fields(const post_filter_t *filter)
{
	if (is_deleted(filter->type))
		return "d"POST_LIST_FIELDS",ename,deleted,junk";
	return POST_LIST_FIELDS;
}

query_t *build_post_query(const post_filter_t *filter, bool asc, int limit)
{
	query_t *q = query_new(0);
	query_select(q, post_list_fields(filter));
	query_from(q, post_table_name(filter));
	build_post_filter(q, filter, &asc);
	query_limit(q, limit);
	return q;
}

void res_to_post_info_full(db_res_t *res, int row, bool archive,
		post_info_full_t *p)
{
	res_to_post_info(res, row, archive, &p->p);
	p->res = res;
	p->content = db_get_value(res, row, 17);
	p->length = db_get_length(res, row, 17);
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

int post_deletion_trigger(db_res_t *res, int bid, bool archive, bool deletion)
{
	int rows = db_res_rows(res);
	if (rows > 0) {
		if (bid) {
			post_id_t min = POST_ID_MAX;
			for (int i = 0; i < rows; ++i) {
				post_id_t pid = db_get_post_id(res, i, 0);
				if (pid < min)
					min = pid;
			}
			if (!archive)
				update_fake_pid(bid, deletion ? -rows : rows, min);
		}

		for (int i = 0; i < rows; ++i) {
			user_id_t uid = db_get_user_id(res, i, 1);
			if (uid && db_get_bool(res, i, 3)) {
				const char *uname = db_get_value(res, i, 2);
				adjust_user_post_count(uname, deletion ? -1 : 1);
			}
		}
	}
	return rows;
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

	query_t *q = query_new(0);
	query_append(q, "WITH rows AS ( DELETE FROM");
	query_append(q, post_table_name(filter));
	build_post_filter(q, filter, NULL);
	if (!force)
		query_and(q, "NOT marked");
	query_and(q, "NOT sticky");
	query_returning(q, POST_LIST_FIELDS_FULL ")");
	query_insert(q, "posts.deleted ", "("POST_LIST_FIELDS_FULL
			",eraser,deleted,junk,bm_visible,ename)");
	query_append(q, "SELECT " POST_LIST_FIELDS_FULL ","
			" %"DBIdUID", %t, %b AND (water OR %b),"" %b, %s FROM rows",
			session.uid, now, decrease, junk, bm_visible, currentuser.userid);
	query_returning(q, "id,owner,uname,junk");

	db_res_t *res = query_exec(q);
	int rows = post_deletion_trigger(res, filter->bid, filter->archive, true);
	db_clear(res);
	return rows;
}

int undelete_posts(post_filter_t *filter)
{
	query_t *q = query_new(0);
	query_append(q, "WITH rows AS ( DELETE FROM posts.deleted");
	build_post_filter(q, filter, NULL);
	query_returning(q, "junk,"POST_LIST_FIELDS_FULL ")");
	query_insert(q, "posts.recent", "(junk," POST_LIST_FIELDS_FULL ")");
	query_select(q, "junk,"POST_LIST_FIELDS_FULL);
	query_from(q, "rows");
	query_returning(q, "id,owner,uname,junk");

	db_res_t *res = query_exec(q);
	int rows = post_deletion_trigger(res, filter->bid, filter->archive, false);
	db_clear(res);
	return rows;
}

db_res_t *query_post_by_pid(const post_filter_t *filter, const char *fields)
{
	query_t *q = query_new(0);
	query_select(q, fields);
	query_from(q, post_table_name(filter));
	query_where(q, post_table_index(filter));
	query_append(q, "= %"DBIdPID, filter->min);

	db_res_t *res = query_exec(q);
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
	len += new_title_len - orig_title_len;
	char *s = malloc(len + 1);
	char *p = s;
	size_t l = begin - content;
	memcpy(p, content, l);
	p += l;
	memcpy(p, title, new_title_len);
	p += new_title_len;
	*p++ = '\n';
	l = end - l2_end;
	memcpy(p, l2_end, end - l2_end);
	s[len] = '\0';
	return s;
}

bool alter_title(const post_info_t *ip, const char *title)
{
	post_filter_t filter = { .min = ip->id, .type = post_list_type(ip), };
	db_res_t *res = query_post_by_pid(&filter, "content");
	if (res && db_res_rows(res) == 1) {
		char *content = replace_content_title(db_get_value(res, 0, 0),
				db_get_length(res, 0, 0), title);
		db_clear(res);
		if (!content)
			return false;

		query_t *q = query_new(0);
		query_update(q, post_table_name(&filter));
		query_append(q, "SET title = %s, content = %s", title, content);
		query_where(q, "id = %"DBIdPID, ip->id);

		res = query_cmd(q);
		bool success = res;

		db_clear(res);
		free(content);
		return success;
	}
	return false;
}

bool alter_content(const post_info_t *ip, const char *content)
{
	post_filter_t filter = { .type = post_list_type(ip), };

	query_t *q = query_new(0);
	query_update(q, post_table_name(&filter));
	query_set(q, "content = %s", content);
	query_where(q, "id = %"DBIdPID, ip->id);

	db_res_t *res = query_cmd(q);
	db_clear(res);
	return res;
}

int get_post_mark(const post_info_t *p)
{
	int mark = ' ';

	if (p->flag & POST_FLAG_DIGEST) {
		if (p->flag & POST_FLAG_MARKED)
			mark = 'b';
		else
			mark = 'g';
	} else if (p->flag & POST_FLAG_MARKED) {
		mark = 'm';
	}

	if (mark == ' ' && (p->flag & POST_FLAG_WATER))
		mark = 'w';

	if (brc_unread(p->id)) {
		if (mark == ' ')
			mark = DEFINE(DEF_NOT_N_MASK) ? '+' : 'N';
		else
			mark = toupper(mark);
	}

	return mark;
}
