#include <stdio.h>
#include <stdbool.h>
#include "bbs.h"
#include "record.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

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

/**
 * Post an article.
 * @param pr The post request.
 * @return file id on success, -1 on error.
 */
unsigned int do_post_article(const post_request_t *pr)
{
	if (!pr || !pr->title || !pr->content || !pr->board)
		return 0;

	bool anony = pr->anony && (pr->board->flag & BOARD_ANONY_FLAG);
	const char *userid = NULL, *nick = NULL, *ip = pr->ip;
	if (anony) {
		userid = ANONYMOUS_ACCOUNT;
		nick = ANONYMOUS_NICK;
		ip = ANONYMOUS_SOURCE;
	} else if (pr->user) {
		userid = pr->user->userid;
		nick = pr->user->username;
	} else if (pr->autopost) {
		userid = pr->userid;
		nick = pr->nick;
	}
	if (!userid || !nick)
		return 0;

	char dir[HOMELEN];
	int idx = snprintf(dir, sizeof(dir), "boards/%s/", pr->board->name);
	const char *pfx = "M.";

	char fname[HOMELEN];
	FILE *fptr;
	if ((fptr = get_fname(dir, pfx, fname, sizeof(fname))) == NULL)
		return 0;

	fprintf(fptr, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			userid, nick, pr->board->name, pr->title, BBSNAME,
			getdatestring(time(NULL), DATE_ZH));

	if (pr->cp)
		convert_to_file(pr->cp, pr->content, CONVERT_ALL, fptr);
	else
		fputs(pr->content, fptr);

	if (!anony && pr->sig > 0)
		add_signature(fptr, userid, pr->sig);
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
	valid_gbk_file(fname, '?');

	struct fileheader fh;
	memset(&fh, 0, sizeof(fh));	
	strlcpy(fh.filename, fname + idx, sizeof(fh.filename));
	strlcpy(fh.owner, userid, sizeof(fh.owner));
	strlcpy(fh.title, pr->title, sizeof(fh.title));

	if (pr->noreply)
		fh.accessed[0] |= FILE_NOREPLY;
	if (pr->mmark)
		fh.accessed[0] |= FILE_MARKED;

	// TODO: assure fid order in .DIR
	fh.id = get_nextid2(pr->board->id);
	if (pr->o_fp) { // reply
		fh.reid = pr->o_fp->id;
		fh.gid = pr->o_fp->gid;
	} else {
		fh.reid = fh.id;
		fh.gid = fh.id;
	}

	setwbdir(dir, pr->board->name);
	append_record(dir, &fh, sizeof(fh));
	updatelastpost(pr->board);

	if (!pr->autopost) {
		brc_fcgi_init(userid, pr->board->name);
		brc_addlist_legacy(fh.filename);
		brc_update(userid, pr->board->name);
	}

	return fh.id;
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
			| (db_get_bool(r, i, 8) ? POST_FLAG_LOCKED : 0)
			| (db_get_bool(r, i, 9) ? POST_FLAG_IMPORT : 0);
	p->replies = db_get_integer(r, i, 10);
	p->comments = db_get_integer(r, i, 11);
	p->score = db_get_integer(r, i, 12);
	strlcpy(p->utf8_title, db_get_value(r, i, 13), sizeof(p->utf8_title));
}

void set_post_flag(post_info_t *ip, post_flag_e flag, bool set)
{
	if (set)
		ip->flag |= flag;
	else
		ip->flag &= ~flag;
}

int _load_sticky_posts(post_list_filter_t *filter, post_info_t **posts)
{
	if (filter->type != POST_LIST_NORMAL)
		return 0;

	if (!*posts)
		*posts = malloc(sizeof(**posts) * MAX_NOTICE);

	db_res_t *r = db_query("SELECT " POST_LIST_FIELDS " FROM posts"
			" WHERE board = %d AND sticky ORDER BY id DESC", filter->bid);
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

static size_t post_table_name(char *table, size_t size, post_list_type_e type)
{
	const char *t;
	switch (type) {
		case POST_LIST_TRASH:
		case POST_LIST_JUNK:
			t = "posts_deleted";
			break;
		default:
			t = "posts";
			break;
	}
	return strlcpy(table, t, size);
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

int build_post_query(char *query, size_t size, post_list_type_e type, bool asc,
		int limit)
{
	char table[16];
	post_table_name(table, sizeof(table), type);

	return snprintf(query, size, "SELECT " POST_LIST_FIELDS
			" FROM %s WHERE board = %%d AND id %c %%"DBIdPID" AND %s"
			" ORDER BY id %s LIMIT %d", table, asc ? '>' : '<',
			post_filter(type), asc ? "ASC" : "DESC", limit);
}

void res_to_post_info_full(db_res_t *res, int row, post_info_full_t *p)
{
	res_to_post_info(res, row, &p->p);
	p->res = res;
	p->content = db_get_value(res, row, 14);
	p->length = db_get_length(res, row, 14);
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
	convert_to_file(env.u2g, utf8_str, length, fp);
	fclose(fp);
	return 0;
}
