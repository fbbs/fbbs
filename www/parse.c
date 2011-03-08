#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "fbbs/time.h"
#include "libweb.h"
#include "mmap.h"
#include "fbbs/web.h"

#define BEGIN_WITH(b, e, s) \
	(e - b >= sizeof(s) - 1 && strncasecmp(b, s, sizeof(s) - 1) == 0)
#define END_WITH(b, e, s) \
	(e - b >= sizeof(s) - 1 && strncasecmp(e - sizeof(s) + 1, s, sizeof(s) - 1) == 0)

typedef struct string_t {
	const char *begin;
	const char *end;
} string_t;

typedef struct ansi_color_t {
	int hl;
	int fg;
	int bg;
} ansi_color_t;

/**
 * Find line end from string.
 * @param begin The line beginning.
 * @param end Off-the-end pointer of the string.
 * @return Off-the-end pointer of the line.
 */
static const char *_get_line_end(const char *begin, const char *end)
{
	const char *s = begin;
	while (s != end && *s != '\n') {
		++s;
	}
	return (s == end ? s : s + 1);
}

static fb_time_t _parse_header_date(const string_t *line)
{
	const char *begin = memchr(line->begin, '(', line->end - line->begin);
	if (!begin || begin + sizeof("1996年04月19日11:11:11") > line->end)
		return -1;

	struct tm t;
	const char *s = begin + 1;
	t.tm_year = strtol(s, NULL, 10) - 1900;
	s += sizeof("年") + 3;
	t.tm_mon = strtol(s, NULL, 10) - 1;
	s += sizeof("月") + 1;
	t.tm_mday = strtol(s, NULL, 10);
	s += sizeof("日") + 1;
	t.tm_hour = strtol(s, NULL, 10);
	t.tm_min = strtol(s + 3, NULL, 10);
	t.tm_sec = strtol(s + 6, NULL, 10);

	time_t time = mktime(&t);
	return time;
}

static const void *_memrchr(const void *s, int c, size_t n)
{
	const unsigned char *b = s, *p = s;
	p += n;
	while (--p >= b) {
		if (*p == c)
			return p;
	}
	return NULL;
}

static const char *_memstr(const void *m, const char *s, size_t size)
{
	if (!m || !s)
		return NULL;
	const char *p = m;
	const char *e = p + size;
	size_t len = strlen(s);

	while (p != e) {
		p = memchr(p, *s, e - p);
		if (!p)
			return NULL;
		if (e - p >= len && memcmp(p, s, len) == 0)
			return p;
		++p;
	}
	return NULL;
}

static void _xml_escape(const char *begin, const char *end)
{
	for (const char *s = begin; s != end; ++s) {
		switch (*s) {
			case '<':
				fputs("&lt;", stdout);
				break;
			case '>':
				fputs("&gt;", stdout);
				break;
			case '&':
				fputs("&amp;", stdout);
				break;
			case ' ':
				fputs("&#160;", stdout);
				break;
			case '\r':
			case '\n':
				break;
			default:
				fputc(*s, stdout);
				break;
		}
	}
}

static void _print_node(const char *name, string_t *value)
{
	printf("<%s>", name);
	_xml_escape(value->begin, value->end);
	printf("</%s>", name);
}

static const char *_parse_ansi_code(const char *begin, const char *end, ansi_color_t *ansi)
{
	ansi_color_t a = { .hl = -1, .fg = -1, .bg = -1 };
	const char *s = begin + 1;
	if (s == end || *s != '[')
		return s;
	const char *last = ++s;
	for (; s != end; ++s) {
		if (*s == ';' || *s == 'm') {
			int num = strtol(last, NULL, 10);
			if (num == 0 || num == 1)
				a.hl = num;
			else if (num >= 30 && num <= 37)
				a.fg = num;
			else if (num >= 40 && num <= 47)
				a.bg = num;
			last = s + 1;
		} else if (!isdigit(*s)) {
			return s;
		}
		if (*s == 'm') {
			if (s == begin + 2) { // "\033[m"
				ansi->hl = 0;
				ansi->fg = 37;
				ansi->bg = 40;
			} else {
				if (a.hl != -1)
					ansi->hl = a.hl;
				if (a.fg != -1)
					ansi->fg = a.fg;
				if (a.bg != -1)
					ansi->bg = a.bg;
			}
			return s + 1;
		}
	}
	return end;
}

static void _print_ansi_text(const char *begin, const char *end, ansi_color_t *ansi)
{
	bool open = false;
	const char *last = begin;
	for (const char *s = begin; s != end; ++s) {
		if (*s == '\033') {
			_xml_escape(last, s);
			s =  _parse_ansi_code(s, end, ansi) - 1;
			last = s + 1;
			if (open)
				printf("</c>");
			printf("<c h='%d' f='%d' b='%d'>", ansi->hl, ansi->fg, ansi->bg);
			open = true;
		}
	}
	_xml_escape(last, end);
	if (open)
		printf("</c>");
}

static const char *_print_header(const char *begin, size_t size)
{
	const char *end = begin + size;
	string_t line = { .begin = begin };
	line.end = _get_line_end(begin, end);

	string_t owner, nick, board, title;

	if (!(owner.begin = _memstr(begin, "发信人: ", line.end - begin)))
		return begin;
	owner.begin += sizeof("发信人: ") - 1;
	if (!(owner.end = memchr(owner.begin, ' ', line.end - owner.begin)))
		return begin;

	if ((nick.begin = owner.end + 2) >= end)
		return begin;
	if (!(nick.end = _memrchr(nick.begin, ')', line.end - nick.begin)))
		return begin;

	board.begin = memchr(nick.end, ':', line.end - nick.end);
	if ((board.begin += 2) >= end)
		return begin;
	if (!(board.end = memchr(board.begin, '\033', line.end - board.begin)))
		board.end = line.end - 1;

	line.begin = line.end;
	line.end = _get_line_end(line.begin, end);

	if ((title.begin = line.begin + sizeof("标  题: ") - 1) >= end)
		return begin;
	title.end = line.end - 1;

	line.begin = line.end;
	line.end = _get_line_end(line.begin, end);
	fb_time_t date = _parse_header_date(&line);
	if (date < 0)
		return begin;

	_print_node("owner", &owner);
	_print_node("nick", &nick);
	_print_node("board", &board);

	ansi_color_t ansi = { .hl = 0, .fg = 37, .bg = 40 };
	printf("<title>");
	_print_ansi_text(title.begin, title.end, &ansi);
	printf("</title>");

	printf("<date>%s</date>", getdatestring(date, DATE_ZH));

	return line.end;
}

static const char *_get_url(const char *begin, const char *end)
{
	const char *url = "0123456789abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ~$-_.+!*')(,/:;=?@%#[]";
	const char *s = begin;
	while (s != end) {
		if (!strchr(url, *s))
			return s;
		++s;
	}
	return end;
}

static const char *_print_url(const char *begin, const char *end, int option)
{
	const char *e = _get_url(begin, end);
	printf("<a ");
	if (!(option & (PARSE_NOQUOTEIMG | PARSE_NOSIGIMG))
			&& (END_WITH(begin, e, ".jpg") || END_WITH(begin, e, ".gif")
			|| END_WITH(begin, e, ".png") || END_WITH(begin, e, "jpeg"))) {
		printf("i='i' ");
	}
	printf("href='");
	fwrite((char *)begin, e - begin, 1, stdout);
	printf("'/>");
	return e;
}

static void _print_paragraph(const char *begin, const char *end, int option)
{
	ansi_color_t ansi = { .hl = 0, .fg = 37, .bg = 40 };
	const char *s = begin;
	while (s != end) {
		const char *url = _memstr(s, "http://", end - s);
		if (url) {
			_print_ansi_text(s, url, &ansi);
			s = _print_url(url, end, option);
		} else {
			_print_ansi_text(s, end, &ansi);
			break;
		}
	}
}

static bool _is_quote(const char *begin, const char *end)
{
	return BEGIN_WITH(begin, end, ": ") || BEGIN_WITH(begin, end, "【 在 ");
}

static void _print_body(const char *begin, const char *end, int option)
{
	/*
	 * <pa m='t'></pa> text
	 * <pa m='q'></pa> quote
	 * <pa m='t'></pa> text
	 * <pa m='q'></pa> quote
	 * ...
	 * <pa m='s'></pa> signature
	 */
	printf("<pa m='t'>");
	bool in_text = true, in_signature = false, in_quote = false;

	const char *s, *e;
	for (s = begin; s != end; s = e) {
		e = _get_line_end(s, end);
		if (!in_signature) {
			if (e - s == 3 && memcmp(s, "--\n", 3) == 0) {
				if (option & PARSE_NOSIG)
					break;
				in_text = false;
				in_quote = false;
				in_signature = true;
				printf("</pa><pa m='s'><p>--</p>");
				continue;
			}
			if (_is_quote(s, e) != in_quote) {
				printf(in_quote ? "</pa><pa m='t'>" : "</pa><pa m='q'>");
				in_quote = !in_quote;
			}
		}
		fputs("<p>", stdout);
		if (e == s + 1) {
			fputs("<br/>", stdout);
		} else {
			int opt = option;
			if (!in_quote)
				opt &= ~PARSE_NOQUOTEIMG;
			if (!in_signature)
				opt &= ~PARSE_NOSIGIMG;
			_print_paragraph(s, e, opt);
		}
		fputs("</p>", stdout);
	}
	printf("</pa>");
}

int xml_print_post(const char *file, int option)
{
	if (!file)
		return -1;

	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) < 0)
		return -1;
	if (m.size <= 0) {
		mmap_close(&m);
		return 0;
	}

	const char *begin = _print_header(m.ptr, m.size);

	// skip the blank line after header
	if (begin != (const char *)m.ptr)
		++begin;
	_print_body(begin, (const char *)m.ptr + m.size, option);
	
	mmap_close(&m);
	return 0;
}
