#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

extern int enabledbchar;

int getdata(int line, int col, const char *prompt, char *buf, int len,
		int echo, int clearlabel)
{
	int ch, clen = 0, curr = 0, x, y;
	int currDEC=0, i, patch=0;
	char tmp[STRLEN];
	extern int RMSG;
	extern int msg_num;

	if (clearlabel == YEA)
		buf[0] = '\0';
	move(line, col);
	if (prompt)
		prints("%s", prompt);
	screen_coordinates(&y, &x);
	col += (prompt == NULL) ? 0 : strlen(prompt);
	x = col;
	buf[len - 1] = '\0';
	curr = clen = strlen(buf);
	buf[curr] = '\0';
	prints("%s", buf);

	if (dumb_term || echo == NA) {
		while ((ch = terminal_getchar()) != '\n') {
			if (RMSG == YEA && msg_num == 0) {
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
			if (ch == '\n')
				break;
			if (ch == '\177' || ch == Ctrl('H')) {
				if (clen == 0) {
					continue;
				}
				clen--;
				terminal_putchar(Ctrl('H'));
				terminal_putchar(' ');
				terminal_putchar(Ctrl('H'));
				continue;
			}
			if (!isprint2(ch)) {
				continue;
			}
			if (clen >= len - 1) {
				continue;
			}
			buf[clen++] = ch;
			if (echo)
				terminal_putchar(ch);
			else
				terminal_putchar('*');
		}
		buf[clen] = '\0';
		outc('\n');
		terminal_flush();
		return clen;
	}
	clrtoeol();
	while (1) {
		if (RMSG) {
			refresh();
		}
		ch = terminal_getchar();
		if ((RMSG == YEA) && msg_num == 0) {
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
		if (ch == Ctrl('R')) {
			enabledbchar=~enabledbchar&1;
			continue;
		}
		if (ch == '\177' || ch == Ctrl('H')) {
			if (curr == 0) {
				continue;
			}
			currDEC = patch = 0;
			if (enabledbchar&&buf[curr-1]&0x80) {
				for (i=curr-2; i>=0&&buf[i]&0x80; i--)
					patch ++;
				if (patch%2==0 && buf[curr]&0x80)
					patch = 1;
				else if (patch%2)
					patch = currDEC = 1;
				else
					patch = 0;
			}
			if (currDEC)
				curr --;
			strcpy(tmp, &buf[curr+patch]);
			buf[--curr] = '\0';
			(void) strcat(buf, tmp);
			clen--;
			if (patch)
				clen --;
			move(y, x);
			prints("%s", buf);
			clrtoeol();
			move(y, x + curr);
			continue;
		}
		if (ch == KEY_DEL) {
			if (curr >= clen) {
				curr = clen;
				continue;
			}
			strcpy(tmp, &buf[curr + 1]);
			buf[curr] = '\0';
			(void) strcat(buf, tmp);
			clen--;
			move(y, x);
			prints("%s", buf);
			clrtoeol();
			move(y, x + curr);
			continue;
		}
		if (ch == KEY_LEFT) {
			if (curr == 0) {
				continue;
			}
			curr--;
			move(y, x + curr);
			continue;
		}
		if (ch == Ctrl('E') || ch == KEY_END) {
			curr = clen;
			move(y, x + curr);
			continue;
		}
		if (ch == Ctrl('A') || ch == KEY_HOME) {
			curr = 0;
			move(y, x + curr);
			continue;
		}
		if (ch == KEY_RIGHT) {
			if (curr >= clen) {
				curr = clen;
				continue;
			}
			curr++;
			move(y, x + curr);
			continue;
		}
		if (!isprint2(ch)) {
			continue;
		}
		if (clen >= len - 1) {
			continue;
		}
		if (!buf[curr]) {
			buf[curr + 1] = '\0';
			buf[curr] = ch;
		} else {
			strlcpy(tmp, &buf[curr], len);
			buf[curr] = ch;
			buf[curr + 1] = '\0';
			strncat(buf, tmp, len - curr);
		}
		curr++;
		clen++;
		move(y, x);
		prints("%s", buf);
		move(y, x + curr);
	}
	buf[clen] = '\0';
	if (echo) {
		move(y, x);
		prints("%s", buf);
	}
	outc('\n');
	refresh();
	return clen;
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

void update_endline(void)
{
	extern time_t login_start_time; //main.c

	char buf[255], date[STRLEN];
	int cur_sec, allstay;

	screen_move_clear(-1);

	if (!DEFINE(DEF_ENDLINE))
		return;

	fb_time_t now = fb_time();
	strlcpy(date, format_time(now, TIME_FORMAT_ZH), sizeof(date));
	cur_sec = now % 10;
	if (cur_sec == 0) {
		if (resolve_boards() < 0)
			exit(1);
		strlcpy(date, brdshm->date, 30);
		cur_sec = 1;
	}
	if (cur_sec < 5) {
		allstay = (now - login_start_time) / 60;
		sprintf(buf, "[\033[36m%.12s\033[33m]", currentuser.userid);
		prints(	"\033[1;44;33m[\033[36m%29s\033[33m]"
			//% "[\033[36m%4d\033[33m人/\033[36m%3d\033[33m友]"
			"[\033[36m%4d\033[33m\xc8\xcb/\033[36m%3d\033[33m\xd3\xd1]"
			"      "
			//% "帐号%-24s[\033[36m%3d\033[33m:\033[36m%2d\033[33m]\033[m",
			"\xd5\xca\xba\xc5%-24s[\033[36m%3d\033[33m:\033[36m%2d\033[33m]\033[m",
			date, session_count_online(),
			session_count_online_followed(!HAS_PERM(PERM_SEECLOAK)),
			buf, (allstay / 60) % 1000, allstay % 60);
		return;
	}

	if (is_birth(&currentuser)) {
		//% "                     啦啦～～，生日快乐!"
		//% "   记得要请客哟 :P                   ");
		prints("\033[0;1;44;33m[\033[36m                     \xc0\xb2\xc0\xb2\xa1\xab\xa1\xab\xa3\xac\xc9\xfa\xc8\xd5\xbf\xec\xc0\xd6!"
				"   \xbc\xc7\xb5\xc3\xd2\xaa\xc7\xeb\xbf\xcd\xd3\xb4 :P                   \033[33m]\033[m");
	}
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
	update_endline();
	move(1, 0);
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
	move(line, col);
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
				move(++line, col);
				clrtoeol();
			}
			outc(buf[i]);
			col++;
		} else {
			col = 0;
			move(++line, col);
			clrtoeol();
		}
	}
	move(y, x);
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
	move(line, col);
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
		move(y, x);
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
					move(y, x);
				}
				//Ctrl('H')中退行bug
				if (x == maxcol && y - starty + 1 < MAX_MSG_LINE) {
					move(y + 1, 0);
					clrtoeol();
					move(y, x);
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
				move(y, x);
			}
			if(i == curr - 1) { //打印到buf最后一个字符时的x和y是下一步初始xy
				cursory = y;
				cursorx = x;
			}
		}
		clrtoeol();
		move(cursory, cursorx);
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
				move(starty+k,0);
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
	refresh();
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
		move(-1, 0);
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


