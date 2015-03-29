#include <signal.h>
#include "bbs.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/fileio.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

/*Added by Ashinmarch on 2007.12.01*/
extern int RMSG;

static int morekey(void)
{
	int     ch;
	while (1) {
		switch (ch = egetch()) { 
			case 'q':
			case KEY_LEFT:
			case EOF:
				return KEY_LEFT;
			case ' ':
			case KEY_RIGHT:
			case KEY_PGDN:
			case Ctrl('F'): 
				return KEY_RIGHT;
			case KEY_PGUP:
			case Ctrl('B'):
				return KEY_PGUP;
			case '\r':
			case '\n':
			case KEY_DOWN:
			case 'j':
				return KEY_DOWN;
			case 'k':
			case KEY_UP:
				return KEY_UP;
			case 'h':
			case 'H':
			case '?':
				return 'H';
			case 'b':
			case 'B':
			case KEY_HOME:
				return KEY_HOME;
			case 'e':
			case 'E':
			case KEY_END:
				return KEY_END;
			case 'l':
			case 'L':
				return 'L';
			case 'g':
			case 'G':
				return 'G';
			case 'y':
			case 'Y':
			case 'n':
			case 'N':
			case 'r':
			case 'R':
			case 'c':
			case 'C':
			case 'm':
			case 'M':
				return toupper(ch);
			case '*':
				return ch;
			default:
				;
		}
	}
}

/*Add by SmallPig*/
int countln(char *fname)
{
	FILE   *fp;
	char    tmp[256];
	int     count = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		return 0;

	while (fgets(tmp, sizeof(tmp), fp) != NULL)
		count++;
	fclose(fp);
	return count;
}

enum {
	/** A ::linenum_t will be assigned for every block. */
	LINENUM_BLOCK_SIZE = 4096, 
	DEFAULT_TERM_WIDTH = 80,   ///< Default terminal width.
	TAB_STOP = 4,       ///< Columns of a tab stop.
	IS_QUOTE = 0x1,     ///< The line is part of quotation if this bit is set.
	HAS_TABSTOP = 0x2,  ///< The line has tab stop(s) if this bit is set.
	NO_QUOTE = 0x1,     ///< Show quote with normal style.
	NO_EMPHASIZE = 0x2, ///< Show emphasized text with normal style.
};

/** Line number cache structure. */
typedef struct linenum_t {
	char *offset;  ///< Pointer to the first character of the line.
	int number;    ///< Line number, starts from 0.
	int prop;      ///< Properties. See ::IS_QUOTE, ::HAS_TABSTOP.
} linenum_t;

/** Mmap_more stream structure. */
typedef struct more_file_t {
	char *buf;        ///< Starting address of the text.
	size_t size;      ///< Length of the text.
	linenum_t *ln;    ///< Starting address of line number cache.
	int ln_size;      ///< Length of line number cache.
	int total;        ///< Lines in total.
	int width;        ///< Max length of each line.
	int line;         ///< Line to fetch, starting from 0.
	char *begin;      ///< Starting address of last fetched line.
	char *end;        ///< Off-the-end pointer of last fetched line.
	int prop;         ///< Properties of last fetched line.
	int opt;          ///< Options of the more file.
} more_file_t;

typedef int (*more_open_t)(const char *, size_t size, more_file_t *);

typedef int (*more_prompt_t)(more_file_t *);

typedef int (*more_handler_t)(more_file_t *, int);

/**
 * Open a file as more stream.
 * @param file File to open.
 * @param size Length of file name buffer.
 * @param width Line width.
 * @param func A function to fill the stream with file content.
 * @return an ::more_file_t pointer on success, NULL on error.
 */
static more_file_t *more_open(const char *file, size_t size, int width,
		more_open_t func)
{
	more_file_t *more = malloc(sizeof(*more));
	if (more == NULL)
		return NULL;

	memset(more, 0, sizeof(*more));
	more->total = -1;
	more->width = width;

	if (func(file, size, more) != 0) {
		free(more);
		return NULL;
	}
	return more;
}

/**
 * Close more stream.
 * @param more The more stream.
 */
static void more_close(more_file_t *more)
{
	if (more->buf != NULL)
		free(more->buf);
	free(more);
}

/**
 * Get next line from more stream.
 * On success, d->begin, d->row, d->prop is set and returned.
 * @param[in,out] d The more stream.
 * @return Size of string, -1 on error.
 */
static ssize_t more_getline(more_file_t *d)
{
	const char *code = "[0123456789;";
	if (d->width < 2) {
		return -1;
	}
	if (d->end == NULL)
		d->end = d->buf;
	d->begin = d->end;

	// update line number cache.
	linenum_t *l = d->ln + ((d->begin - d->buf) / LINENUM_BLOCK_SIZE);
	if (l->offset == NULL) {
		l->number = d->line;
		l->offset = d->begin;
		l->prop = d->prop;
	}

	char *ptr, *begin = d->begin, *end = d->buf + d->size;
	if (begin == end)
		return 0;
	d->line++;
	bool in_esc = false;
	int len = d->width;

	d->prop &= ~HAS_TABSTOP;
	if (!(d->prop & IS_QUOTE)
			&& (!strncmp(begin, ": ", 2) || !strncmp(begin, "> ", 2)))
		d->prop |= IS_QUOTE;

	for (ptr = begin; ptr < end; ++ptr) {
		if (*ptr == '\n') {
			d->prop &= ~IS_QUOTE;
			break;
		}
		if (*ptr == '\033') {
			in_esc = true;
			continue;
		}
		if (in_esc) {
			if (!memchr(code, *ptr, strlen(code)))
				in_esc = false;
			continue;
		}
		if (*ptr == '\t') {
			d->prop |= HAS_TABSTOP;
			len -= TAB_STOP - (d->width - len) % TAB_STOP;
			if (len < 0)
				--ptr;
			if (len <= 0)
				break;
			continue;
		}
		if (*ptr & 0x80) {
			len -= 2;
			if (len < 0) {
				--ptr;
				break;
			}
			++ptr;
			if (len == 0)
				break;
		} else {
			if (--len == 0)
				break;
		}
	}

	if (++ptr >= end) {
		d->total = d->line;
		d->end = end;
		return d->end - d->begin;
	}

	if (len == 0) {
		// trailing escape sequence
		if (*ptr == '\033') {
			while (++ptr < end && memchr(code, *ptr, strlen(code)))
				;
			++ptr;
		}
		// include trailing '\n' or '\r\n'.
		if (*ptr == '\n') {
			++ptr;
			d->prop &= ~IS_QUOTE;
		} else if (*ptr == '\r' && *(ptr + 1) == '\n') {
			ptr += 2;
			d->prop &= ~IS_QUOTE;
		}
	}
	d->end = ptr;
	if (ptr >= end)
		d->total = d->line;
	return d->end - d->begin;
}

/**
 * Get the strarting address of given line.
 * @param d The more stream.
 * @param line Line number.
 * @return The strarting address of given line.
 */
static char *more_seek(more_file_t *d, int line)
{
	if (line < 0)
		line = 0;
	// find nearest row in line cache.
	linenum_t *lnptr, *lend = d->ln + d->ln_size;
	for (lnptr = d->ln; lnptr != lend; ++lnptr) {
		if (lnptr->number > line || lnptr->offset == NULL)
			break;
	}
	if (--lnptr < d->ln) {
		// line cache is empty
		lnptr = d->ln;
		lnptr->number = 0;
		lnptr->offset = d->buf;
	}

	if (d->line > line || d->line < lnptr->number) {
		d->line = lnptr->number;
		d->end = lnptr->offset;
	}
	while (d->line < line) {
		if (more_getline(d) <= 0)
			break;			
	}
	return d->begin;

}

/**
 * Get count of lines of given more stream.
 * @param d The more stream.
 * @return Count of lines.
 */
static int more_countline(more_file_t *d)
{
	if (d->total >= 0)
		return d->total;

	// Save status
	char *begin = d->begin;
	char *end = d->end;
	int line = d->line;
	int prop = d->prop;

	// Search line number cache
	linenum_t *l = d->ln + d->ln_size - 1;
	for (; l >= d->ln; --l) {
		if (l->offset != NULL)
			break;
	}
	if (l->offset != NULL) {
		d->line = l->number;
		d->end = l->offset;
	} else {
		d->line = 0;
		d->end = d->buf;
	}

	while (d->total < 0)
		more_getline(d);

	// Restore status
	d->begin = begin;
	d->end = end;
	d->line = line;
	d->prop = prop;

	return d->total;
}

/**
 * Print current line in an more stream.
 * @param d The more stream.
 */
static void more_puts(more_file_t *d)
{
	char *ptr;
	int offset = 0;
	for (ptr = d->begin; ptr != d->end; ++ptr) {
		if (*ptr != '\t' && *ptr != '\r') {
			outc(*ptr);
			++offset;
		} else {
			int count = TAB_STOP - offset % TAB_STOP;
			while (count-- != 0 && offset < d->width) {
				outc(' ');
				++offset;
			}
		}
	}
	if (*(d->end - 1) != '\n')
		outc('\n');
	return;
}

static int more_open_buffer(const char *buf, size_t size, more_file_t *more)
{
	more->size = size;
	more->ln_size = size / LINENUM_BLOCK_SIZE + 1; // at least one block
	more->buf = malloc(size + more->ln_size * sizeof(linenum_t));
	if (!more->buf)
		return -1;

	memcpy(more->buf, buf, size);
	more->ln = (linenum_t *)(more->buf + size);
	memset(more->ln, 0, more->ln_size * sizeof(linenum_t));
	return 0;
}

static int more_open_file(const char *file, size_t size, more_file_t *more)
{
	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) < 0)
		return -1;

	int ret = more_open_buffer(m.ptr, m.size, more);

	mmap_close(&m);
	return ret;
}

static int more_prompt_file(more_file_t *more)
{
	//% prints("\033[0;1;44;32m下面还有喔(%d%%) 第(%d-%d)行 \033[33m|"
	prints("\033[0;1;44;32m\xcf\xc2\xc3\xe6\xbb\xb9\xd3\xd0\xe0\xb8(%d%%) \xb5\xda(%d-%d)\xd0\xd0 \033[33m|"
			//% " l 上篇 | b e 开头末尾 | g 跳转 | h 帮助\033[K\033[m",
			" l \xc9\xcf\xc6\xaa | b e \xbf\xaa\xcd\xb7\xc4\xa9\xce\xb2 | g \xcc\xf8\xd7\xaa | h \xb0\xef\xd6\xfa\033[K\033[m",
			(more->end - more->buf) * 100 / more->size,
			more->line - screen_lines() + 2, more->line);
	return 0;
}

static int is_emphasize(const char *str)
{
	//% "【 在")
	return (strneq2(str, "\xa1\xbe \xd4\xda") || strneq2(str, "==>")
			//% "□ 引用"
			|| strneq2(str, "\xa1\xf5 \xd2\xfd\xd3\xc3")
			//% "※ 引述"
			|| strneq2(str, "\xa1\xf9 \xd2\xfd\xca\xf6"));
}

static int is_quotation(const char *str)
{
	return (!strncmp(str, ": ", 2) || !strncmp(str, "> ", 2));
}

static int more_main(more_file_t *more, bool promptend, int line, int lines,
		int stuff, more_prompt_t prompt, more_handler_t handler)
{
	int lines_read = 1, pos = 0, i = 0, ch = 0;
	bool is_quote, is_wrapped, colored = false;
	int new_row;
	char *buf_end = more->buf + more->size;
	char linebuf[7];

	screen_clrtobot();
	// TODO: stuffmode
	while (true) {
		is_quote = more->prop & IS_QUOTE;
		while (i++ < screen_lines() - 1) {
			// Get a line
			if (more_getline(more) <= 0) {
				if (promptend)
					pressanykey();
				return ch;
			}
			lines_read++;
			if (lines == 0 || lines_read <= lines) {
				if (!(more->opt & NO_EMPHASIZE) && is_emphasize(more->begin)) {
					outs("\033[1;33m");
					more_puts(more);
					colored = true;
				} else {
					is_wrapped = (more->begin != more->buf)
							&& (*(more->begin - 1) != '\n');
					if (!(more->opt & NO_QUOTE) && (is_quote
							|| (!is_wrapped && is_quotation(more->begin)))) {
						is_quote = true;
						outs("\033[0;36m");
						colored = true;
					} else {
						if (colored) {
							outs("\033[m");
							colored = false;
						}
					}
					more_puts(more);
					is_wrapped = (*(more->end - 1) != '\n');
					if (is_quote && !is_wrapped)
						is_quote = false;
				}
				// Scrolling
				if (++pos == screen_lines()) {
					screen_scroll();
					--pos;
				}
			} else {
				if (promptend)
					pressanykey();
				screen_flush();
				return ch;
			}
		}

		// Reaching end by KEY_END can be rolled back.
		if (more->end == buf_end && ch != KEY_END) {
			if (promptend)
				pressanykey();
			return ch;
		}

		// If screen is filled, wait for user command.
		screen_move_clear(-1);
		prompt(more);

		ch = morekey();
		screen_move_clear(-1);
		screen_flush();
		switch (ch) {
			case KEY_LEFT:
				return ch;
				break;
			case KEY_RIGHT:
				i = 1;
				break;
			case KEY_DOWN:
				i = screen_lines() - 2;
				break;
			case KEY_PGUP:
				screen_clear();
				i = pos = 0;
				new_row = more->line - (2 * screen_lines() - 3);
				if (new_row < 0)
					return ch;
				more_seek(more, new_row);
				break;
			case KEY_UP:
				screen_clear();
				i = pos = 0;
				new_row = more->line - screen_lines();
				if (new_row < 0) {
					return ch;
				}
				more_seek(more, new_row);
				break;
			case KEY_HOME:
				screen_clear();
				i = pos = 0;
				more_seek(more, 0);
				break;
			case 'R':
			case KEY_END:
				more_countline(more);
				i = screen_lines() - 1 - (more->total - more->line);
				if (i < 0)
					i = 0;
				if (i == screen_lines() - 1)
					break;
				more_seek(more, more->total - (screen_lines() - 1) + i);
				break;
			case 'G':
				//% getdata(-1, 0, "跳转到的行号:", linebuf,
				getdata(-1, 0, "\xcc\xf8\xd7\xaa\xb5\xbd\xb5\xc4\xd0\xd0\xba\xc5:", linebuf,
						sizeof(linebuf), true, true);
				new_row = strtol(linebuf, NULL, 10) - 1;
				if (new_row < 0)
					new_row = 0;
				more_seek(more, new_row);
				if (more->total >= 0 && new_row >= more->total)
					more_seek(more, more->total - 1);
				screen_clear();
				i = pos = 0;
				break;
			case 'H':
				show_help("help/morehelp");
				i = pos = 0;
				new_row = more->line - screen_lines() + 1;
				if (new_row < 0)
					new_row = 0;
				more_seek(more, new_row);
				break;
			case 'L':
				return KEY_PGUP;
				break;
			default:
				if (handler) {
					ch = (*handler)(more, ch);
					if (ch)
						return ch;
				}
				break;
			}
	}
	if (promptend)
		pressanykey();
	return ch;
}

/**
 * Article reading function for telnet.
 * @param file File to show.
 * @param promptend Whether immediately ask user to press any key.
 * @param line The start line (on screen) of the article area.
 * @param numlines Lines shown at most, 0 means no limit.
 * @param stuffmode todo..
 * @return Last dealt key, -1 on error.
 */
static int rawmore2(const char *file, int promptend, int line, int numlines, int stuffmode)
{
	more_file_t *more = more_open(file, 0, DEFAULT_TERM_WIDTH, more_open_file);
	if (more == NULL)
		return -1;
	int ch = more_main(more, promptend, line, numlines, stuffmode,
			more_prompt_file, NULL);
	more_close(more);
	return ch;
}

static int more_open_msg(const char *file, size_t size, more_file_t *more)
{
	more->opt |= NO_QUOTE | NO_EMPHASIZE;

	FILE *fp = fopen(file, "r");
	if (fp == NULL)
		return -1;
	file_lock_all(fileno(fp), FILE_WRLCK);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	int len = 0;
	char head[LINE_BUFSIZE], buf[LINE_BUFSIZE];
	more->buf = malloc(size);
	if (more->buf != NULL) {
		fseek(fp, 0, SEEK_SET);
		while (fgets(head, sizeof(head), fp) && fgets(buf, sizeof(buf), fp)) {
			// truncate head.
			memcpy(more->buf + len, head, 71);
			len += 71;
			memcpy(more->buf + len, "\033[K\033[m\n", 7);
			len += 7;
			len += strlcpy(more->buf + len, buf, size - len);
			if (len > size) {
				len = size;
				break;
			}
		}
		file_lock_all(fileno(fp), FILE_UNLCK);
		fclose(fp);

		more->size = len;
		more->ln_size = len / LINENUM_BLOCK_SIZE + 1;
		more->buf = realloc(more->buf, len + more->ln_size * sizeof(linenum_t));
		if (more->buf == NULL)
			return -1;
		more->ln = (linenum_t *)(more->buf + len);
		memset(more->ln, 0, more->ln_size * sizeof(linenum_t));
	} else {
		return -1;
	}
	return 0;
}

static int more_prompt_msg(more_file_t *more)
{
	//% prints("\033[0;1;44;32m讯息浏览器 (%d%%) 第(%d-%d)行 \033[33m| "
	prints("\033[0;1;44;32m\xd1\xb6\xcf\xa2\xe4\xaf\xc0\xc0\xc6\xf7 (%d%%) \xb5\xda(%d-%d)\xd0\xd0 \033[33m| "
			//% "c 清除 | m 寄回信箱 | b e 开头末尾 | g 跳转\033[K\033[m",
			"c \xc7\xe5\xb3\xfd | m \xbc\xc4\xbb\xd8\xd0\xc5\xcf\xe4 | b e \xbf\xaa\xcd\xb7\xc4\xa9\xce\xb2 | g \xcc\xf8\xd7\xaa\033[K\033[m",
			(more->end - more->buf) * 100 / more->size,
			more->line - screen_lines() + 2, more->line);
	return 0;
}

static int more_handle_msg(more_file_t *more, int ch)
{
	switch (ch) {
		case 'C':
			//% if (askyn("确定要清除吗？", NA, YEA))
			if (askyn("\xc8\xb7\xb6\xa8\xd2\xaa\xc7\xe5\xb3\xfd\xc2\xf0\xa3\xbf", NA, YEA))
				return ch;
			break;
		case 'M':
			//% if (askyn("确定要寄回吗？", NA, YEA))
			if (askyn("\xc8\xb7\xb6\xa8\xd2\xaa\xbc\xc4\xbb\xd8\xc2\xf0\xa3\xbf", NA, YEA))
				return ch;
			break;
	}
	return 0;
}

int msg_more(void)
{
	char file[HOMELEN], title[STRLEN];
	if (!strcmp(currentuser.userid, "guest"))
		return 0;
#ifdef LOG_MY_MESG
	setuserfile(file, "msgfile.me");
#else
	setuserfile(file, "msgfile");
#endif
	set_user_status(ST_LOOKMSGS);

	int ch = 0;
	more_file_t *more = more_open(file, 0, DEFAULT_TERM_WIDTH, more_open_msg);
	if (!more) {
		//% 没有任何的讯息存在...
		presskeyfor("\xc3\xbb\xd3\xd0\xc8\xce\xba\xce\xb5\xc4\xd1\xb6\xcf\xa2\xb4\xe6\xd4\xda...", -1);
	} else {
		screen_clear();
		ch = more_main(more, false, 0, 0, false, more_prompt_msg,
				more_handle_msg);
		if (!ch) {
			screen_move_clear(-1);
			//% prints("\033[0;1;44;31m[讯息浏览器]  \033[33mc 清除 | "
			prints("\033[0;1;44;31m[\xd1\xb6\xcf\xa2\xe4\xaf\xc0\xc0\xc6\xf7]  \033[33mc \xc7\xe5\xb3\xfd | "
					//% "m 寄回信箱\033[K\033[m");
					"m \xbc\xc4\xbb\xd8\xd0\xc5\xcf\xe4\033[K\033[m");
			ch = toupper(terminal_getchar());
		}
		switch (ch) {
			case 'C':
				unlink(file);
				break;
			case 'M':
				//% snprintf(title, sizeof(title), "[%s] 所有讯息备份",
				snprintf(title, sizeof(title), "[%s] \xcb\xf9\xd3\xd0\xd1\xb6\xcf\xa2\xb1\xb8\xb7\xdd",
						format_time(fb_time(), TIME_FORMAT_ZH));
				mail_file(file, currentuser.userid, title);
				unlink(file);
				break;
			default:
				break;
		}
	}
	more_close(more);
	screen_clear();
	return ch;
}

int ansimore4(char *filename, int promptend, char *board, char *path, int ent)
{
	screen_clear();
	return rawmore2(filename, promptend, 0, 0, NA);
}

int ansimore_buffer(const char *buf, size_t size, int promptend)
{
	screen_clear();

	more_file_t *more = more_open(buf, size, DEFAULT_TERM_WIDTH,
			more_open_buffer);
	if (!more)
		return -1;
	int ch = more_main(more, promptend, 0, 0, NA, more_prompt_file, NULL);
	more_close(more);

	screen_move(-1, 0);
	prints("\033[m");
	screen_flush();
	return ch;
}

int ansimore(const char *filename, int promptend)
{
	int ch;
	screen_clear();
	ch = rawmore2(filename, promptend, 0, 0, NA);
	screen_move(-1, 0);
	prints("\033[0m\033[m");
	screen_flush();
	return ch;
}

int ansimore2(char *filename, int promptend, int row, int numlines)
{
	int     ch;
	ch = rawmore2(filename, promptend, row, numlines, NA);
	screen_flush();
	return ch;
}

void show_help(const char *fname)
{
	ansimore(fname, YEA);
	screen_clear();
}

int mainreadhelp(void)
{
	show_help("help/mainreadhelp");
	return FULLUPDATE;
}

int mailreadhelp(void)
{
	show_help("help/mailreadhelp");
	return FULLUPDATE;
}
