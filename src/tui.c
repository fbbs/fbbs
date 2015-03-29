#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

static int remove_character(char *buf, int *cur, int clen, bool backspace)
{
	if (*cur <= 0 || clen <= 0)
		return 0;

	int pos = 0, dec = 0;
	for (int i = 0; i < clen; ) {
		pos = i;
		dec = (buf[i] & 0x80) ? 2 : 1;
		i += dec;
		if ((!backspace && (pos >= *cur || i > *cur))
				|| (backspace && i >= *cur))
			break;
	}

	int remaining = clen - pos - dec;
	if (remaining > 0)
		memmove(buf + pos, buf + pos + dec, remaining);
	*cur = pos;

	clen -= dec;
	if (clen < 0) {
		dec += clen;
		clen = 0;
	}
	buf[clen] = '\0';
	return dec;
}

static int _tui_input(int line, int col, const char *prompt, char *buf,
		int len, int echo, int clear, bool utf8)
{
	extern int RMSG;
	extern int msg_num;

	if (clear)
		buf[0] = '\0';
	buf[len - 1] = '\0';

	int real_x = prompt ? screen_display_width(prompt, utf8) : 0;

	int cur, clen;
	cur = clen = strlen(buf);

	while (1) {
		screen_move(line, col);
		clrtoeol();
		if (prompt) {
			if (utf8)
				screen_printf("%s", prompt);
			else
				prints("%s", prompt);
		}
		if (echo)
			prints("%s", buf);
		else
			tui_repeat_char('*', clen);
		screen_move(line, real_x + cur);

		if (RMSG)
			screen_flush();

		int ch = terminal_getchar();

		if (RMSG && msg_num == 0) {
			if (ch == Ctrl('Z') || ch == KEY_UP) {
				buf[0] = Ctrl('Z');
				clen = 1;
				break;
			}
			if (ch == Ctrl('A') || ch == KEY_DOWN) {
				buf[0] = Ctrl('A');
				clen = 1;
				break;
			}
		}

		if (ch == '\n' || ch == '\r')
			break;

		if (ch == '\x7f' || ch == Ctrl('H')) {
			clen -= remove_character(buf, &cur, clen, true);
		} else if (ch == KEY_DEL) {
			clen -= remove_character(buf, &cur, clen, false);
		} else if (ch == KEY_LEFT) {
			if (cur > 0)
				--cur;
		} else if (ch == KEY_RIGHT) {
			if (cur < clen)
				++cur;
		} else if (ch == Ctrl('E') || ch == KEY_END) {
			cur = clen;
		} else if (ch == Ctrl('A') || ch == KEY_HOME) {
			cur = 0;
		} else if (isprint2(ch) && clen < len - 1) {
			if (buf[cur] != '\0')
				memmove(buf + cur + 1, buf + cur, clen - cur);
			buf[cur] = ch;
			buf[++cur] = '\0';
			++clen;
		}
	}

	screen_putc('\n');
	screen_flush();
	return clen;
}

int getdata(int line, int col, const char *prompt, char *buf, int len,
		int echo, int clear)
{
	return _tui_input(line, col, prompt, buf, len, echo, clear, false);
}

/**
 * 从终端获取(GBK)输入
 * @param line 输入输出所在行
 * @param prompt 提示字符串, 必须为UTF-8编码
 * @param buf 存放输入字符串的缓冲区
 * @param len 缓冲区长度
 * @param echo 回显输入字符串, 或以*号代替
 * @return 输入字符串的长度
 */
int tui_input(int line, const char *prompt, char *buf, int len, int echo)
{
	return _tui_input(line, 0, prompt, buf, len, echo, false, true);
}

static char *boardmargin(void)
{
	static char buf[STRLEN];

	if (currbp->id)
		//% snprintf(buf, sizeof(buf), "讨论区 [%s]", currboard);
		snprintf(buf, sizeof(buf), "\xcc\xd6\xc2\xdb\xc7\xf8 [%s]", currboard);
	else {
		brc_init(currentuser.userid, DEFAULTBOARD);

		board_t board;
		get_board(DEFAULTBOARD, &board);
		change_board(&board);

		//% sprintf(buf, "讨论区 [%s]", currboard);
		sprintf(buf, "\xcc\xd6\xc2\xdb\xc7\xf8 [%s]", currboard);
	}
	return buf;
}

static bool is_birth(const struct userec *user)
{
	struct tm *tm;
	time_t now;

	now = time(0);
	tm = localtime(&now);

	if (strcasecmp(user->userid, "guest") == 0)
		return NA;

	if (user->birthmonth == (tm->tm_mon + 1)
			&& user->birthday == tm->tm_mday)
		return YEA;
	else
		return NA;
}

static void describe_stay(int stay, char *buf, size_t size)
{
	char str[5];
	if (stay < 10 * 60)
		snprintf(str, sizeof(str), "%d:%2d", stay / 60, stay % 60);
	else if (stay < 1000 * 60)
		snprintf(str, sizeof(str), "%3dh", stay / 60);
	else if (stay < 1000 * 24 * 60)
		snprintf(str, sizeof(str), "%3dd", stay / 60 / 24);
	else
		strlcpy(str, "999+", sizeof(str));

	snprintf(buf, size, "[\033[1;36m%s\033[33m]", str);
}

static void notice_count(int *replies, int *mentions)
{
	static fb_time_t last;

	fb_time_t now = fb_time();
	if (now - last > 10) {
		user_id_t user_id = session_uid();
		*replies = post_reply_get_count(user_id);
		*mentions = post_mention_get_count(user_id);
		last = now;
	} else {
		*replies = post_reply_get_count_cached();
		*mentions = post_mention_get_count_cached();
	}
}

static bool _suppress_notice;

void tui_suppress_notice(bool suppress_notice)
{
	_suppress_notice = suppress_notice;
}

static void notice_string(char *buf, size_t size)
{
	*buf = '\0';

	int replies, mentions;
	notice_count(&replies, &mentions);

	bool empty = !(replies || mentions);
	if (!empty) {
		size_t orig_size = size;
		char **dst = &buf;
		if (replies) {
			char str[24];
			//% %d篇回复
			snprintf(str, sizeof(str), "%d\xc6\xaa\xbb\xd8\xb8\xb4", replies);
			strappend(dst, &size, str);
		}
		if (mentions) {
			char str[24];
			//% %d篇提及
			snprintf(str, sizeof(str), "%d\xc6\xaa\xcc\xe1\xbc\xb0",
					mentions);
			strappend(dst, &size, str);
		}
		//% 按^T查看
		if (!_suppress_notice && orig_size - size <= 22)
			strappend(dst, &size, " \xb0\xb4^T\xb2\xe9\xbf\xb4");
	}
}

void tui_repeat_char(int c, int repeat)
{
	for (int i = 0; i < repeat; ++i)
		outc(c);
}

void tui_update_status_line(void)
{
	extern time_t login_start_time; //main.c

	char date[STRLEN];

	screen_move_clear(-1);

	if (!DEFINE(DEF_ENDLINE))
		return;

	fb_time_t now = fb_time();
	int cur_sec = now % 10;

	if (cur_sec < 5) {
		strlcpy(date, format_time(now, TIME_FORMAT_ZH), sizeof(date));
	} else {
		if (resolve_boards() >= 0)
			strlcpy(date, brdshm->date, 30);
		else
			date[0] = '\0';
	}

	if (cur_sec >= 5 && is_birth(&currentuser)) {
		//% "啦啦～～，生日快乐!   记得要请客哟 :P"
		prints("\033[0;1;44;33m[\033[36m                     "
				"\xc0\xb2\xc0\xb2\xa1\xab\xa1\xab\xa3\xac\xc9\xfa\xc8\xd5"
				"\xbf\xec\xc0\xd6!   \xbc\xc7\xb5\xc3\xd2\xaa\xc7\xeb"
				"\xbf\xcd\xd3\xb4 :P                   \033[33m]\033[m");
	} else {
		int stay = (now - login_start_time) / 60;
		char stay_str[20];
		describe_stay(stay, stay_str, sizeof(stay_str));

		char notice[128];
		notice_string(notice, sizeof(notice));
		int notice_len = strlen(notice);

		prints(	"\033[1;44;33m[\033[36m%29s\033[33m]"
			//% "人 友"
			"[\033[36m%5d\033[33m\xc8\xcb\033[36m%3d\033[33m\xd3\xd1]",
			date, session_count_online(),
			session_count_online_followed(!HAS_PERM(PERM_SEECLOAK)));
		if (notice_len) {
			int space = 25 - notice_len;
			if (space > 0) {
				tui_repeat_char(' ', space);
				prints("[\033[%d;36m%s\033[m\033[1;33;44m]%s\033[m",
						_suppress_notice ? 1 : 5, notice, stay_str);
			} else {
				space += 6;
				if (space > 0)
					tui_repeat_char(' ', space);
				prints("[\033[%d;36m%s\033[m\033[1;33;44m]\033[m",
						_suppress_notice ? 1 : 5, notice);
			}
		} else {
			int space = 25 - strlen(currentuser.userid);
			tui_repeat_char(' ', space);
			prints("[\033[36m%s\033[33m]%s\033[m", currentuser.userid,
					stay_str);
		}
	}
}

static void print_option(const char *opt, char *quantifier, int count,
		int order)
{
	prints(" %d) ", order);
	if (count > 0 && quantifier)
		prints("%d%s", count, quantifier);
	prints("%s", opt);
}

extern int post_list_reply(void);
extern int post_list_mention(void);

int tui_check_notice(const char *board_name)
{
	screen_move_clear(-1);
	prints("\033[m\xb2\xe9\xbf\xb4:");

	int replies, mentions;
	notice_count(&replies, &mentions);

	print_option("\xc8\xa1\xcf\xfb", NULL, 0, 0);
	print_option("\xbb\xd8\xb8\xb4", "\xc6\xaa", replies, 1);
	print_option("\xcc\xe1\xbc\xb0", "\xc6\xaa", mentions, 2);

	int choice = 0;
	if (replies)
		choice = 1;
	else if (mentions)
		choice = 2;
	prints(" [%d]: ", choice);

	char ans[2];
	int x, y;
	screen_coordinates(&x, &y);
	getdata(x, y, "", ans, sizeof(ans), true, true);

	if (*ans == '\0')
		*ans = '0' + choice;
	if (*ans == '1') {
		post_list_reply();
		brc_init(currentuser.userid, board_name);
		return FULLUPDATE;
	}
	if (*ans == '2') {
		post_list_mention();
		brc_init(currentuser.userid, board_name);
		return FULLUPDATE;
	}
	return MINIUPDATE;
}

void showtitle(const char *title, const char *mid)
{
	extern char BoardName[]; //main.c
	char buf[STRLEN], *note;
	int spc1;
	int spc2;

	note = boardmargin();
	spc1 = 39 + num_ans_chr(title) - strlen(title) - strlen(mid) / 2;
	spc2 = 79 - (strlen(title) - num_ans_chr(title) + spc1 + strlen(note)
			+ strlen(mid));
	spc1 += spc2;
	spc1 = (spc1 > 2) ? spc1 : 2; //防止过小
	spc2 = spc1 / 2;
	spc1 -= spc2;
	screen_move_clear(0);
	sprintf(buf, "%*s", spc1, "");
	if (!strcmp(mid, BoardName))
		prints("[1;44;33m%s%s[37m%s[1;44m", title, buf, mid);
	else if (mid[0] == '[')
		prints("[1;44;33m%s%s[5;36m%s[m[1;44m", title, buf, mid);
	else
		prints("[1;44;33m%s%s[36m%s", title, buf, mid);
	sprintf(buf, "%*s", spc2, "");
	prints("%s[33m%s[m\n", buf, note);
	tui_update_status_line();
	screen_move(1, 0);
}

void firsttitle(const char *title)
{
	extern int mailXX; //main.c
	extern char BoardName[]; //main.c
	char middoc[30];

	if (chkmail())
		//% strcpy(middoc, strstr(title, "讨论区列表") ? "[您有信件，按 M 看新信]"
		strcpy(middoc, strstr(title, "\xcc\xd6\xc2\xdb\xc7\xf8\xc1\xd0\xb1\xed") ? "[\xc4\xfa\xd3\xd0\xd0\xc5\xbc\xfe\xa3\xac\xb0\xb4 M \xbf\xb4\xd0\xc2\xd0\xc5]"
				//% : "[您有信件]");
				: "[\xc4\xfa\xd3\xd0\xd0\xc5\xbc\xfe]");
	else if (mailXX == 1)
		//% strcpy(middoc, "[信件过量，请整理信件!]");
		strcpy(middoc, "[\xd0\xc5\xbc\xfe\xb9\xfd\xc1\xbf\xa3\xac\xc7\xeb\xd5\xfb\xc0\xed\xd0\xc5\xbc\xfe!]");
	else
		strcpy(middoc, BoardName);

	showtitle(title, middoc);
}

// Show 'title' on line 0, 'prompt' on line1.
void docmdtitle(const char *title, const char *prompt)
{
	firsttitle(title);
	screen_move_clear(1);
	prints("%s", prompt);
}

/* Added by Ashinmarch on 2007.12.01
 * used to support display of multi-line msgs
 * */
int show_data(const char *buf, int maxcol, int line, int col)
{
	bool chk = false;
	size_t len = strlen(buf);
	int i, x, y;
	screen_coordinates(&y, &x);
	screen_move(line, col);
	clrtoeol();
	for (i = 0; i < len; i++) {
		if (chk) {
			chk = false;
		} else {
			if(buf[i] < 0)
				chk = true;
		}
		if (chk && col >= maxcol)
			col++;
		if (buf[i] != '\r' && buf[i] != '\n') {
			if (col > maxcol) {
				col = 0;
				screen_move(++line, col);
				clrtoeol();
			}
			outc(buf[i]);
			col++;
		} else {
			col = 0;
			screen_move(++line, col);
			clrtoeol();
		}
	}
	screen_move(y, x);
    return line;
}

int multi_getdata(int line, int col, int maxcol, const char *prompt,
		char *buf, int len, int maxline, int clearlabel, int textmode)
{
	extern int RMSG;
	extern int msg_num;
	int ch, x, y, startx, starty, curr, i, k, chk, cursorx, cursory, size;
	bool init = true;
	char tmp[MAX_MSG_SIZE+1];

	if (clearlabel == YEA)
		memset(buf, 0, len);
	screen_move(line, col);
	if (prompt)
		prints("%s", prompt);
	screen_coordinates(&starty, &startx);
	curr = strlen(buf);
	strncpy(tmp, buf, MAX_MSG_SIZE);
	tmp[MAX_MSG_SIZE] = 0;
	cursory = starty;
	cursorx = startx;
	while (true) {
		y = starty;
		x = startx;
		screen_move(y, x);
		chk = 0;
		if (curr == 0) {
			cursory = y;
			cursorx = x;
		}
		//以下遍历buf的功能是显示出每次terminal_getchar的动作。
		size = strlen(buf);
		for (i = 0; i < size; i++) {
			if (chk) {
				chk = 0;
			} else {
				if (buf[i] < 0)
					chk=1;
			}
			if (chk && x >= maxcol)
				x++;
			if (buf[i] != '\r' && buf[i] != '\n') {
				if (x > maxcol) {
					clrtoeol();
					x = 0;
					y++;
					screen_move(y, x);
				}
				//Ctrl('H')中退行bug
				if (x == maxcol && y - starty + 1 < MAX_MSG_LINE) {
					screen_move(y + 1, 0);
					clrtoeol();
					screen_move(y, x);
				}
				if (init)
					prints("\033[4m");
				prints("%c", buf[i]);
				x++;
			}
			else {
				clrtoeol();
				x = 0;
				y++;
				screen_move(y, x);
			}
			if(i == curr - 1) { //打印到buf最后一个字符时的x和y是下一步初始xy
				cursory = y;
				cursorx = x;
			}
		}
		clrtoeol();
		screen_move(cursory, cursorx);
		ch = terminal_getchar();

		if ((RMSG == YEA) && msg_num == 0) 
		{
			if (ch == Ctrl('Z') ) 
			{
				buf[0] = Ctrl('Z');
				x = 1;
				break; //可以改成return 某个行试试
			}
			if (ch == Ctrl('A') ) 
			{
				buf[0] = Ctrl('A');
				x = 1;
				break;
			}
		}

		if(ch == Ctrl('Q'))
		{
			init = true;
			buf[0]=0; curr=0;
			for(k=0; k < MAX_MSG_LINE;k++)
			{
				screen_move(starty+k,0);
				clrtoeol();
			}
			continue;
		}


		if(textmode == 0){
			if ((ch == '\n' || ch == '\r'))
				break;
		}
		else{
			if (ch == Ctrl('W'))
				break;
		}

		switch(ch) {
			case KEY_UP:
				init = false;
				if (cursory > starty) {
					y = starty; x = startx;
					chk = 0;
					if(y == cursory - 1 && x <= cursorx)
						curr = 0;
					size = strlen(buf);
					for (i = 0; i < size; i++) {
						if (chk) {
							chk = 0;
						} else {
							if (buf[i] < 0)
								chk = 1;
						}
						if (chk && x >= maxcol)
							x++;
						if (buf[i] != '\r' && buf[i] != '\n') {
							if(x > maxcol) {
								x = col;
								y++;
							}
							x++;
						}
						else {
							x = col;
							y++;
						}
						if (y == cursory - 1 && x <= cursorx)
							curr = i + 1;
					}
				}
				break;
			case KEY_DOWN:
				init=false;
				if(cursory<y) {
					y = starty; x = startx;
					chk = 0;
					if(y==cursory+1&&x<=cursorx)
						curr=0;
					size = strlen(buf);
					for(i=0; i<size; i++) {
						if(chk) chk=0;
						else if(buf[i]<0) chk=1;
						if(chk&&x>=maxcol) x++;
						if(buf[i]!=13&&buf[i]!=10) {
							if(x>maxcol) {
								x = col;
								y++;
							}
							x++;
						}
						else {
							x = col;
							y++;
						}
						if(y==cursory+1&&x<=cursorx)
							curr=i+1;
					}
				}
				break;
			case '\177':
			case Ctrl('H'):
				if(init) {
					init=false;
					buf[0]=0;
					curr=0;
				}
				if(curr>0) {
					int currDec = 0, patch = 0;
					if(buf[curr-1] < 0){
						for(i = curr - 2; i >=0 && buf[i]<0; i--)
							patch++;
						if(patch%2 == 0 && buf[curr] < 0)
							patch = 1;
						else if(patch%2)
							patch = currDec = 1;
						else
							patch = 0;
					}
					if(currDec) curr--;
					strcpy(tmp, &buf[curr+patch]);
					buf[--curr] = 0;
					strcat(buf, tmp);
				}
				break;
			case KEY_DEL:
				if (init) {
					init = false;
					buf[0] = '\0';
					curr = 0;
				}
				size = strlen(buf);
				if (curr < size)
					memmove(buf + curr, buf + curr + 1, size - curr);
				break;
			case KEY_LEFT:
				init=false;
				if(curr>0) {
					curr--;
				}
				break;
			case KEY_RIGHT:
				init=false;
				if(curr<strlen(buf)) {
					curr++;
				}
				break;
			case KEY_HOME:
			case Ctrl('A'):
				init=false;
				curr--;
				while (curr >= 0 && buf[curr] != '\n' && buf[curr] != '\r')
					curr--;
				curr++;
				break;
			case KEY_END:
			case Ctrl('E'):
				init = false;
				size = strlen(buf);
				while (curr < size && buf[curr] != '\n' && buf[curr] != '\r')
					curr++;
				break;
			case KEY_PGUP:
				init=false;
				curr=0;
				break;
			case KEY_PGDN:
				init=false;
				curr = strlen(buf);
				break;
			default:
				if(isprint2(ch)&&strlen(buf)<len-1) {
					if(init) {
						init=false;
						buf[0]=0;
						curr=0;
					}
					size = strlen(buf);
					memmove(buf + curr + 1, buf + curr, size - curr + 1);
					size++;
					buf[curr++]=ch;
					y = starty; x = startx;
					chk = 0;
					for(i = 0; i < size; i++) {
						if(chk) chk=0;
						else if(buf[i]<0) chk=1;
						if(chk&&x>=maxcol) x++;
						if(buf[i]!=13&&buf[i]!=10) {
							if(x>maxcol) {
								x = col;
								y++;
							}
							x++;
						}
						else {
							x = col;
							y++;
						}
					}
					//采用先插入后检查是否超过maxline，如果超过，那么删去这个字符调整
					if (y - starty + 1 > maxline) {
						memmove(buf + curr -1, buf + curr, size - curr + 1);
						curr--;
					}
				}
				init=false;
				break;
		}
	}

	return y-starty+1;
}

/**
 * Prompt and wait user to press any key.
 * @param[in] msg The prompt message.
 * @param[in] x Line number.
 */
void presskeyfor(const char *msg, int x)
{
	extern int showansi;
	showansi = 1;
	screen_move_clear(x);
	outs(msg);
	egetch();
	screen_move_clear(x);
}

/**
 * Prompt and wait user to press any key.
 */
void pressanykey(void)
{
	//% 按任何键继续...
	presskeyfor("\033[m                                \033[5;1;33m"
			"\xb0\xb4\xc8\xce\xba\xce\xbc\xfc\xbc\xcc\xd0\xf8...\033[m", -1);
}

int pressreturn(void)
{
	extern int showansi;
	char buf[3];
	showansi = 1;
	screen_move_clear(-1);
	getdata(-1, 0,
			//% "                              [1;33m请按 ◆[5;36mEnter[m[1;33m◆ 继续\033[m",
			"                              [1;33m\xc7\xeb\xb0\xb4 \xa1\xf4[5;36mEnter[m[1;33m\xa1\xf4 \xbc\xcc\xd0\xf8\033[m",
			buf, 2, NOECHO, YEA);
	screen_move_clear(-1);
	screen_flush();
	return 0;
}

/**
 * Ask for confirmation.
 * @param str The prompt string.
 * @param defa Default answer.
 * @param gobottom True if prompt at the bottom of the screen.
 * @return True if user answers "y", false if "n", default answer otherwise.
 */
bool askyn(const char *str, bool defa, bool gobottom)
{
	int x, y;
	char buf[100];
	char ans[3];
	snprintf(buf, sizeof(buf), "%s %s", str, (defa) ? "(YES/no)? [Y]"
			: "(yes/NO)? [N]");
	if (gobottom)
		screen_move(-1, 0);
	screen_coordinates(&x, &y);
	clrtoeol();
	getdata(x, y, buf, ans, 2, DOECHO, YEA);
	switch (ans[0]) {
		case 'Y':
		case 'y':
			return true;
		case 'N':
		case 'n':
			return false;
		default:
			return defa;
	}
}

void printdash(const char *mesg)
{
	char buf[80], *ptr;
	int len;
	memset(buf, '=', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > sizeof(buf) - 4)
		len = sizeof(buf) - 4;
		ptr = buf + (sizeof(buf) - 1 - len) / 2 - 1;
		*ptr++ = ' ';
		memcpy(ptr, mesg, len);
		ptr[len] = ' ';
	}
	prints("%s\n", buf);
}

/* 990807.edwardc fix beep sound in bbsd .. */

void bell(void)
{
	terminal_putchar(Ctrl('G'));
}

void stand_title(const char *title)
{
	screen_clear();
	prints(ANSI_CMD_SO"%s"ANSI_CMD_SE, title);
}
