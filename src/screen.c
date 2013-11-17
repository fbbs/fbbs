#include <arpa/telnet.h>
#include "bbs.h"
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
	uint16_t old_len; ///< 上次输出时的字符串长度
	uint16_t len; ///< 当前字符串长度
	bool modified; ///< 自从上次输出以来是否修改过
	uchar_t data[SCREEN_LINE_LEN]; ///< 缓冲区
} screen_line_t;

extern int iscolor;

bool dumb_term = true;

static int cur_ln = 0;  ///< Current line.
static int cur_col = 0; ///< Current column.
static int roll; //roll 表示首行在screen.buf的偏移量
//因为随着光标滚动,screen.buf[0]可能不再保存第一行的数据
static int scrollcnt;
static int tc_col;      ///< Terminal's current column.
static int tc_line;     ///< Terminal's current line.
unsigned char docls;

typedef struct {
	screen_line_t *buf;
	int columns;
	int lines;
} screen_t;

static screen_t screen = {
	.lines = 24,
	.columns = 255,
};

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

	uchar_t buf[80];
	int len = terminal_read(buf, sizeof(buf));
	if (len == 12) {
		if (buf[0] != IAC || buf[1] != WILL || buf[2] != TELOPT_NAWS)
			return;
		if (buf[3] != IAC || buf[4] != SB || buf[5] != TELOPT_NAWS
				|| buf[10] != IAC || buf[11] != SE)
			return;
		screen.lines = buf[9];
	}
	if (len == 9) {
		if (buf[0] != IAC || buf[1] != SB || buf[2] != TELOPT_NAWS
				|| buf[7] != IAC || buf[8] != SE)
			return;
		screen.lines = buf[6];
	}

	if (screen.lines < MIN_SCREEN_LINES)
		screen.lines = MIN_SCREEN_LINES;
	if (screen.lines > MAX_SCREEN_LINES)
		screen.lines = MAX_SCREEN_LINES;
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

void screen_init(void)
{
	if (!dumb_term && !screen.buf)
		screen.columns = WRAPMARGIN;

	if (screen.columns > SCREEN_LINE_LEN)
		screen.columns = SCREEN_LINE_LEN;

	screen.buf = malloc(screen.lines * sizeof(*screen.buf));
	for (int i = screen.lines - 1; i >= 0; --i) {
		screen_line_t *sl = screen.buf + i;
		sl->modified = false;
		sl->len = 0;
		sl->old_len = 0;
	}
	docls = YEA;
	roll = 0;
}

/**
 * Generate and send terminal move cmd.
 * @param col Column to move to.
 * @param line Line to move to.
 */
static void do_move(int col, int line)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "\033[%d;%dH", line + 1, col + 1);
	char *p;
	for (p = buf; *p != '\0'; p++)
		terminal_putchar(*p);
}

//	从老位置(was_col,was_ln)移动到新位置(new_col,new_ln)
static void rel_move(int was_col, int was_ln, int new_col, int new_ln)
{
	if (new_ln >= screen.lines || new_col >= screen.columns) //越界,返回
		return;
	tc_col = new_col;
	tc_line = new_ln;
	if ((new_col == 0) && (new_ln == was_ln + 1)) { //换行
		terminal_putchar('\n');
		if (was_col != 0) //到第一列位置,返回
			terminal_putchar('\r');
		return;
	}
	if ((new_col == 0) && (new_ln == was_ln)) { //不换行,到第一列位置,并返回
		if (was_col != 0)
			terminal_putchar('\r');
		return;
	}
	if (was_col == new_col && was_ln == new_ln)
		return;
	if (new_col == was_col - 1 && new_ln == was_ln) {
		terminal_putchar(Ctrl('H'));
		return;
	}
	do_move(new_col, new_ln);
}

/**
 * Redraw the screen.
 */
void screen_redraw(void)
{
	if (dumb_term)
		return;
	ansi_cmd(ANSI_CMD_CL);
	tc_col = 0;
	tc_line = 0;
	for (int i = 0; i < screen.lines; ++i) {
		screen_line_t *sl = screen.buf + (i + roll) % screen.lines;
		if (!sl->len)
			continue;
		rel_move(tc_col, tc_line, 0, i);
		terminal_write_cached(sl->data, sl->len);
		tc_col += sl->len;
		if (tc_col >= screen.columns) {
			tc_col = screen.columns - 1;
		}
		sl->modified = false;
		sl->old_len = sl->len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	docls = NA;
	scrollcnt = 0;
	terminal_flush();
}

void refresh(void)
{
	screen_line_t *bp = screen.buf;
	if (!bp) {
		terminal_flush();
		return;
	}

	if (!terminal_input_buffer_empty())
		return;
	if ((docls) || (abs(scrollcnt) >= (screen.lines - 3))) {
		screen_redraw();
		return;
	}
	if (scrollcnt < 0) {
		rel_move(tc_col, tc_line, 0, 0);
		while (scrollcnt < 0) {
			ansi_cmd(ANSI_CMD_SR);
			scrollcnt++;
		}
	}
	if (scrollcnt > 0) {
		rel_move(tc_col, tc_line, 0, screen.lines - 1);
		while (scrollcnt > 0) {
			terminal_putchar('\n');
			scrollcnt--;
		}
	}
	for (int i = 0; i < screen.lines; i++) {
		int j = (i + roll) % screen.lines;
		if (bp[j].modified) {
			bp[j].modified = false;
			rel_move(tc_col, tc_line, 0, i);
			terminal_write_cached(bp[j].data, bp[j].len);
			tc_col = bp[j].len;
			if (tc_col >= screen.columns) {
				tc_col -= screen.columns;
				tc_line++;
				if (tc_line >= screen.lines)
					tc_line = screen.lines - 1;
			}
		}
		if (bp[j].old_len > bp[j].len) {
			rel_move(tc_col, tc_line, bp[j].len, i);
			ansi_cmd(ANSI_CMD_CE);
		}
		bp[j].old_len = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	terminal_flush();
}

/**
 * Move to given position.
 * @param y Line number.
 * @param x Column number.
 */
void move(int y, int x)
{
	cur_col = x;
	cur_ln = y < 0 ? y + screen.lines : y;
}

/**
 * Get current position.
 * @param[out] y Line number.
 * @param[out] x Column number.
 */
void getyx(int *y, int *x)
{
	*y = cur_ln;
	*x = cur_col;
}

/**
 * Reset screen and move to (0, 0).
 */
void clear(void)
{
	if (dumb_term)
		return;
	roll = 0;
	docls = YEA;
	screen_line_t *slp;
	int i;
	for (i = 0; i < screen.lines; i++) {
		slp = screen.buf + i;
		slp->modified = false;
		slp->len = 0;
		slp->old_len = 0;
	}
	move(0, 0);
}

//清除screen.buf中的第i行,将mode与len置0
void clear_whole_line(int i)
{
	register screen_line_t *slp = &screen.buf[i];
	slp->modified = false;
	slp->len = 0;
	slp->old_len = 79;
}

/**
 * Clear to end of current line.
 */
void clrtoeol(void)
{
	if (dumb_term)
		return;
	screen_line_t *slp = screen.buf + (cur_ln + roll) % screen.lines;
	if (cur_col > slp->len)
		memset(slp->data + slp->len, ' ', cur_col - slp->len + 1);
	slp->len = cur_col;
}

//从当前行清除到最后一行
void clrtobot(void)
{
	register screen_line_t *slp;
	register int i, j;
	if (dumb_term)
		return;
	for (i = cur_ln; i < screen.lines; i++) {
		j = i + roll;
		while (j >= screen.lines)
			//求j%screen.lines ? 因为减法比取余时间少?
			j -= screen.lines;
		slp = &screen.buf[j];
		slp->modified = false;
		slp->len = 0;
		if (slp->old_len > 0)
			slp->old_len = 255;
	}
}

static char nullstr[] = "(null)";

/**
 * Output a character.
 * @param c The character.
 * @return 1, 0 on ansi control codes.
 */
int outc(int c)
{
	static bool inansi;
#ifndef BIT8
	c &= 0x7f;
#endif

	if (inansi) {
		if (c == 'm') {
			inansi = false;
			return 0;
		}
		return 0;
	}
	if (c == KEY_ESC && !iscolor) {
		inansi = true;
		return 0;
	}

	if (dumb_term) {
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

	screen_line_t *slp = screen.buf + (cur_ln + roll) % screen.lines;
	unsigned int col = cur_col;

	if (!isprint2(c)) {
		if (c == '\n' || c == '\r') {
			if (col > slp->len)
				memset(slp->data + slp->len, ' ', col - slp->len + 1);
			slp->len = col;
			cur_col = 0;
			if (cur_ln < screen.lines)
				cur_ln++;
			return 1;
		} else {
			if (c != KEY_ESC || !showansi)
				c = '*';
		}
	}

	if (col > slp->len)
		memset(slp->data + slp->len, ' ', col - slp->len);
	slp->modified = true;
	slp->data[col] = c;
	col++;
	if (col > slp->len)
		slp->len = col;

	if (col >= screen.columns) {
		col = 0;
		if (cur_ln < screen.lines)
			cur_ln++;
	}
	cur_col = col; /* store cur_col back */
	return 1;
}

/**
 * Output a string.
 * @param str The string.
 */
void outs(const char *str)
{
	while (*str != '\0') {
		outc(*str++);
	}
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

int dec[] = { 1000000000, 100000000, 10000000, 1000000, 100000, 10000,
		1000, 100, 10, 1 };

/*以ANSI格式输出可变参数的字符串序列*/
void prints(const char *fmt, ...)
{
	va_list ap;
	char *bp;
	int count, hd, indx;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt == '%') {
			int sgn = 1;
			int sgn2 = 1;
			int val = 0;
			int len, negi;
			bool long_ = false;
			fmt++;
			switch (*fmt) {
				case '-':
					while (*fmt == '-') {
						sgn *= -1;
						fmt++;
					}
					break;
				case 'l':
					long_ = true;
					++fmt;
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
					{
						int64_t n;
						if (long_)
							n = va_arg(ap, int64_t);
						else
							n = va_arg(ap, int);

						negi = NA;
						if (n < 0) {
							negi = YEA;
							n *= -1;
						}
						for (indx = 0; indx < 10; indx++)
							if (n >= dec[indx])
								break;
						if (n == 0)
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
							while (n >= dec[indx]) {
								count++;
								n -= dec[indx];
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
					}
					break;
				case 'c': {
						int i = va_arg(ap, int);
						outc(i);
					}
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

/**
 * Scroll down one line.
 */
void scroll(void)
{
	if (dumb_term) {
		prints("\n");
		return;
	}
	scrollcnt++;
	roll++;
	if (roll >= screen.lines)
		roll -= screen.lines;
	move(screen.lines - 1, 0);
	clrtoeol();
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
		getyx(&x, &y);
		move(line, 0);
		clrtoeol();
		prints("%s", saved);
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
    static char temp[MAX_MSG_LINE * 2 + 2][SCREEN_LINE_LEN];
    screen_line_t *bp = screen.buf;
    int x, y;
    
    switch (mode) {
        case 0:
            strncpy(temp[line], (const char *)bp[line].data, SCREEN_LINE_LEN);
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
