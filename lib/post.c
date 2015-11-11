#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <sys/uio.h>
#include <wchar.h>
#include "bbs.h"
#include "mmap.h"
#include "fbbs/backend.h"
#include "fbbs/brc.h"
#include "fbbs/cfg.h"
#include "fbbs/convert.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"

#include "s11n/frontend_post.h"

int post_record_cmp(const void *p1, const void *p2)
{
	const post_record_t *r1 = p1, *r2 = p2;
	if (r1->id > r2->id)
		return 1;
	return r1->id == r2->id ? 0 : -1;
}

static int post_record_open_file(const char *file, record_perm_e rdonly,
		record_t *rec)
{
	return record_open(file, post_record_cmp, sizeof(post_record_t), rdonly,
			rec);
}

static int _post_record_open(int board_id, record_perm_e rdonly, record_t *rec)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "board/%d", board_id);
	return post_record_open_file(file, rdonly, rec);
}

static int _post_record_open_sticky(int board_id, record_perm_e rdonly,
		record_t *rec)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "board/%d.sticky", board_id);
	return post_record_open_file(file, rdonly, rec);
}

enum {
	POST_BOARD_BUF_SIZE = 50,
	POST_TRASH_BUF_SIZE = 50,
};

void post_record_to_info(const post_record_t *pr, post_info_t *pi, int count)
{
	for (int i = 0; i < count; ++i) {
		pi->flag = pr->flag;
		pi->user_id = pr->user_id;
		pi->user_id_replied = pr->user_id_replied;
		pi->id = pr->id;
		pi->reply_id = pr->reply_id;
		pi->thread_id = pr->thread_id;
		pi->delete_stamp = 0;
		pi->board_id = pr->board_id;
		strlcpy(pi->user_name, pr->user_name, sizeof(pi->user_name));
		pi->eraser_name[0] = '\0';
		strlcpy(pi->board_name, pr->board_name, sizeof(pi->board_name));
		strlcpy(pi->utf8_title, pr->utf8_title, sizeof(pi->utf8_title));

		++pr;
		++pi;
	}
}

void post_record_extended_to_info(const post_record_extended_t *pre,
		post_info_t *pi, int count)
{
	for (int i = 0; i < count; ++i) {
		post_info_t *pii = pi + i;
		post_record_to_info((const post_record_t *) (pre + i), pii, 1);
		pii->delete_stamp = pre[i].stamp;
		strlcpy(pii->eraser_name, pre[i].eraser_name,
				sizeof(pii->eraser_name));
	}
}

#define POST_RECORD_READ_HELPER(type, bufsize, converter)  \
	type read_buf[bufsize]; \
	while (size > 0) { \
		int max = bufsize; \
		max = size > max ? max : size; \
\
		int count = record_read(rec, read_buf, max); \
		if (count <= 0) \
			break; \
\
		converter(read_buf, buf, count); \
		records += count; \
		size -= max; \
	}

int post_record_read(record_t *rec, int base, post_info_t *buf, int size,
		post_list_type_e type)
{
	if (record_seek(rec, base, RECORD_SET) < 0)
		return 0;

	int records = 0;
	if (is_deleted(type)) {
		POST_RECORD_READ_HELPER(post_record_extended_t, POST_TRASH_BUF_SIZE,
				post_record_extended_to_info);
	} else {
		POST_RECORD_READ_HELPER(post_record_t, POST_BOARD_BUF_SIZE,
				post_record_to_info);
	}
	return records;
}

char *post_convert_to_utf8(const char *file)
{
	char *utf8_content = NULL;
	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) == 0) {
		if (m.size) {
			utf8_content = malloc(m.size * 2);
			convert(CONVERT_G2U, m.ptr, CONVERT_ALL, utf8_content, m.size * 2,
					NULL, NULL);
			mmap_close(&m);
		}
	}
	return utf8_content;
}

enum {
	SIGNATURE_LINE_LEN = 256,
};

static void read_signature_legacy(const char *uname, int offset, char *buffer,
		size_t size)
{
	char *out = buffer;
	size_t remain = size;
	strappend(&out, &remain, "\n--\n");
	if (offset <= 0)
		return;

	char file[HOMELEN];
	sethomefile(file, uname, "signatures");
	FILE *fp = fopen(file, "r");
	if (!fp)
		return;

	char buf[256];
	for (int i = (offset - 1) * MAXSIGLINES; i > 0; --i) {
		if (!fgets(buf, sizeof(buf), fp)) {
			fclose(fp);
			return;
		}
	}

	int blank = 0;
	for (int i = 0; i < MAXSIGLINES; ++i) {
		if (!fgets(buf, sizeof(buf), fp))
			break;
		if (buf[0] == '\n' || streq(buf, "\r\n")) {
			++blank;
		} else {
			while (blank-- > 0)
				strappend(&out, &remain, "\n");
			blank = 0;
			//% ":·" "·[FROM:"
			if (!strstr(buf, ":\xa1\xa4"BBSNAME" "BBSHOST"\xa1\xa4[FROM:"))
				strappend(&out, &remain, buf);
		}
	}
	fclose(fp);
}

static char *generate_content(const post_request_t *pr, const char *uname,
		const char *nick, const char *ip, bool anony, size_t length)
{
	UTF8_BUFFER(nick, NAMELEN);
	convert_g2u(nick, utf8_nick);

	char header[512];
	snprintf(header, sizeof(header),
			"发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			uname, utf8_nick, pr->board->name, pr->title, BBSNAME_UTF8,
			format_time(fb_time(), TIME_FORMAT_UTF8_ZH));
	int header_len = strlen(header);

	int content_len = length ? length : strlen(pr->content);
	if (pr->convert_type != CONVERT_NONE)
		content_len *= 2;

	char signature[MAXSIGLINES * SIGNATURE_LINE_LEN + 5];
	char utf8_signature[sizeof(signature) * 2 + 1];
	if (!anony && pr->sig > 0) {
		read_signature_legacy(uname, pr->sig, signature, sizeof(signature));
		convert_g2u(signature, utf8_signature);
	} else {
		strlcpy(utf8_signature, "\n--", sizeof(utf8_signature));
	}
	int signature_len = strlen(utf8_signature);

	char source[256] = { '\0' };
	if (ip) {
		char utf8_ip[80];
		convert_g2u(ip, utf8_ip);
		snprintf(source, sizeof(source), "\033[m\033[1;%2dm※ %s:·"BBSNAME_UTF8
				" "BBSHOST"·%s[FROM: %s]\033[m\n", 31 + rand() % 7,
				pr->crosspost ? "转载" : "来源", pr->web ? "HTTP " : "",
				utf8_ip);
	}
	int source_len = strlen(source);

	int total_len = header_len + content_len + signature_len + source_len + 2;
	char *content = malloc(total_len);

	memcpy(content, header, header_len);
	if (pr->convert_type != CONVERT_NONE)
		convert(pr->convert_type, pr->content, CONVERT_ALL,
				content + header_len, total_len - header_len, NULL, NULL);
	else
		strlcpy(content + header_len, pr->content, total_len - header_len);

	int len = strlen(content);
	if (len < total_len)
		memcpy(content + len, utf8_signature, total_len - len);
	len += signature_len;
	if (content[len - 1] != '\n' && len < total_len) {
		content[len++] = '\n';
	}
	if (len < total_len)
		memcpy(content + len, source, total_len - len);
	content[len + source_len] = '\0';
	return content;
}

int post_get_board_count(int board_id)
{
	return mdb_integer(0, "HGET", POST_BOARD_COUNT_KEY " %d", board_id);
}

static void post_set_board_count(int board_id, int count)
{
	mdb_integer(0, "HSET", POST_BOARD_COUNT_KEY " %d %d", board_id, count);
}

enum {
	POST_QUOTE_LINE_MAX = 5,
	POST_QUOTE_WIDTH_MAX = 140,
	/** A line will be truncated at this width (78 for quoted line) */
	TRUNCATE_WIDTH = 76,
};

/**
 * Find newline in [begin, end), truncate at TRUNCATE_WIDTH.
 * @param begin The head pointer.
 * @param end The off-the-end pointer.
 * @param utf8 是否以UTF-8编码
 * @return Off-the-end pointer to the first (truncated) line.
 */
static const char *get_truncated_line(const char *begin, const char *end,
		bool utf8)
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
			if (utf8) {
				const char *ptr = s;
				size_t size = end - s;
				wchar_t wc = next_wchar(&ptr, &size);
				if (wc == WEOF || wc == 0) {
					++s;
				} else {
					width -= fb_wcwidth(wc);
					if (width < 0)
						return s;
					s = ptr - 1;
				}
			} else {
				width -= 2;
				if (width < 0)
					return s;
				++s;
			}
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

#define PRINT_CONST_STRING(s)  filter(s, sizeof(s) - 1, fp)

static void quote_author(const char *begin, const char *lend, bool mail,
		bool utf8, FILE *fp, filter_t filter)
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

	if (utf8)
		PRINT_CONST_STRING("\n【 在 ");
	else
		PRINT_CONST_STRING("\n\xa1\xbe \xd4\xda ");
	if (ptr > quser)
		filter(quser, ptr - quser, fp);
	if (utf8)
		PRINT_CONST_STRING(" 的");
	else
		PRINT_CONST_STRING(" \xb5\xc4");
	if (mail) {
		if (utf8)
			PRINT_CONST_STRING("来信");
		else
			PRINT_CONST_STRING("\xc0\xb4\xd0\xc5");
	} else {
		if (utf8)
			PRINT_CONST_STRING("大作");
		else
			PRINT_CONST_STRING("\xb4\xf3\xd7\xf7");
	}
	if (utf8)
		PRINT_CONST_STRING("中提到: 】\n");
	else
		PRINT_CONST_STRING("\xd6\xd0\xcc\xe1\xb5\xbd: \xa1\xbf\n");
}

static const char *find_char(const char *begin, const char *end, int c)
{
	for (const char *p = begin; p < end ; ++p) {
		if (*p == c)
			return p;
	}
	return NULL;
}

static const char *reverse_find_char(const char *begin, const char *end, int c)
{
	for (const char *p = end - 1; p >= begin; --p) {
		if (*p == c)
			return p;
	}
	return NULL;
}

static void quote_author_pack(const char *begin, const char *end, FILE *fp,
		filter_t filter)
{
	const char *user_begin = begin + sizeof("发信人: ") - 1;
	const char *lend = get_newline(begin, end);
	const char *user_end = reverse_find_char(begin, lend, ',');

	begin = get_newline(lend, end);
	lend = get_newline(begin, end);

	const char *date_begin = find_char(begin, lend, '(');
	if (date_begin)
		++date_begin;
	const char *date_end = find_char(begin, lend, ')');

	PRINT_CONST_STRING("    \033[0;1;32m");
	if (user_end && user_end > user_begin)
		filter(user_begin, user_end - user_begin, fp);
	PRINT_CONST_STRING(" 于 \033[1;36m");
	if (date_begin && date_end > date_begin)
		filter(date_begin, date_end - date_begin, fp);
	PRINT_CONST_STRING("\033[0;1m 提到：\033[0m\n\n");
}

#define GBK_SOURCE  "\xa1\xf9 \xc0\xb4\xd4\xb4:\xa1\xa4"
#define UTF8_SOURCE  "※ 来源:·"

static void print_omit_string(bool utf8, filter_t filter, FILE *fp)
{
	if (utf8) {
		PRINT_CONST_STRING(": .................（以下省略）");
	} else {
		PRINT_CONST_STRING(": .................\xa3\xa8"
				"\xd2\xd4\xcf\xc2\xca\xa1\xc2\xd4\xa3\xa9");
	}
}

static const char *update_quote_width(const char *begin, const char *end,
		bool utf8, size_t *width)
{
	const char *ptr = begin;
	while (ptr < end && *width) {
		if (utf8) {
			size_t left = end - ptr;
			const char *next = ptr;
			wchar_t wc = next_wchar(&next, &left);
			if (!wc || wc == WEOF)
				return ptr;
			int w = fb_wcwidth(wc);
			if (w <= 0)
				w = next - ptr > 0 ? next - ptr : 1;
			if (*width < w)
				return ptr;
			*width -= w;
			ptr = next;
		} else {
			if (*ptr & 0x80) {
				if (*width < 2)
					return ptr;
				*width -= 2;
				ptr += 2;
			} else {
				*width -= 1;
				++ptr;
			}
		}
	}
	return ptr > end ? end : ptr;
}

/**
 * 以指定模式处理文本生成引文
 * @param str 要处理的原文字符串
 * @param size 字符串长度
 * @param fp 输出流, 如为空则使用标准输出
 * @param mode 引文模式
 * @param mail 原文是否是信件
 * @param utf8 是否以UTF-8编码
 * @param filter 输出转换函数, 如为空则使用默认输出函数
 */
void post_quote_string(const char *str, size_t size, FILE *fp,
		post_quote_e mode, bool mail, bool utf8, filter_t filter)
{
	if (!filter)
		filter = default_filter;

	const char *begin = str, *end = str + size;
	const char *lend = get_newline(begin, end);
	if (mode == POST_QUOTE_PACK || mode == POST_QUOTE_PACK_COMPACT)
		quote_author_pack(begin, end, fp, filter);
	else
		quote_author(begin, lend, mail, utf8, fp, filter);

	bool header = true;
	size_t lines = 0, width = POST_QUOTE_WIDTH_MAX;
	const char *ptr;
	while (1) {
		ptr = lend;
		if (ptr >= end)
			break;

		if (mode == POST_QUOTE_PACK || mode == POST_QUOTE_PACK_COMPACT)
			lend = get_newline(ptr, end);
		else
			lend = get_truncated_line(ptr, end, utf8);
		if (header && *ptr == '\n') {
			header = false;
			continue;
		}

		if (lend - ptr == 3 && !memcmp(ptr, "--\n", 3)) {
			if (mode == POST_QUOTE_LONG || mode == POST_QUOTE_AUTO
					|| mode == POST_QUOTE_PACK
					|| mode == POST_QUOTE_PACK_COMPACT)
				break;
		}

		if (!header || mode == POST_QUOTE_ALL) {
			if ((mode == POST_QUOTE_LONG || mode == POST_QUOTE_AUTO)
					&& !qualify_quotation(ptr, lend)) {
				if (*(lend - 1) != '\n')
					lend = get_newline(lend, end);
				continue;
			}

			if (mode == POST_QUOTE_SOURCE && !utf8
					&& lend - ptr > 10 + sizeof(GBK_SOURCE)
					&& !memcmp(ptr + 10, GBK_SOURCE, sizeof(GBK_SOURCE))) {
				break;
			}

			if (mode == POST_QUOTE_SOURCE && utf8
					&& lend - ptr > 10 + sizeof(UTF8_SOURCE)
					&& !memcmp(ptr + 10, UTF8_SOURCE, sizeof(UTF8_SOURCE))) {
				break;
			}

			if (mode == POST_QUOTE_AUTO) {
				if (++lines > POST_QUOTE_LINE_MAX) {
					print_omit_string(utf8, filter, fp);
					break;
				}
				const char *truncated_end = update_quote_width(ptr, lend,
						utf8, &width);
				if (truncated_end < lend) {
					PRINT_CONST_STRING(": ");
					filter(ptr, truncated_end - ptr, fp);
					PRINT_CONST_STRING("\n");
					print_omit_string(utf8, filter, fp);
					break;
				}
			}

			if (mode != POST_QUOTE_SOURCE && mode != POST_QUOTE_PACK
					&& mode != POST_QUOTE_PACK_COMPACT)
				PRINT_CONST_STRING(": ");
			filter(ptr, lend - ptr, fp);
			if (*(lend - 1) != '\n')
				PRINT_CONST_STRING("\n");
		}
	}
}

void post_quote_file(const char *orig, const char *output, post_quote_e mode,
		bool mail, bool utf8, filter_t filter)
{
	if (mode != POST_QUOTE_NOTHING) {
		FILE *fp = fopen(output, "w");
		if (fp) {
			mmap_t m = { .oflag = O_RDONLY };
			if (mmap_open(orig, &m) == 0) {
				post_quote_string(m.ptr, m.size, fp, mode, mail, utf8, filter);
				mmap_close(&m);
			}
			fclose(fp);
		}
	}
}

bool post_match_filter(const post_record_t *pr, const post_filter_t *filter,
		int offset)
{
	bool match = true;
	if (filter->uid)
		match &= pr->user_id == filter->uid;
	if (filter->min)
		match &= pr->id >= filter->min;
	if (filter->max)
		match &= pr->id <= filter->max;
	if (filter->tid)
		match &= pr->thread_id == filter->tid;
	if (filter->flag)
		match &= (pr->flag & filter->flag) == filter->flag;
	if (filter->offset_min)
		match &= offset >= filter->offset_min - 1;
	if (filter->offset_max)
		match &= offset < filter->offset_max;
	if (filter->type == POST_LIST_TOPIC)
		match &= pr->id == pr->thread_id;
	if (*filter->utf8_keyword) {
		match &= (bool) strcasestr(pr->utf8_title, filter->utf8_keyword);
	}
	return match;
}

bool is_deleted(post_list_type_e type)
{
	return type == POST_LIST_TRASH || type == POST_LIST_JUNK;
}

int post_dump_gbk_file(const char *utf8_str, size_t length, char *file,
		size_t size)
{
	if (!utf8_str || !file)
		return -1;

	file_temp_name(file, size);
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	convert_to_file(CONVERT_U2G, utf8_str, length, fp);
	fclose(fp);
	return 0;
}

/** 版面最新一篇文章的时间 @mdb_hash */
#define LAST_POST_KEY  "last_post"

bool set_last_post_time(int bid, fb_time_t stamp)
{
	return mdb_cmd("HSET", LAST_POST_KEY " %d %"PRIdFBT, bid, stamp);
}

fb_time_t get_last_post_time(int bid)
{
	return (fb_time_t) mdb_integer(0, "HGET", LAST_POST_KEY " %d", bid);
}

/**
 * 删除版面上符合条件的文章
 * @param filter 文章过滤条件，其中必须指定版面 ID
 * @param junk 是否减少作者文章数
 * @param bm_visible 是则进入版面垃圾箱，否则进入站务垃圾箱
 * @param force 是否删除带保留标记的文章
 * @return 被删除的文章数
 */
int post_delete(const post_filter_t *filter, bool junk, bool bm_visible,
		bool force)
{
	if (!filter->bid)
		return 0;

	backend_request_post_delete_t req = {
		.filter = (post_filter_t *) filter,
		.junk = junk,
		.bm_visible = bm_visible,
		.force = force,
		.user_id = session_get_user_id(),
		.user_name = currentuser.userid,
	};
	backend_response_post_delete_t resp;
	bool ok = backend_cmd(&req, &resp, post_delete);
	return ok ? resp.deleted : 0;
}

int post_undelete(const post_filter_t *filter, bool bm_visible)
{
	if (!filter->bid)
		return 0;

	backend_request_post_undelete_t req = {
		.filter = (post_filter_t *) filter,
		.bm_visible = bm_visible,
	};
	backend_response_post_undelete_t resp;

	bool ok = backend_cmd(&req, &resp, post_undelete);
	return ok ? resp.undeleted : 0;
}

int post_mark_raw(fb_time_t stamp, int flag)
{
	int mark = ' ';

	if (flag & POST_FLAG_DIGEST) {
		if (flag & POST_FLAG_MARKED)
			mark = 'b';
		else
			mark = 'g';
	} else if (flag & POST_FLAG_MARKED) {
		mark = 'm';
	}

	if (mark == ' ' && (flag & POST_FLAG_WATER))
		mark = 'w';

	if (stamp && brc_unread(stamp)) {
		if (mark == ' ')
			mark = DEFINE(DEF_NOT_N_MASK) ? '+' : 'N';
		else
			mark = toupper(mark);
	}

	return mark;
}

int post_mark(const post_info_t *p)
{
	return post_mark_raw(post_stamp(p->id), p->flag);
}

fb_time_t post_stamp(post_id_t id)
{
	return (fb_time_t) ((id >> 21) / 1000);
}

post_id_t post_id_from_stamp(fb_time_t stamp)
{
	return (((post_id_t) stamp) * 1000) << 21;
}

/**
 * Publish a post.
 * @param pr The post request.
 * @return file id on success, -1 on error.
 */
post_id_t post_new(const post_request_t *pr)
{
	if (!pr || !pr->title || (!pr->content && !pr->gbk_file) || !pr->board)
		return 0;

	bool anony = pr->anony && (pr->board->flag & BOARD_FLAG_ANONY);
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
		content = post_convert_to_utf8(pr->gbk_file);
	else
		content = generate_content(pr, uname, nick, ip, anony, pr->length);
	if (!content)
		return 0;

	backend_request_post_new_t req = {
		.reply_id = pr->reid,
		.thread_id = pr->tid,
		.title = pr->title,
		.user_name = uname,
		.board_name = pr->board->name,
		.content = content,
		.board_id = pr->board->id,
		.user_id = session_get_user_id(),
		.user_id_replied = pr->uid_replied,
		.marked = pr->marked,
		.locked = pr->locked,
		.hide_user_id = pr->hide_user_id,
		.anonymous = anony,
	};
	backend_response_post_new_t resp;
	bool ok = backend_cmd(&req, &resp, post_new);

	free(content);

	if (!ok)
		return 0;
	if (resp.id) {
		if (!pr->autopost) {
			fb_time_t stamp = post_stamp(resp.id);
			brc_init(uname, pr->board->name);
			brc_mark_as_read(stamp);
			brc_sync(uname);
		}
	}
	return resp.id;
}

/** 版面文章记录缓存是否失效 @mdb_hash */
#define POST_RECORD_INVALIDITY_KEY  "post_record_invalidity"

void post_record_invalidity_change(int board_id, int delta)
{
	if (board_id > 0) {
		mdb_cmd("HINCRBY", POST_RECORD_INVALIDITY_KEY" %d %d",
				board_id, delta);
	}
}

int post_record_invalidity_get(int board_id)
{
	return mdb_integer(0, "HGET", POST_RECORD_INVALIDITY_KEY" %d", board_id);
}

void post_record_from_query(db_res_t *res, int row, post_record_t *post,
		bool sticky)
{
	post->id = db_get_post_id(res, row, 0);
	post->reply_id = db_get_post_id(res, row, 1);
	post->thread_id = db_get_post_id(res, row, 2);
	post->user_id = db_get_user_id(res, row, 3);
	post->user_id_replied = db_get_user_id(res, row, 4);
	post->board_id = db_get_integer(res, row, 7);
	post->flag = (db_get_bool(res, row, 9) ? POST_FLAG_DIGEST : 0)
			| (db_get_bool(res, row, 10) ? POST_FLAG_MARKED : 0)
			| (db_get_bool(res, row, 11) ? POST_FLAG_LOCKED : 0)
			| (db_get_bool(res, row, 12) ? POST_FLAG_IMPORT : 0)
			| (db_get_bool(res, row, 13) ? POST_FLAG_WATER : 0)
			| (sticky ? POST_FLAG_STICKY : 0);
	string_copy_allow_null(post->user_name, db_get_value(res, row, 6),
			sizeof(post->user_name));
	string_copy_allow_null(post->utf8_title, db_get_value(res, row, 15),
			sizeof(post->utf8_title));

	post->board_name[0] = '\0';
}

static void convert_post_record_extended(db_res_t *res, int row,
		post_record_extended_t *post)
{
	post->basic.flag |= db_get_bool(res, row, 19) ? POST_FLAG_JUNK : 0;
	post->bm_visible = db_get_bool(res, row, 20);
	post->eraser_id = db_get_user_id(res, row, 17);
	post->stamp = db_get_time(res, row, 16);
	string_copy_allow_null(post->eraser_name, db_get_value(res, row, 18),
			sizeof(post->eraser_name));
}

int post_record_compare(const void *ptr1, const void *ptr2)
{
	const post_record_t *p1 = ptr1, *p2 = ptr2;
	if (p1->id > p2->id)
		return 1;
	return p1->id == p2->id ? 0 : -1;
}

static int post_sticky_compare(const void *ptr1, const void *ptr2)
{
	return -post_record_compare(ptr1, ptr2);
}

static bool update_record(record_t *rec, int bid, bool sticky)
{
	query_t *q = query_new(0);
	query_select(q, POST_TABLE_FIELDS);
	query_from(q, "post.recent");
	query_where(q, "board_id = %d", bid);
	if (sticky)
		query_and(q, "sticky");
	db_res_t *res = query_exec(q);
	if (!res)
		return false;

	int rows = db_res_rows(res);
	post_record_t *posts = malloc(sizeof(*posts) * rows);
	if (posts) {
		for (int i = 0; i < rows; ++i) {
			post_record_from_query(res, i, posts + i, sticky);
		}
		qsort(posts, rows, sizeof(*posts),
				sticky ? post_sticky_compare : post_record_compare);
		record_write(rec, posts, rows, 0);
		record_truncate(rec, rows);
		free(posts);
	}

	db_clear(res);
	if (!sticky)
		post_set_board_count(bid, rows);
	return true;
}

/**
 * 更新版面文章记录缓存
 * @param[in] board_id 版面ID
 * @param[in] force 强制更新
 * @return 成功更新返回true, 无须更新或者出错返回false
 */
bool post_update_record(int board_id, bool force)
{
	bool updated = false;
	int invalid = 0;
	if (force || (invalid = post_record_invalidity_get(board_id))) {
		record_t record;
		if (_post_record_open(board_id, RECORD_WRITE, &record) >= 0) {
			if (record_try_lock_all(&record, RECORD_WRLCK) == 0) {
				updated = update_record(&record, board_id, false);
				if (invalid)
					post_record_invalidity_change(board_id, -invalid);
				record_lock_all(&record, RECORD_UNLCK);
			}
			record_close(&record);
		}
	}
	return updated;
}

int post_record_open(int board_id, record_t *record)
{
	int fd = _post_record_open(board_id, RECORD_READ, record);
	post_update_record(board_id, fd < 0);
	if (fd < 0)
		return _post_record_open(board_id, RECORD_READ, record);
	return fd;
}

bool post_update_sticky_record(int board_id)
{
	bool updated = false;
	record_t record;
	if (_post_record_open_sticky(board_id, RECORD_WRITE, &record) >= 0) {
		if (record_try_lock_all(&record, RECORD_WRLCK) == 0) {
			updated = update_record(&record, board_id, true);
			record_lock_all(&record, RECORD_UNLCK);
		}
		record_close(&record);
	}
	return updated;
}

int post_record_open_sticky(int board_id, record_t *record)
{
	int fd = _post_record_open_sticky(board_id, RECORD_READ, record);
	if (fd >= 0)
		return fd;
	post_update_sticky_record(board_id);
	return _post_record_open_sticky(board_id, RECORD_READ, record);
}

bool post_update_trash_record(record_t *record, post_trash_e trash,
		int board_id)
{
	query_t *q = query_new(0);
	query_select(q, POST_TABLE_FIELDS "," POST_TABLE_DELETED_FIELDS);
	query_from(q, "post.deleted");
	query_where(q, "board_id = %d", board_id);
	if (trash == POST_TRASH)
		query_and(q, "bm_visible");
	else
		query_and(q, "NOT bm_visible");

	db_res_t *res = query_exec(q);
	if (!res)
		return false;

	int rows = db_res_rows(res);
	post_record_extended_t *posts = malloc(sizeof(*posts) * rows);
	if (posts) {
		for (int i = 0; i < rows; ++i) {
			post_record_from_query(res, i, (post_record_t *) (posts + i),
					false);
			convert_post_record_extended(res, i, posts + i);
		}
		qsort(posts, rows, sizeof(*posts), post_record_compare);
		record_write(record, posts, rows, 0);
		record_truncate(record, rows);
		free(posts);
	}

	db_clear(res);
	return true;
}

int post_record_open_trash(int board_id, post_trash_e trash, record_t *record)
{
	char file[HOMELEN];
	file_temp_name(file, sizeof(file));
	int ret = record_open(file, post_record_cmp,
			sizeof(post_record_extended_t), RECORD_WRITE, record);
	unlink(file);

	if (ret >= 0)
		post_update_trash_record(record, trash, board_id);
	return ret;
}

int post_set_flag(const post_filter_t *filter, post_flag_e flag, bool set,
		bool toggle)
{
	backend_request_post_set_flag_t req = {
		.filter = (post_filter_t *) filter,
		.flag = flag,
		.set = set,
		.toggle = toggle,
	};

	backend_response_post_set_flag_t resp;
	bool ok = backend_cmd(&req, &resp, post_set_flag);
	return ok ? resp.affected : 0;
}

int post_sticky_count(int board_id)
{
	db_res_t *res = db_query("SELECT COUNT(*) FROM post.recent"
			" WHERE sticky AND board_id = %d", board_id);
	int count = 0;
	if (res && db_res_rows(res) >= 1) {
		count = db_get_bigint(res, 0, 0);
	}
	db_clear(res);
	return count;
}

enum {
	POST_CONTENT_CACHE_DIRECTORIES = 1000,
};

char *post_content_cache_filename(post_id_t post_id, char *file,
		size_t size)
{
	snprintf(file, size, "post/%03"PRIdPID"/%"PRIdPID,
			post_id % POST_CONTENT_CACHE_DIRECTORIES,
			post_id / POST_CONTENT_CACHE_DIRECTORIES);
	return file;
}

char *post_content_deleted_filename(post_id_t post_id, char *file,
		size_t size)
{
	snprintf(file, size, "post/%03"PRIdPID"/%"PRIdPID"~",
			post_id % POST_CONTENT_CACHE_DIRECTORIES,
			post_id / POST_CONTENT_CACHE_DIRECTORIES);
	return file;
}

static bool post_content_update_cache(const char *file, const char *str,
		bool force)
{
	int mode = O_WRONLY | O_CREAT;
	if (!force)
		mode |= O_EXCL;

	int fd = open(file, mode, 0644);
	if (fd < 0)
		return false;

	bool ok = false;
	if (file_lock_all(fd, FILE_WRLCK) == 0) {
		size_t size = strlen(str);
		if (file_write(fd, str, size) == size)
			ok = true;
		(void) file_lock_all(fd, FILE_UNLCK);
	}
	close(fd);
	return ok;
}

char *post_content_get(post_id_t post_id, bool read_deleted)
{
	char file[HOMELEN];
	post_content_cache_filename(post_id, file, sizeof(file));

	char *str = file_read_all(file);
	if (str)
		return str;

	char file2[HOMELEN];
	post_content_deleted_filename(post_id, file2, sizeof(file2));
	bool deleted = dashf(file2);

	if (!read_deleted && deleted)
		return NULL;

	query_t *q = query_new(0);
	query_select(q, "content");
	query_from(q, "post.content");
	query_where(q, "post_id = %"DBIdPID, post_id);

	db_res_t *res = query_exec(q);
	if (res && db_res_rows(res) == 1) {
		const char *s = db_get_value(res, 0, 0);
		str = strdup(s);
	}
	db_clear(res);

	if (str && !deleted)
		post_content_update_cache(file, str, false);
	return str;
}

bool post_content_set(post_id_t post_id, const char *str)
{
	if (!str)
		return false;

	query_t *q = query_new(0);
	query_update(q, "post.content");
	query_set(q, "content = %s", str);
	query_where(q, "post_id = %"DBIdPID, post_id);

	bool ok = false;
	db_res_t *res = query_cmd(q);
	if (res && db_cmd_rows(res) == 1)
		ok = true;
	db_clear(res);

	if (ok) {
		char file[HOMELEN];
		post_content_cache_filename(post_id, file, sizeof(file));
		unlink(file);
	}
	return ok;
}

bool post_alter_title(int board_id, post_id_t post_id, const char *title)
{
	if (!title)
		return false;

	backend_request_post_alter_title_t req = {
		.board_id = board_id,
		.post_id = post_id,
		.title = title,
	};
	backend_response_post_alter_title_t resp;

	bool ok = backend_cmd(&req, &resp, post_alter_title);
	return ok && resp.ok;
}

char *post_reply_table_name(user_id_t user_id, char *name, size_t size)
{
	int partitions = config_get_integer("post_reply_partitions", 1);
	if (partitions < 1)
		partitions = 1;

	int partition = user_id % partitions;
	snprintf(name, size, "post.reply_%d", partition);
	return name;
}

int _post_reply_load(const char *table_name, bool unread_only,
		user_id_t user_id, post_id_t post_id, post_info_t *buf, size_t size)
{
	query_t *q = query_new(0);
	query_select(q, "post_id, reply_id, thread_id, user_id,"
			" user_name, board_id, board_name, title, is_read");
	query_from(q, table_name);
	query_where(q, "user_id_replied = %"DBIdUID, user_id);
	query_and(q, "post_id < %"DBIdPID, post_id);
	if (unread_only)
		query_and(q, "NOT is_read");
	query_orderby(q, "post_id", false);
	query_limit(q, size);

	db_res_t *res = query_exec(q);
	if (!res)
		return -1;

	int rows = db_res_rows(res);
	for (int i = 0; i < rows && i < size; ++i) {
		post_info_t *pi = buf + i;
		pi->id = db_get_post_id(res, i, 0);
		pi->reply_id = db_get_post_id(res, i, 1);
		pi->thread_id = db_get_post_id(res, i, 2);
		pi->flag = db_get_bool(res, i, 8) ? POST_FLAG_READ : 0;
		pi->user_id = db_get_user_id(res, i, 3);
		pi->board_id = db_get_integer(res, i, 5);
		string_copy_allow_null(pi->user_name, db_get_value(res, i, 4),
				sizeof(pi->user_name));
		string_copy_allow_null(pi->board_name, db_get_value(res, i, 6),
				sizeof(pi->board_name));
		string_copy_allow_null(pi->utf8_title, db_get_value(res, i, 7),
				sizeof(pi->utf8_title));
	}
	db_clear(res);
	return rows;
}

int post_reply_load(bool unread_only, user_id_t user_id, post_id_t post_id,
		post_info_t *buf, size_t size)
{
	char table_name[64];
	post_reply_table_name(user_id, table_name, sizeof(table_name));
	return _post_reply_load(table_name, unread_only, user_id, post_id, buf,
			size);
}

static int _post_reply_delete(const char *table_name, user_id_t user_id,
		post_id_t post_id)
{
	query_t *q = query_new(0);
	query_sappend(q, "DELETE FROM", table_name);
	query_where(q, "user_id_replied = %"DBIdUID" AND post_id = %"DBIdPID,
			user_id, post_id);
	db_res_t *res = query_cmd(q);
	int rows = db_cmd_rows(res);
	db_clear(res);
	return rows;
}

int post_reply_delete(user_id_t user_id, post_id_t post_id)
{
	char table_name[64];
	post_reply_table_name(user_id, table_name, sizeof(table_name));
	return _post_reply_delete(table_name, user_id, post_id);
}

#define POST_REPLY_COUNT_KEY  "post:reply_count"

/** 缓存的新回复计数 */
static int _post_reply_count;

bool post_reply_incr_count(user_id_t user_id, int delta)
{
	bool ok = mdb_cmd("HINCRBY", POST_REPLY_COUNT_KEY " %"PRIdUID" %d",
			user_id, delta);
	if (ok)
		_post_reply_count += delta;
	return ok;
}

int post_reply_get_count(user_id_t user_id)
{
	return _post_reply_count = mdb_integer(0, "HGET",
			POST_REPLY_COUNT_KEY " %"PRIdUID, user_id);
}

void post_reply_clear_count(user_id_t user_id)
{
	if (mdb_cmd("HDEL", POST_REPLY_COUNT_KEY " %"PRIdUID, user_id))
		_post_reply_count = 0;
}

int post_reply_get_count_cached(void)
{
	return _post_reply_count;
}

int post_reply_mark_as_read(post_id_t post_id, user_id_t user_id,
		bool is_reply, bool before)
{
	char table_name[64];
	if (is_reply)
		post_reply_table_name(user_id, table_name, sizeof(table_name));
	else
		post_mention_table_name(user_id, table_name, sizeof(table_name));

	query_t *q = query_new(0);
	query_update(q, table_name);
	query_set(q, "is_read = TRUE");
	query_where(q, "user_id_replied = %"DBIdUID, user_id);
	query_and(q, "NOT is_read");
	if (before)
		query_and(q, "post_id <= %"DBIdPID, post_id);
	else
		query_and(q, "post_id = %"DBIdPID, post_id);

	db_res_t *res = query_cmd(q);
	int rows = db_cmd_rows(res);
	if (!before && rows > 0) {
		if (is_reply)
			post_reply_incr_count(user_id, -rows);
		else
			post_mention_incr_count(user_id, -rows);
	}
	db_clear(res);
	return rows;
}

char *post_mention_table_name(user_id_t user_id, char *name, size_t size)
{
	int partitions = config_get_integer("post_mention_partitions", 1);
	if (partitions < 1)
		partitions = 1;

	int partition = user_id % partitions;
	snprintf(name, size, "post.mention_%d", partition);
	return name;
}

int post_mention_load(bool unread_only, user_id_t user_id, post_id_t post_id,
		post_info_t *buf, size_t size)
{
	char table_name[64];
	post_mention_table_name(user_id, table_name, sizeof(table_name));
	return _post_reply_load(table_name, unread_only, user_id, post_id, buf,
			size);
}

int post_mention_delete(user_id_t user_id, post_id_t post_id)
{
	char table_name[64];
	post_mention_table_name(user_id, table_name, sizeof(table_name));
	return _post_reply_delete(table_name, user_id, post_id);
}

#define POST_MENTION_COUNT_KEY  "post:mention_count"

/** 缓存的新提及计数 */
static int _post_mention_count;

bool post_mention_incr_count(user_id_t user_id, int delta)
{
	bool ok = mdb_cmd("HINCRBY", POST_MENTION_COUNT_KEY " %"PRIdUID" %d",
			user_id, delta);
	if (ok)
		_post_mention_count += delta;
	return ok;
}

int post_mention_get_count(user_id_t user_id)
{
	return _post_mention_count =  mdb_integer(0, "HGET",
			POST_MENTION_COUNT_KEY " %"PRIdUID, user_id);
}

void post_mention_clear_count(user_id_t user_id)
{
	if (mdb_cmd("HDEL", POST_MENTION_COUNT_KEY " %"PRIdUID, user_id))
		_post_mention_count = 0;
}

int post_mention_get_count_cached(void)
{
	return _post_mention_count;
}

static const char *next_mention(const char *begin, const char *end,
		char *user_name, size_t size)
{
	*user_name = '\0';
	bool mention = false;
	const char *at = NULL, *ptr;
	for (ptr = begin; ptr < end; ++ptr) {
		if (mention) {
			if (!isalpha(*ptr)) {
				if (*ptr != '.' && ptr - at >= 2 && ptr - at < size) {
					strlcpy(user_name, at, ptr - at + 1);
					return ptr;
				}
				mention = false;
				at = NULL;
			}
		}
		if (!mention) {
			if (*ptr == '@') {
				mention = true;
				at = ptr + 1;
			}
		}
	}
	return ptr;
}

static int process_mention(const char *user_name,
		char (*user_names)[IDLEN + 1], size_t size, int count,
		post_id_t post_id, post_mention_handler_t handler, void *args)
{
	for (size_t i = 0; i < count; ++i) {
		if (strcaseeq(user_name, user_names[i]))
			return 0;
	}

	if (count < size) {
		strlcpy(user_names[count], user_name, sizeof(user_names[count]));
		handler(user_name, post_id, args);
	}
	return 1;
}

int post_scan_for_mentions(const char *title, const char *content,
		post_id_t post_id, post_mention_handler_t handler, void *args)
{
	const char *begin = content;
	if (strneq(title, "Re: ", 4))
		begin = strstr(begin, "\n\n");
	if (!begin)
		return 0;

	// 跳过签名档
	const char *end = strstr(begin, "\n--\n");
	if (end)
		++end;
	else
		end = content + strlen(content);

	int count = 0;
	char user_names[POST_MENTION_LIMIT][IDLEN + 1] = { { '\0' } };

	const char *line_end;
	while (begin < end && (line_end = get_line_end(begin, end)) <= end) {
		// 跳过引用行
		if (line_end - begin >= 2 && *begin == ':' && begin[1] == ' ') {
			begin = line_end;
			continue;
		}

		const char *ptr = begin;
		while (ptr < line_end) {
			char user_name[IDLEN + 1];
			ptr = next_mention(ptr, line_end, user_name, sizeof(user_name));
			if (*user_name) {
				count += process_mention(user_name, user_names,
						ARRAY_SIZE(user_names), count, post_id, handler, args);
			}
		}
		begin = line_end;
	}
	return count;
}

static int clear_mention(const char *user_name, post_id_t post_id, void *args)
{
	if (streq(user_name, currentuser.userid)) {
		post_reply_mark_as_read(post_id, session_get_user_id(), false, false);
	}
	return 0;
}

void post_mark_as_read(post_id_t id, user_id_t user_id_replied,
		const char *utf8_title, const char *content)
{
	bool unread = brc_mark_as_read(post_stamp(id));
	if (unread) {
		if (user_id_replied == session_get_user_id()) {
			post_reply_mark_as_read(id, user_id_replied, true, false);
		}
		if (content) {
			post_scan_for_mentions(utf8_title, content, id,
					clear_mention, NULL);
		}
	}
}
