#include <arpa/telnet.h>
#include <stdarg.h>
#include <wchar.h>
#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

/** 输出一条ANSI指令 */
#define ansi_cmd(cmd)  \
	terminal_write_cached((unsigned char *) cmd, sizeof(cmd) - 1)

enum {
	SCREEN_LINE_LEN = 1019,
	MIN_SCREEN_LINES = 24,
	MAX_SCREEN_LINES = 100,
};

/** 屏幕上的一行 */
typedef struct {
	uint16_t old_width; ///< 上次输出时的字符串宽度
	uint16_t len; ///< 当前字符串长度
	bool modified; ///< 自从上次输出以来是否修改过
	uchar_t data[SCREEN_LINE_LEN]; ///< 缓冲区
} screen_line_t;

typedef struct {
	screen_line_t *buf; ///< 行缓冲区数组
	int columns; ///< 列数
	int lines; ///< 行数
	int cur_ln; ///< 当前操作所在行
	int cur_col; ///< 当前操作所在列
	int tc_line; ///< 终端当前所在行
	int tc_col; ///< 终端当前所在列
	int roll; ///< 首行在buf中的偏移量
	int scrollcnt;
	bool redraw; ///< 重绘屏幕
} screen_t;

static screen_t screen = {
	.lines = 24,
	.columns = 255,
};

bool screen_inited(void)
{
	return screen.buf;
}

int screen_lines(void)
{
	return screen.lines;
}

// Get height of client window.
// See RFC 1073 "Telnet Window Size Option"
void screen_negotiate_size(void)
{
	// An example: Server suggest and client agrees to use NAWS.
	//             (Negotiate About Window Size)
	//    (server sends)  IAC DO  NAWS
	//    (client sends)  IAC WILL NAWS
	//	  (client sends)  IAC SB  NAWS 0 80 0 24 IAC SE
	uchar_t naws[] = { IAC, DO, TELOPT_NAWS };
	terminal_write(naws, sizeof(naws));
}

int num_ans_chr(const char *str)
{
	int len, i, ansinum, ansi;

	ansinum=0;
	ansi=NA;
	len=strlen(str);
	for (i=0; i < len; i++) {
		if (str[i] == KEY_ESC) {
			ansi = YEA;
			ansinum++;
			continue;
		}
		if (ansi) {
			if (!strchr("[0123456789; ", str[i]))
				ansi = NA;
			ansinum++;
			continue;
		}
	}
	return ansinum;
}

void screen_init(int lines)
{
	if (!screen.buf || screen.columns > SCREEN_LINE_LEN)
		screen.columns = SCREEN_LINE_LEN;

	if (lines < MIN_SCREEN_LINES)
		lines = MIN_SCREEN_LINES;
	if (lines > MAX_SCREEN_LINES)
		lines = MAX_SCREEN_LINES;

	if (!screen.buf) {
		screen.buf = malloc(screen.lines * sizeof(*screen.buf));
		for (int i = screen.lines - 1; i >= 0; --i) {
			screen_line_t *sl = screen.buf + i;
			sl->modified = false;
			sl->len = 0;
			sl->old_width = 0;
		}
		screen.redraw = true;
		screen.roll = 0;
	} else if (lines > screen.lines) {
		screen.buf = realloc(screen.buf, lines * sizeof(*screen.buf));
		screen.lines = lines;
	}
}

/**
 * 生成并发送ANSI光标移动指令
 * @param col 要移动到的列
 * @param line 要移动到的行
 */
static void move_terminal_cursor_to(int col, int line)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "\033[%d;%dH", line + 1, col + 1);
	for (char *p = buf; *p; ++p)
		terminal_putchar(*p);
}

/**
 * 移动终端光标到指定位置
 * @param col 要移动到的列
 * @param line 要移动到的行
 */
static void move_terminal_cursor(int col, int line)
{
	if (line >= screen.lines || col >= screen.columns)
		return;

	if (!col && line == screen.tc_line + 1) {
		terminal_putchar('\n');
		if (screen.tc_col)
			terminal_putchar('\r');
	} else if (!col && line == screen.tc_line) {
		if (screen.tc_col)
			terminal_putchar('\r');
	} else if (col == screen.tc_col - 1 && line == screen.tc_line) {
		terminal_putchar(Ctrl('H'));
	} else if (col != screen.tc_col || line != screen.tc_line) {
		move_terminal_cursor_to(col, line);
	}

	screen.tc_col = col;
	screen.tc_line = line;
}

static inline screen_line_t *get_screen_line(int line)
{
	if (line < 0)
		line += screen.lines;
	return screen.buf + (line + screen.roll) % screen.lines;
}

static int _write_helper(const char *buf, size_t len, void *arg)
{
	if (!screen.buf) {
		const char *end = buf + len;
		for (const char *ptr = buf; ptr < end; ++ptr) {
			if (*ptr == '\n')
				terminal_putchar('\r');
			terminal_putchar(*ptr);
		}
	} else {
		terminal_write_cached((const uchar_t *) buf, len);
	}
	return 0;
}

static void screen_write_cached(const uchar_t *data, uint16_t len)
{
	if (terminal_is_utf8()) {
		_write_helper((const char *) data, len, NULL);
	} else {
		convert(CONVERT_U2G, (const char *) data, len, NULL, 0,
				_write_helper, NULL);
	}
}

/**
 * 重绘屏幕
 */
void screen_redraw(void)
{
	if (!screen.buf)
		return;

	ansi_cmd(ANSI_CMD_CL);
	screen.tc_col = 0;
	screen.tc_line = 0;

	for (int i = 0; i < screen.lines; ++i) {
		screen_line_t *sl = get_screen_line(i);
		if (!sl->len)
			continue;

		move_terminal_cursor(0, i);
		screen_write_cached(sl->data, sl->len);

		screen.tc_col += sl->len;
		if (screen.tc_col >= screen.columns) {
			screen.tc_col = screen.columns - 1;
		}
		sl->modified = false;
		sl->data[sl->len] = '\0';
		sl->old_width = screen_display_width((char *) sl->data, true);
	}
	move_terminal_cursor(screen.cur_col, screen.cur_ln);
	screen.redraw = false;
	screen.scrollcnt = 0;
	terminal_flush();
}

void screen_flush(void)
{
	if (!screen.buf) {
		terminal_flush();
		return;
	}

	if (!terminal_input_buffer_empty())
		return;
	if (screen.redraw || (abs(screen.scrollcnt) >= (screen.lines - 3))) {
		screen_redraw();
		return;
	}
	if (screen.scrollcnt < 0) {
		move_terminal_cursor(0, 0);
		while (screen.scrollcnt < 0) {
			ansi_cmd(ANSI_CMD_SR);
			screen.scrollcnt++;
		}
	}
	if (screen.scrollcnt > 0) {
		move_terminal_cursor(0, screen.lines - 1);
		while (screen.scrollcnt > 0) {
			terminal_putchar('\n');
			screen.scrollcnt--;
		}
	}
	for (int i = 0; i < screen.lines; i++) {
		screen_line_t *sl = get_screen_line(i);
		if (!sl->modified)
			continue;
		sl->modified = false;

		sl->data[sl->len] = '\0';
		int width = screen_display_width((char *) sl->data, true);

		move_terminal_cursor(0, i);
		screen_write_cached(sl->data, sl->len);

		screen.tc_col = width;
		if (screen.tc_col >= screen.columns) {
			screen.tc_col -= screen.columns;
			screen.tc_line++;
			if (screen.tc_line >= screen.lines)
				screen.tc_line = screen.lines - 1;
		}

		if (sl->old_width > width)
			ansi_cmd(ANSI_CMD_CE);
		sl->old_width = width;
	}

	screen_line_t *sl = get_screen_line(screen.cur_ln);
	move_terminal_cursor(screen.cur_col == sl->len && screen.cur_col ?
			sl->old_width : screen.cur_col, screen.cur_ln);
	terminal_flush();
}

/**
 * Move to given position.
 * @param line Line number.
 * @param col Column number.
 */
void screen_move(int line, int col)
{
	screen.cur_col = col;
	screen.cur_ln = line < 0 ? line + screen.lines : line;
}

/**
 * 获得当前屏幕坐标
 * @param[out] line 行坐标
 * @param[out] col 列坐标
 */
void screen_coordinates(int *line, int *col)
{
	*line = screen.cur_ln;
	*col = screen.cur_col;
}

/**
 * Reset screen and move to (0, 0).
 */
void screen_clear(void)
{
	if (screen.buf) {
		screen.roll = 0;
		screen.redraw = true;
		for (int i = 0; i < screen.lines; ++i) {
			screen_clear_line(i);
		}
		screen_move(0, 0);
	}
}

void screen_clear_line(int line)
{
	screen_line_t *sl = screen.buf + line;
	sl->modified = true;
	sl->len = 0;
}

void screen_move_clear(int line)
{
	screen_move(line, 0);
	clrtoeol();
}

/**
 * Clear to end of current line.
 */
void clrtoeol(void)
{
	if (!screen.buf) {
		if (screen.cur_col == 0)
			terminal_putchar('\r');
		ansi_cmd(ANSI_CMD_CE);
	} else {
		screen_line_t *sl = get_screen_line(screen.cur_ln);
		if (screen.cur_col > sl->len)
			memset(sl->data + sl->len, ' ', screen.cur_col - sl->len + 1);
		sl->len = screen.cur_col;
		sl->modified = true;
	}
}

/** 从当前行清除到最后一行 */
void screen_clrtobot(void)
{
	if (screen.buf) {
		for (int i = screen.cur_ln; i < screen.lines; ++i) {
			screen_line_t *sl = get_screen_line(i);
			sl->modified = true;
			sl->len = 0;
			sl->old_width = SCREEN_LINE_LEN;
		}
	}
}

static const char *nullstr = "(null)";

void screen_puts(const char *s, size_t size)
{
	if (!size)
		size = strlen(s);

	if (!screen.buf) {
		screen_write_cached((const uchar_t *) s, size);
		return;
	}

	screen_line_t *sl = get_screen_line(screen.cur_ln);

	const char *end = s + size;
	for (const char *p = s; p < end; ++p) {
		if (*p == '\n' || *p == '\r') {
			if (screen.cur_col > sl->len)
				memset(sl->data + sl->len, ' ', screen.cur_col- sl->len + 1);
			sl->len = screen.cur_col;
			screen.cur_col = 0;
			if (screen.cur_ln < screen.lines)
				sl = get_screen_line(++screen.cur_ln);
		} else {
			if (screen.cur_col > sl->len)
				memset(sl->data + sl->len, ' ', screen.cur_col - sl->len);
			sl->modified = true;
			sl->data[screen.cur_col++] = *p;
			if (screen.cur_col > sl->len)
				sl->len = screen.cur_col;
		}
	}
}

void screen_putc(int c)
{
	char ch = c;
	screen_puts(&ch, 1);
}

static int screen_put_gbk_helper(const char *buf, size_t len, void *arg)
{
	screen_puts(buf, len);
	return 0;
}

static void screen_put_gbk(int c)
{
	static int left = 0;
	if (left) {
		char buf[3] = { left, c, '\0' };
		convert(CONVERT_G2U, buf, 2, NULL, 0, screen_put_gbk_helper, NULL);
		left = 0;
	} else if (c & 0x80) {
		left = c;
	} else {
		screen_putc(c);
	}
}

/**
 * Output a character.
 * @param c The character.
 * @return 1, 0 on ansi control codes.
 * @deprecated
 */
int outc(int c)
{
	if (!screen.buf) {
		if (!isprint2(c)) {
			if (c == '\n') {
				terminal_putchar('\r');
			} else {
				if (c != KEY_ESC || !showansi)
					c = '*';
			}
		}
		terminal_putchar(c);
		return 1;
	}

	screen_line_t *slp = get_screen_line(screen.cur_ln);

	if (!isprint2(c)) {
		if (c == '\n' || c == '\r') {
			if (screen.cur_col> slp->len) {
				memset(slp->data + slp->len, ' ',
						screen.cur_col - slp->len + 1);
			}
			slp->len = screen.cur_col;
			screen.cur_col = 0;
			if (screen.cur_ln < screen.lines)
				screen.cur_ln++;
			return 1;
		} else {
			if (c != KEY_ESC || !showansi)
				c = '*';
		}
	}

	screen_put_gbk(c);

	if (screen.cur_col >= screen.columns) {
		screen.cur_col = 0;
		if (screen.cur_ln < screen.lines)
			screen.cur_ln++;
	}
	return 1;
}

/**
 * Output a string.
 * @param str The string.
 * @deprecated
 */
void outs(const char *str)
{
	while (*str != '\0') {
		outc(*str++);
	}
}

size_t screen_display_width(const char *ptr, bool utf8)
{
	size_t width = 0;
	bool ansi = false;
	while (*ptr) {
		if (ansi) {
			if (isalpha(*ptr))
				ansi = false;
		} else {
			if (*ptr == '\033') {
				ansi = true;
			} else {
				if (utf8) {
					wchar_t wc = next_wchar(&ptr, NULL);
					if (wc == WEOF)
						break;
					width += fb_wcwidth(wc);
					--ptr;
				} else {
					++width;
				}
			}
		}
		++ptr;
	}
	return width;
}

static void _print_string(const char *ptr, size_t width, bool utf8,
		bool left_adjust)
{
	if (!ptr)
		ptr = nullstr;

	size_t padding = 0;
	if (width) {
		size_t w = screen_display_width(ptr, utf8);
		if (w < width)
			padding = width - w;
	}

	if (padding && !left_adjust)
		tui_repeat_char(' ', padding);

	if (utf8) {
		screen_puts(ptr, 0);
	} else {
		convert(CONVERT_G2U, ptr, CONVERT_ALL, NULL, 0,
				screen_put_gbk_helper, NULL);
	}

	if (padding && left_adjust)
		tui_repeat_char(' ', padding);
}

static void _put_string(const char *ptr, const char *end, bool utf8)
{
	if (ptr && end && end > ptr) {
		size_t size = end - ptr;
		if (utf8) {
			screen_puts(ptr, size);
		} else {
			convert(CONVERT_G2U, ptr, size, NULL, 0, screen_put_gbk_helper,
					NULL);
		}
	}
}

/**
 * 格式化输出到屏幕
 * 支持形如%d %ld %c %s %-7s %7s的格式
 * @param[in] fmt 格式字符串
 * @param[in] utf8 输入是否为UTF-8编码
 * @param[in] ap 参数列表
 */
static void screen_vprintf(const char *fmt, bool utf8, va_list ap)
{
	const char *ptr = fmt;
	while (*fmt != '\0') {
		if (*fmt == '%') {
			_put_string(ptr, fmt, utf8);

			bool left_adjust = false, long_ = false;
			size_t width = 0;

			++fmt;
			switch (*fmt) {
				case '-':
					left_adjust = true;
					++fmt;
					break;
				case 'l':
					long_ = true;
					++fmt;
					break;
			}

			while (isdigit(*fmt)) {
				width *= 10;
				width += *fmt - '0';
				++fmt;
			}

			switch (*fmt) {
				case 's': {
					const char *ptr = va_arg(ap, const char *);
					_print_string(ptr, width, utf8, left_adjust);
					break;
				}
				case 'd': {
					int64_t n;
					if (long_) {
						n = va_arg(ap, int64_t);
					} else {
						n = va_arg(ap, int);
					}

					char buf[24];
					snprintf(buf, sizeof(buf), "%ld", n);
					_print_string(buf, width, true, left_adjust);
					break;
				}
				case 'c': {
					int i = va_arg(ap, int);
					screen_putc(i);
					break;
				}
				case '\0':
					return;
				default:
					if (utf8)
						screen_putc(*fmt);
					else
						screen_put_gbk(*fmt);
					break;
			}
			ptr = ++fmt;
		} else {
			++fmt;
		}
	}
	_put_string(ptr, fmt, utf8);
}

/**
 * 格式化输出到屏幕
 * 输入必须为UTF-8编码
 * @param[in] fmt 格式字符串
 * @see screen_vprintf
 */
void screen_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	screen_vprintf(fmt, true, ap);
	va_end(ap);
}

/**
 * 格式化输出到屏幕
 * 输入必须为GBK编码
 * @param[in] fmt 格式字符串
 * @see screen_vprintf
 * @deprecated
 */
void prints(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	screen_vprintf(fmt, false, ap);
	va_end(ap);
}

/**
 * Scroll down one line.
 */
void screen_scroll(void)
{
	++screen.scrollcnt;
	if (++screen.roll >= screen.lines)
		screen.roll -= screen.lines;
	screen_move_clear(-1);
}

/**
 * 寻找字符串中指定宽度的位置
 * @param[in] ptr 字符串
 * @param[in] size 字符串长度
 * @param[in,out] width 宽度。如果字符串宽度不够或位于字符中间，则返回剩余宽度
 * @return 指定宽度所在位置
 */
static const char *seek_to_width(const char *ptr, size_t size, size_t *width)
{
	bool ansi = false;
	const char *end = ptr + size;
	while (ptr < end && *width) {
		if (ansi) {
			if (isalpha(*ptr))
				ansi = false;
			++ptr;
		} else {
			if (*ptr == '\033') {
				ansi = true;
				++ptr;
			} else {
				const char *p = ptr;
				size_t left = end - ptr;
				wchar_t wc = next_wchar(&p, &left);
				if (wc == WEOF)
					break;

				size_t w = fb_wcwidth(wc);
				if (*width >= w) {
					*width -= w;
					ptr = p;
				} else {
					break;
				}
			}
		}
	}
	return ptr;
}

/**
 * 在屏幕上指定位置替换字符串
 * @param line 行
 * @param col 列
 * @param str 要替换的字符串
 */
void screen_replace(int line, int col, const char *str)
{
	screen_line_t *sl = get_screen_line(line);
	char buf[sizeof(sl->data)];
	memcpy(buf, sl->data, sl->len);
	const char *end = buf + sl->len;

	size_t w = col;
	const char *ptr = seek_to_width(buf, end - buf, &w);

	screen_move_clear(line);
	if (ptr > buf)
		screen_puts(buf, ptr - buf);
	if (w) {
		tui_repeat_char(' ', w);
	}
	screen_puts(str, 0);

	size_t width = screen_display_width(str, true);
	if (ptr < end) {
		const char *ptr2 = seek_to_width(ptr, end - ptr, &width);
		if (ptr2 < end)
			screen_puts(ptr2, end - ptr2);
	}
}

void screen_save_line(int line, bool save)
{
	static char saved[SCREEN_LINE_LEN];

	screen_line_t *bp = screen.buf;
	line = line < 0 ? line + screen.lines : line;

	if (save) {
		strlcpy(saved, (const char *) bp[line].data, sizeof(saved));
	} else {
		int x, y;
		screen_coordinates(&x, &y);
		screen_move_clear(line);
		screen_printf("%s", saved);
		screen_move(x, y);
		screen_flush();
	}
}

/* Added by Ashinmarch on 2007.12.01,
 * to support multi-line msgs
 * It is used to save screen lines overwrited by msgs
 */
void saveline_buf(int line, int mode)//0:save 1:restore
{
	static char temp[MAX_MSG_LINE * 2 + 2][SCREEN_LINE_LEN];
	screen_line_t *bp = screen.buf;
	int x, y;

	switch (mode) {
		case 0:
			strlcpy(temp[line], (const char *) bp[line].data,
					sizeof(temp[line]));
			temp[line][bp[line].len] = '\0';
			break;
		case 1:
			screen_coordinates(&x, &y);
			screen_move(line, 0);
			screen_clear_line(line);
			screen_printf("%s", temp[line]);
			screen_move(x,y);
			break;
	}
}
