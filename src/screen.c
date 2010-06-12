#include <ctype.h>
#include <string.h>
#include <stdio.h>

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

static screen_t stdscr;

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
		slp->smod = slp->emod = 0;
		++slp;
	}
}

void screen_init(telconn_t *tc, int lines, int cols)
{
	_screen_init(&stdscr, tc, lines, cols);
}

static void _move(screen_t *s, int line, int col)
{
	s->cur_ln = line;
	s->cur_col = col;
}

void move(int line, int col)
{
	_move(&stdscr, line, col);
}

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

void clear(void)
{
	_clear(&stdscr);
}

static void _clrtoeol(screen_t *s)
{
	screen_line_t *slp = s->lines + (s->cur_ln + s->roll) % s->scr_lns;
	if (s->cur_col > slp->len)
		memset(slp->data + slp->len, ' ', s->cur_col - slp->len + 1);
	slp->len = s->cur_col;
}

void clrtoeol(void)
{
	_clrtoeol(&stdscr);
}

static void _scroll(screen_t *s)
{
	s->scroll_cnt++;
	s->roll++;
	if (s->roll >= s->scr_lns)
		s->roll -= s->scr_lns;
	_move(s, s->scr_lns - 1, 0);
	_clrtoeol(s);
}

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
			if (col > slp->len)
				memset(slp->data + slp->len, ' ', col - slp->len);
			if (col < slp->smod)
				slp->smod = col;
			newline = false;
		}

		c = *str++;

		if (!isprint2(c)) {
			if (c == '\n' || c == '\r') {
				slp->data[col] = ' ';
				slp->len = col;
				s->cur_col = 0;
				if (col > slp->emod)
					slp->emod = col;
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

	if (col > slp->emod)
		slp->emod = col;
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

/**
 * Print first n bytes of a string.
 * @param str The string.
 * @param n Maximum output bytes.
 * @param ansi Whether ansi control codes should be excluded in length or not.
 */
static void outns(const char *str, int n, bool ansi)
{
	if (!ansi) {
		while (*str != '\0' && n > 0) {
			outc(*str++);
			n--;
		}
	} else {
		while (*str != '\0' && n > 0) {
			n -= outc(*str++);
		}
		outs("\033[m");
	}
}

static inline void ochar(screen_t *s, int c)
{
	telnet_putc(s->tc, c);
}

static void term_move(screen_t *s, int line, int col)
{
	if (col == 0 && line == s->tc_ln + 1) { // newline
		ochar(s, '\n');
		if (s->tc_ln != 0)
			ochar(s, '\r');
	} else if (col == 0 && line == s->tc_ln) { // return
		if (s->tc_ln != 0)
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
	term_move(s, s->cur_col, s->cur_ln);
	s->scroll_cnt = 0;
	s->clear = false;
	telnet_flush(s->tc);
}

void redoscr(void)
{
	_redoscr(&stdscr);
}

static void _refresh(screen_t *s)
{
	if (!buffer_empty(&s->tc->inbuf))
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
			if (slp->emod >= slp->len)
				slp->emod = slp->len - 1;
			term_move(s, i, slp->smod);
			telnet_write(s->tc, slp->data + slp->smod, slp->emod - slp->smod + 1);
			s->tc_col = slp->emod + 1;
		}
		if (slp->oldlen > slp->len) {
			term_move(s, i, slp->len);
			term_cmd(TERM_CMD_CE);
		}
		slp->oldlen = slp->len;
	}
	term_move(s, s->cur_ln, s->cur_col);
	telnet_flush(s->tc);
}

void refresh(void)
{
	_refresh(&stdscr);
}


int dec[] = { 1000000000, 100000000, 10000000, 1000000, 100000, 10000,
		1000, 100, 10, 1 };

/*以ANSI格式输出可变参数的字符串序列*/
void prints(char *fmt, ...) {
	va_list ap;
	char *bp;
	register int i, count, hd, indx;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt == '%') {
			int sgn = 1;
			int sgn2 = 1;
			int val = 0;
			int len, negi;
			fmt++;
			switch (*fmt) {
				case '-':
					while (*fmt == '-') {
						sgn *= -1;
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
					bp = va_arg(ap, char *);
					if (bp == NULL)
						bp = nullstr;
					if (val) {
						register int slen = strlen(bp);
						if (!sgn2) {
							if (val <= slen)
								outns(bp, val, true);
							else
								outns(bp, slen, true);
						} else if (val <= slen)
							outns(bp, val, false);
						else if (sgn > 0) {
							for (slen = val - slen; slen > 0; slen--)
								outc(' ');
							outs(bp);
						} else {
							outs(bp);
							for (slen = val - slen; slen > 0; slen--)
								outc(' ');
						}
					} else
						outs(bp);
					break;
				case 'd':
					i = va_arg(ap, int);

					negi = NA;
					if (i < 0) {
						negi = YEA;
						i *= -1;
					}
					for (indx = 0; indx < 10; indx++)
						if (i >= dec[indx])
							break;
					if (i == 0)
						len = 1;
					else
						len = 10 - indx;
					if (negi)
						len++;
					if (val >= len && sgn > 0) {
						register int slen;
						for (slen = val - len; slen > 0; slen--)
							outc(' ');
					}
					if (negi)
						outc('-');
					hd = 1, indx = 0;
					while (indx < 10) {
						count = 0;
						while (i >= dec[indx]) {
							count++;
							i -= dec[indx];
						}
						indx++;
						if (indx == 10)
							hd = 0;
						if (hd && !count)
							continue;
						hd = 0;
						outc('0' + count);
					}
					if (val >= len && sgn < 0) {
						register int slen;
						for (slen = val - len; slen > 0; slen--)
							outc(' ');
					}
					break;
				case 'c':
					i = va_arg(ap, int);
					outc(i);
					break;
				case '\0':
					goto endprint;
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
	endprint: return;
}

//将big_picture输出位置1,标准输出区间为(cur_col,cur_col)
void standout() {
	register struct screenline *slp;
	register int ln;
	if (dumb_term)
		return;
	if (!standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = YEA;
		slp->sso = cur_col;
		slp->eso = cur_col;
		slp->mode |= STANDOUT;
	}
}
//	如果standing为真,将当前行在big_picture中的映射设成真
//		并将eso设成eso,cur_col的最大值
void standend() {
	register struct screenline *slp;
	register int ln;
	if (dumb_term)
		return;
	if (standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = NA;
		slp->eso = Max(slp->eso, cur_col);
	}
}
//	根据mode来决定 保存或恢复行line的内容
//		最多只能保存一行,否则会被抹去
void saveline(int line, int mode) /* 0,2 : save, 1,3 : restore */
{
	register struct screenline *bp = big_picture;
	static char tmp[2][256];
	int x, y;

	switch (mode) {
		case 0:
		case 2:
			strlcpy(tmp[mode/2], bp[line].data, LINELEN);
			tmp[mode/2][bp[line].len]='\0';
			break;
		case 1:
		case 3:
			getyx(&x, &y);
			move(line, 0);
			clrtoeol();
			refresh();
			prints("%s", tmp[(mode-1)/2]);
			move(x, y);
			refresh();
	}
}

/* Added by Ashinmarch on 2007.12.01,
 * to support multi-line msgs
 * It is used to save screen lines overwrited by msgs
 */
void saveline_buf(int line, int mode)//0:save 1:restore
{
    static char temp[MAX_MSG_LINE * 2 + 2][LINELEN];
    struct screenline *bp = big_picture;
    int x, y;
    
    switch (mode) {
        case 0:
            strncpy(temp[line], bp[line].data, LINELEN);
            temp[line][bp[line].len] = '\0';
            break;
        case 1:
            getyx(&x, &y);
            move(line, 0);
            clear_whole_line(line);
            prints("%s", temp[line]);
            move(x,y);
            break;
    }
}
