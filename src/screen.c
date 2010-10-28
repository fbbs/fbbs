#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

#include "fbbs/string.h"
#include "fbbs/screen.h"

/**
 * String of commands to clear the entire screen and position the cursor at the
 * upper left corner. 
 */
#define TERM_CMD_CL "\033[H\033[J"

/**
 * String of commands to clear from the cursor to the end of the current line.
 */
#define TERM_CMD_CE "\033[K"

/**
 * String of commands to scroll the screen one line down, assuming it is output
 * with the cursor at the beginning of the top line. 
 */
#define TERM_CMD_SR "\033M"

/** String of commands to enter standout mode. */
#define TERM_CMD_SO "\033[7m"

/** String of commands to leave standout mode. */
#define TERM_CMD_SE "\033[m"

/** Send a terminal command. */
#define term_cmd(cmd)  telnet_write(s->tc, (const uchar_t *)cmd, sizeof(cmd) - 1)

/** The standard screen. */
static screen_t stdscr;

/**
 * Initialize a screen.
 * @param scr The screen to be initialized.
 * @param tc Related telnet connection.
 * @param lines Lines of the screen.
 * @param cols Columns of the screen.
 */
static void _screen_init(screen_t *scr, telconn_t *tc, int lines, int cols)
{
	scr->scr_lns = lines;
	scr->scr_cols = cols;
	scr->cur_ln = scr->cur_col = scr->roll = scr->scroll_cnt = 0;
	scr->size = lines;
	scr->show_ansi = true;
	scr->clear = true;
	scr->tc = tc;
	scr->lines = fb_malloc(sizeof(*scr->lines) * lines);
	screen_line_t *slp = scr->lines;
	for (int i = 0; i < lines; ++i) {
		slp->oldlen = slp->len = 0;
		slp->modified = false;
		++slp;
	}
}

/**
 * Initialize the standard screen.
 * @param tc Related telnet connection.
 * @param lines Lines of the screen.
 * @param cols Columns of the screen.
 */
void screen_init(telconn_t *tc, int lines, int cols)
{
	_screen_init(&stdscr, tc, lines, cols);
}

/**
 * Move to given position.
 * @param s The screen.
 * @param line The line to move to.
 * @param col The column to move to.
 */
static void _move(screen_t *s, int line, int col)
{
	s->cur_ln = line;
	s->cur_col = col;
}

/**
 * Move to given position in the standard screen.
 * @param line The line to move to.
 * @param col The column to move to.
 */
void move(int line, int col)
{
	_move(&stdscr, line, col);
}

/**
 * Clear a screen.
 * @param s The screen.
 */
static void _clear(screen_t *s)
{
	s->roll = 0;
	s->clear = true;

	screen_line_t *slp = s->lines;
	for (int i = 0; i < s->scr_lns; ++i) {
		slp->modified = false;
		slp->len = 0;
		slp->oldlen = 0;
		++slp;
	}
	_move(s, 0, 0);
}

/**
 * Clear the standard screen.
 */
void clear(void)
{
	_clear(&stdscr);
}

/**
 * Clear from current column to the end of line.
 * @param s The screen.
 */
static void _clrtoeol(screen_t *s)
{
	screen_line_t *slp = s->lines + (s->cur_ln + s->roll) % s->scr_lns;
	if (s->cur_col > slp->len)
		memset(slp->data + slp->len, ' ', s->cur_col - slp->len + 1);
	slp->len = s->cur_col;
}

/**
 * Clear from current column to the end of line.
 */
void clrtoeol(void)
{
	_clrtoeol(&stdscr);
}

/**
 * Clear from current line (included) to the bottom of the screen.
 * @param s The screen.
 */
static void _clrtobot(screen_t *s)
{
	screen_line_t *slp;
	for (int i = s->cur_ln; i < s->scr_lns; ++i) {
		slp = s->lines + (s->cur_ln + s->roll) % s->scr_lns;
		slp->modified = true;
		slp->len = 0;
	}
}

/**
 * Clear from current line (included) to the bottom of the standard screen.
 */
void clrtobot(void)
{
	_clrtobot(&stdscr);
}

/**
 * Scroll one line down.
 * @param s The screen.
 */
static void _scroll(screen_t *s)
{
	s->scroll_cnt++;
	s->roll++;
	if (s->roll >= s->scr_lns)
		s->roll -= s->scr_lns;
	_move(s, s->scr_lns - 1, 0);
	_clrtoeol(s);
}

/**
 * Scroll one line down.
 */
void scroll(void)
{
	_scroll(&stdscr);
}

static inline bool isprint2(int c)
{
	return ((c & 0x80) || isprint(c));
}

static void _outs(screen_t *s, const uchar_t *str, int len)
{
	bool newline = true;
	screen_line_t *slp = s->lines + (s->cur_ln + s->roll) % s->scr_lns;
	int col = s->cur_col, c;

	if (len == 0)
		len = strlen((const char *)str);
	while (len-- > 0) {
		if (newline) {
			slp->modified = true;
			if (col > slp->len) {
				memset(slp->data + slp->len, ' ', col - slp->len);
				slp->len = col;
			}
			newline = false;
		}

		c = *str++;

		if (!isprint2(c)) {
			if (c == '\n' || c == '\r') {
				slp->data[col] = ' ';
				slp->len = col;
				s->cur_col = 0;
				if (s->cur_ln < s->scr_lns)
					++s->cur_ln;
				newline = true;
				slp = s->lines + (s->cur_ln + s->roll) % s->scr_lns;
				col = s->cur_col;
				continue;
			} else {
				if (c != KEY_ESC || !s->show_ansi)
					c = '*';
			}
		}

		slp->data[col++] = c;
		if (col >= sizeof(slp->data)) {
			col = sizeof(slp->data) - 1;
		}
	}

	if (col > slp->len)
		slp->len = col;
	s->cur_col = col;
}

void outc(int c)
{
	uchar_t ch = c;
	_outs(&stdscr, &ch, 1);
}

void outs(const char *str)
{
	_outs(&stdscr, (const uchar_t *)str, 0);
}

static inline void ochar(screen_t *s, int c)
{
	telnet_putc(s->tc, c);
}

/**
 * Move to a new position in terminal.
 * @param s The screen.
 * @param line The new line.
 * @param col The new column.
 */
static void term_move(screen_t *s, int line, int col)
{
	if (col == 0 && line == s->tc_ln + 1) { // newline
		ochar(s, '\n');
		if (s->tc_ln != 0)
			ochar(s, '\r');
	} else if (col == 0 && line == s->tc_ln) { // return
		if (s->tc_col != 0)
			ochar(s, '\r');
	} else if (col == s->tc_col - 1 && line == s->tc_ln) { // backspace
		ochar(s, KEY_CTRL_H);
	} else if (col != s->tc_col || line != s->tc_ln) { // arbitrary move
		char buf[16];
		snprintf(buf, sizeof(buf), "\033[%d;%dH", line + 1, col + 1);
		for (char *p = buf; *p != '\0'; p++)
			ochar(s, *p);
	}
	s->tc_ln = line;
	s->tc_col = col;
}

/**
 * Redraw the screen.
 * @param s The screen.
 */
static void _redoscr(screen_t *s)
{
	term_cmd(TERM_CMD_CL);
	s->tc_col = s->tc_ln = 0;

	struct screen_line_t *slp;
	for (int i = 0; i < s->scr_lns; i++) {
		slp = s->lines + (i + s->roll) % s->scr_lns;
		if (slp->len == 0)
			continue;

		term_move(s, i, 0);
		telnet_write(s->tc, slp->data, slp->len);
		s->tc_col += slp->len;

		slp->modified = false;
		slp->oldlen = slp->len;
	}
	term_move(s, s->cur_ln, s->cur_col);
	s->scroll_cnt = 0;
	s->clear = false;
	telnet_flush(s->tc);
}

/**
 * Redraw the standard screen.
 */
void redoscr(void)
{
	_redoscr(&stdscr);
}

/**
 * Flush all modifications of a screen to terminal.
 * @param s The screen.
 */
static void _refresh(screen_t *s)
{
	if (!buffer_empty(s->tc))
		return;

	if (s->clear || s->scroll_cnt >= s->scr_lns - 3) {
		_redoscr(s);
		return;
	}

	if (s->scroll_cnt > 0) {
		term_move(s, s->tc->lines - 1, 0);
		while (s->scroll_cnt-- > 0) {
			ochar(s, '\n');
		}
	}

	screen_line_t *slp;
	for (int i = 0; i < s->scr_lns; ++i) {
		slp = s->lines + (i + s->roll) % s->scr_lns;
		if (slp->modified) {
			slp->modified = false;
			term_move(s, i, 0);
			telnet_write(s->tc, slp->data, slp->len);
			if (slp->len != 0)
				term_cmd(TERM_CMD_CE);
			s->tc_col = slp->len + 1;
		}
		slp->oldlen = slp->len;
	}
	term_move(s, s->cur_ln, s->cur_col);
	telnet_flush(s->tc);
}

/**
 * Flush all modifications of the standard screen to terminal.
 */
void refresh(void)
{
	_refresh(&stdscr);
}

/**
 * Get the width of the standard screen.
 * @return The width of the standard screen.
 */
int get_screen_width(void)
{
	return stdscr.scr_cols;
}

/**
 * Get the height of the standard screen.
 * @return The height of the standard screen.
 */
int get_screen_height(void)
{
	return stdscr.scr_lns;
}

/**
 * Print width-limited string.
 * @param str The string.
 * @param max Maxmimum string width (if max > 0).
 * @param min Minimum string width (if min < 0). If the string has fewer
 *            columns, it is padded with spaces.
 * @param left_align Alignment when padding.
 */
static void outns(const char *str, int max, int min, bool left_align)
{
	int ret, w;
	size_t width = 0;
	wchar_t wc;
	mbstate_t state;
	memset(&state, 0, sizeof(state));
	const char *s = str;
	while (*s != '\0') {
		ret = mbrtowc(&wc, s, MB_CUR_MAX, &state);
		if (ret >= (size_t)-2) {
			break;
		} else {
			w = wcwidth(wc);
			if (w == -1)
				w = 0;
			width += w;
			if (max > 0 && width > max)
				break;
			s += ret;
		}
	}

	if (max > 0) {
		_outs(&stdscr, (const uchar_t *)str, s - str);
	} else if (min > 0) {
		if (!left_align) {
			for (int i = 0; i < min - width; ++i)
				outc(' ');
		}
		_outs(&stdscr, (const uchar_t *)str, s - str);
		if (left_align) {
			for (int i = 0; i < min - width; ++i)
				outc(' ');
		}
	}
}

static void outns2(const char *str, int val, int sgn, int sgn2)
{
	if (val) {
		if (!sgn2) {
			outns(str, val, 0, sgn < 0);
		} else {
			outns(str, 0, val, sgn < 0);
		}
	} else {
		_outs(&stdscr, (const uchar_t *)str, 0);
	}
}

/**
 * Format and print string to the standard screen.
 * Only 3 format specifiers (s, d, c) are supported.
 * Only one of maximum or minimum filed width can be specified.
 * @param fmt The format string.
 */
void prints(const char *fmt, ...)
{
	const char *bp;
	int i;
	char tmp[16];

	va_list ap;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt == '%') {
			int sgn = 1;
			int sgn2 = 1;
			int val = 0;
			fmt++;
			switch (*fmt) {
				case '-':
					while (*fmt == '-') {
						sgn = -sgn;
						fmt++;
					}
					break;
				case '.':
					sgn2 = 0;
					fmt++;
					break;
			}

			while (isdigit(*fmt)) {
				val *= 10;
				val += *fmt - '0';
				fmt++;
			}

			switch (*fmt) {
				case 's':
					bp = va_arg(ap, const char *);
					if (bp == NULL)
						continue;
					outns2(bp, val, sgn, sgn2);
					break;
				case 'd':
					i = va_arg(ap, int);
					snprintf(tmp, sizeof(tmp), "%d", i);
					outns2(tmp, val, sgn, sgn2);
					break;
				case 'c':
					i = va_arg(ap, int);
					outc(i);
					break;
				case '\0':
					va_end(ap);
					return;
				default:
					outc(*fmt);
					break;
			}
			fmt++;
			continue;
		}
		outc(*fmt);
		fmt++;
	}
	va_end(ap);
	return;
}

int getch(void)
{
	return term_getch(stdscr.tc);
}

/**
 * Delete a UTF-8 character.
 * @param backspace If true, delete backward, otherwise forward.
 * @return Bytes deleted.
 */
int delch(bool backspace)
{
	screen_t *s = &stdscr;
	screen_line_t *slp = s->lines + (s->cur_ln + s->roll) % s->scr_lns;
	uchar_t *begin = slp->data, *end = slp->data + slp->len;
	uchar_t *ptr = slp->data + s->cur_col;
	if (backspace) {
		while (--ptr >= begin) {
			if ((*ptr & 0x80) == 0 || (*ptr & 0xC0) == 0xC0)
				break;
		}
		end = slp->data + s->cur_col;
		if (ptr >= begin) {
			slp->modified = true;
			int width = mbwidth((const char *)ptr);
			memmove(ptr, end, slp->len - s->cur_col);
			slp->len -= end - ptr;
			s->cur_col -= width;
			if (s->cur_col < 0)
				s->cur_col = 0;
			return end - ptr;
		}
	} else {
		while (++ptr < end) {
			if ((*ptr & 0x80) == 0 || (*ptr & 0xC0) == 0xC0)
				break;
		}
		begin = slp->data + s->cur_col;
		if (ptr < end) {
			slp->modified = true;
			memmove(begin, ptr, end - ptr);
			slp->len -= ptr - begin;
			return ptr - begin;
		}
	}
	return 0;
}

/**
 * Get user input string.
 * @param line Line to show prompt.
 * @param prompt The prompt message.
 * @param buf The buffer. If not empty, its content is shown.
 * @param len Length of buffer.
 * @param mask Whether to asterisk the echo.
 * @return The user input in bytes.
 */
int getdata(int line, const char *prompt, char *buf, int len, bool mask)
{
	move(line, 0);
	clrtoeol();
	if (prompt)
		outs(prompt);

	int clen = 0;
	if (buf[0] != '\0') {
		outs(buf);
		clen = strlen(buf);
	}
	refresh();

	int ch;
	while (1) {
		ch = getch();
		if (ch == '\n')
			break;
		switch (ch) {
			case KEY_BACKSPACE:
			case KEY_CTRL_H:
				if (clen == 0)
					continue;
				clen -= delch(true);
				break;
			case KEY_DEL:
				if (clen == 0)
					continue;
				clen -= delch(false);
				break;
			case KEY_LEFT: // TODO: ...
				break;
			case KEY_RIGHT:
				break;
			case KEY_HOME:
			case KEY_CTRL_A:
				break;
			case KEY_END:
			case KEY_CTRL_E:
				break;
			default:
				if (!isprint2(ch) || clen >= len - 1)
					continue;
				buf[clen++] = ch;
				if (mask)
					outc('*');
				else
					outc(ch);
				break;
		}
		refresh();
	}
	buf[clen] = '\0';
	return clen;
}

