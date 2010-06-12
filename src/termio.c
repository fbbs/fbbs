#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/telnet.h>

#include "fbbs/util.h"
#include "fbbs/termio.h"

/** Telnet option negotiation sequence status. */
enum {
    TELST_NOR,  ///< Normal byte.
    TELST_IAC,  ///< Right after IAC.
    TELST_COM,  ///< Right after IAC DO/DONT/WILL/WONT.
    TELST_SUB,  ///< Right after IAC SB.
    TELST_SBC,  ///< Right after IAC SB [COMMAND].
    TELST_END,  ///< End of an telnet option.
};

/** ESC process status. */
enum {
    ESCST_BEG,  ///< Begin.
    ESCST_CUR,  ///< Cursor keys.
    ESCST_FUN,  ///< Function keys.
    ESCST_ERR,  ///< Parse error.
};

enum {
    DEFAULT_TERM_COLS = 80,
    DEFAULT_TERM_LINES = 24,
};

/**
 * Read raw bytes from fd.
 * @param[in] fd The file descriptor.
 * @param[out] buf The buffer.
 * @param[in] size The buffer size.
 * @return Bytes read, -1 on error.
 */
static int raw_read(int fd, uchar_t *buf, size_t size)
{
    return read(fd, buf, size);
}

/**
 * Write raw bytes to fd.
 * @param[in] fd The file descriptor.
 * @param[in] buf The buffer.
 * @param[in] len Bytes to write.
 * @return Bytes written, -1 on error.
 */
static int raw_write(int fd, const uchar_t *buf, size_t len)
{
    return write(fd, buf, len);
}

/**
 * Get ch from buffer.
 * @param fd The input file descriptor.
 * @param inbuf The input buffer.
 * @return Character on success, -1 on error.
 */
static int buffered_getch(int fd, iobuf_t *inbuf)
{
    int ret;
    if (inbuf->cur >= inbuf->size) {
        ret = raw_read(fd, inbuf->buf, sizeof(inbuf->buf));
        if (ret < 0)
            return -1;
        inbuf->cur = 0;
        inbuf->size = ret;
    }
    return inbuf->buf[inbuf->cur++];
}

/**
 * Get ch from a telnet connection, with IAC handled.
 * @param tc Telnet connection data.
 * @return The first character after IAC sequence.
 */
static int telnet_getch(telconn_t *tc)
{
    int ch = buffered_getch(tc->fd, &tc->inbuf);
    if (ch != KEY_ESC)
        return ch;

    int status = TELST_IAC;
    while (status != TELST_END) {
        int ch = buffered_getch(tc->fd, &tc->inbuf);
        if (ch < 0)
            return ch;
        switch (status) {
            case TELST_IAC:
                if (ch == SB)
                    status = TELST_SUB;
                else
                    status = TELST_COM;
                break;
            case TELST_COM:
                status = TELST_END;
                break;
            case TELST_SUB:
                if (ch == SE)
                    status = TELST_END;
                break;
            default:
                break;
        }
    }
    return 0;
}

/**
 * Handle ANSI ESC sequences.
 * @param tc Telnet connection data.
 * @return Converted key on success, next key on error.
 */
static int esc_handler(telconn_t *tc)
{
    int status = ESCST_BEG, ch, last = 0;
    while (1) {
        ch = telnet_getch(tc);
        if (ch < 0)
            return ch;
        switch (status) {
            case ESCST_BEG:
                if (ch == '[' || ch == 'O')
                    status = ESCST_CUR;
                else if (ch == '1' || ch == '4')
                    status = ESCST_FUN;
                else
                    return KEY_ESC;
                break;
            case ESCST_CUR:
                if (ch >= 'A' && ch <= 'D')
                    return KEY_UP + ch - 'A';
                else if (ch >= '1' && ch <= '6')
                    status = ESCST_FUN;
                else
                    status = ESCST_ERR;
                break;
            case ESCST_FUN:
                if (ch == '~' && last >= '1' && last <= '6')
                    return KEY_HOME + last - '1';
                else
                    status = ESCST_ERR;
                break;
            case ESCST_ERR:
                return ch;
            default:
                break;
        }
        last = ch;
    }
    return 0;
}

/**
 * Get ch from telnet connection, with ESC sequence handled.
 * @param tc Telnet connection data.
 * @return
 */
static int term_getch(telconn_t *tc)
{
    int ch;
    while (1) {
        ch = telnet_getch(tc);
        switch (ch) {
            case KEY_ESC:
                ch = esc_handler(tc);
                break;
            case KEY_CTRL_L:
                // TODO: redoscr().
                continue;
            case '\r':
                ch = '\n';
                tc->cr = true;
                break;
            case '\n':
                if (tc->cr) {
                    tc->cr = false;
                    continue;
                }
                break;
            case '\0':
                tc->cr = false;
                continue;
            default:
                tc->cr = false;
                break;
        }
        break;
    }
    return ch;
}

void telnet_init(telconn_t *tc, int fd)
{
	tc->fd = fd;
	tc->cr = false;
	tc->lines = DEFAULT_TERM_LINES;
	tc->cols = DEFAULT_TERM_COLS;
	tc->inbuf.cur = 0;
	tc->inbuf.size = 0;
	tc->outbuf.cur = 0;
	tc->outbuf.size = 0;
}

int telnet_flush(telconn_t *tc)
{
	int ret = 0;
	if (tc->outbuf.size > 0)
		ret = raw_write(tc->fd, tc->outbuf.buf, tc->outbuf.size);
	tc->outbuf.size = 0;
	return ret;
}

void telnet_write(telconn_t *tc, const uchar_t *str, int size)
{
	while (size > 0) {
		int len = sizeof(tc->outbuf.buf) - tc->outbuf.size;
		if (size > len) {
			memcpy(tc->outbuf.buf + tc->outbuf.size, str, len);
			tc->outbuf.size += len;
			telnet_flush(tc);
			size -= len;
			str += len;
		} else {
			memcpy(tc->outbuf.buf + tc->outbuf.size, str, size);
			tc->outbuf.size += size;
			break;
		}
	}
}

int telnet_putc(telconn_t *tc, int c)
{
	tc->outbuf.buf[tc->outbuf.size++] = c;
	if (tc->outbuf.size == sizeof(tc->outbuf.buf))
		return telnet_flush(tc);
	else
		return 0;
}

bool buffer_empty(const iobuf_t *buf)
{
    return (buf->cur >= buf->size);
}

#if 0
int getdata(int line, int col, const char *prompt, char *buf, int len,
		int echo, int clearlabel)
{
	int ch, clen = 0, curr = 0, x, y;
	int currDEC=0, i, patch=0;
	char tmp[STRLEN];
	extern unsigned char scr_cols;
	extern int RMSG;
	extern int msg_num;

	if (clearlabel == YEA)
		buf[0] = '\0';
	move(line, col);
	if (prompt)
		prints("%s", prompt);
	y = line;
	col += (prompt == NULL) ? 0 : strlen(prompt);
	x = col;
	buf[len - 1] = '\0';
	curr = clen = strlen(buf);
	buf[curr] = '\0';
	prints("%s", buf);

	if (dumb_term || echo == NA) {
		while ((ch = igetkey()) != '\n') {
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
				ochar(Ctrl('H'));
				ochar(' ');
				ochar(Ctrl('H'));
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
				ochar(ch);
			else
				ochar('*');
		}
		buf[clen] = '\0';
		outc('\n');
		oflush();
		return clen;
	}
	clrtoeol();
	while (1) {
		if ( (uinfo.in_chat == YEA || uinfo.mode == TALK || uinfo.mode
				== FIVE) && RMSG == YEA) {
			refresh();
		}
		ch = igetkey();
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
		if (x + clen >= scr_cols || clen >= len - 1) {
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

	if (selboard)
		snprintf(buf, sizeof(buf), "ÌÖÂÛÇø [%s]", currboard);
	else {
		brc_initial(currentuser.userid, DEFAULTBOARD);
		changeboard(&currbp, currboard, DEFAULTBOARD);
		if (!getbnum(currboard, &currentuser))
			setoboard(currboard);
		selboard = 1;
		sprintf(buf, "ÌÖÂÛÇø [%s]", currboard);
	}
	return buf;
}

void update_endline(void)
{
	extern time_t login_start_time; //main.c
	extern int WishNum; //main.c
	extern int orderWish; //main.c
	extern char GoodWish[][STRLEN - 3]; //main.c

	char buf[255], fname[STRLEN], *ptr, date[STRLEN];
	time_t now;
	FILE *fp;
	int i, cur_sec, allstay, foo, foo2;

	move(t_lines - 1, 0);
	clrtoeol();

	if (!DEFINE(DEF_ENDLINE))
		return;

	now = time(NULL);
	strlcpy(date, getdatestring(now, DATE_ZH), sizeof(date));
	cur_sec = now % 10;
	if (cur_sec == 0) {
		nowishfile:
		if (resolve_boards() < 0)
			exit(1);
		strlcpy(date, brdshm->date, 30);
		cur_sec = 1;
	}
	if (cur_sec < 5) {
		allstay = (now - login_start_time) / 60;
		sprintf(buf, "[\033[36m%.12s\033[33m]", currentuser.userid);
		num_alcounter();
		prints(	"\033[1;44;33m[\033[36m%29s\033[33m]"
			"[\033[36m%4d\033[33mÈË/\033[36m%3d\033[33mÓÑ]"
			"[\033[36m%c%c%c%c%c%c\033[33m]"
			"ÕÊºÅ%-24s[\033[36m%3d\033[33m:\033[36m%2d\033[33m]\033[m",
			date, get_online(), count_friends,
			(uinfo.pager & ALL_PAGER) ? 'P' : 'p',
			(uinfo.pager & FRIEND_PAGER) ? 'O' : 'o',
			(uinfo.pager & ALLMSG_PAGER) ? 'M' : 'm',
			(uinfo.pager & FRIENDMSG_PAGER) ? 'F' : 'f',
			(DEFINE(DEF_MSGGETKEY)) ? 'X' : 'x',
			(uinfo.invisible == 1) ? 'C' : 'c',
			buf, (allstay / 60) % 1000, allstay % 60);
		return;
	}

	// To be removed..
	setuserfile(fname, "HaveNewWish");
	if (WishNum == 9999 || dashf(fname)) {
		if (WishNum != 9999)
			unlink(fname);
		WishNum = 0;
		orderWish = 0;

		if (is_birth(currentuser)) {
			strcpy(GoodWish[WishNum],
					"                     À²À²¡«¡«£¬ÉúÈÕ¿ìÀÖ!"
					"   ¼ÇµÃÒªÇë¿ÍÓ´ :P                   ");
			WishNum++;
		}

		setuserfile(fname, "GoodWish");
		if ((fp = fopen(fname, "r")) != NULL) {
			for (; WishNum < 20;) {
				if (fgets(buf, 255, fp) == NULL)
					break;
				buf[STRLEN - 4] = '\0';
				ptr = strtok(buf, "\n\r");
				if (ptr == NULL || ptr[0] == '#')
					continue;
				strcpy(buf, ptr);
				for (ptr = buf; *ptr == ' ' && *ptr != 0; ptr++)
					;
				if (*ptr == 0 || ptr[0] == '#')
					continue;
				for (i = strlen(ptr) - 1; i < 0; i--)
					if (ptr[i] != ' ')
						break;
				if (i < 0)
					continue;
				foo = strlen(ptr);
				foo2 = (STRLEN - 3 - foo) / 2;
				strcpy(GoodWish[WishNum], "");
				for (i = 0; i < foo2; i++)
					strcat(GoodWish[WishNum], " ");
				strcat(GoodWish[WishNum], ptr);
				for (i = 0; i < STRLEN - 3 - (foo + foo2); i++)
					strcat(GoodWish[WishNum], " ");
				GoodWish[WishNum][STRLEN - 4] = '\0';
				WishNum++;
			}
			fclose(fp);
		}
	}
	if (WishNum == 0)
		goto nowishfile;
	if (orderWish >= WishNum * 2)
		orderWish = 0;
	prints("\033[0;1;44;33m[\033[36m%77s\033[33m]\033[m", GoodWish[orderWish / 2]);
	orderWish++;
}

void showtitle(char *title, char *mid)
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
	spc1 = (spc1 > 2) ? spc1 : 2; //·ÀÖ¹¹ýÐ¡
	spc2 = spc1 / 2;
	spc1 -= spc2;
	move(0, 0);
	clrtoeol();
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

void firsttitle(char *title) {
	extern int mailXX; //main.c
	extern char BoardName[]; //main.c
	char middoc[30];

	if (chkmail())
		strcpy(middoc, strstr(title, "ÌÖÂÛÇøÁÐ±í") ? "[ÄúÓÐÐÅ¼þ£¬°´ M ¿´ÐÂÐÅ]"
				: "[ÄúÓÐÐÅ¼þ]");
	else if (mailXX == 1)
		strcpy(middoc, "[ÐÅ¼þ¹ýÁ¿£¬ÇëÕûÀíÐÅ¼þ!]");
	else
		strcpy(middoc, BoardName);

	showtitle(title, middoc);
}

// Show 'title' on line 0, 'prompt' on line1.
void docmdtitle(char *title, char *prompt) {
	firsttitle(title);
	move(1, 0);
	clrtoeol();
	prints("%s", prompt);
	clrtoeol();
}

/* Added by Ashinmarch on 2007.12.01
 * used to support display of multi-line msgs
 * */
int show_data(const char *buf, int maxcol, int line, int col)
{
	bool chk = false;
	size_t len = strlen(buf);
	int i, x, y;
	getyx(&y, &x);
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

int multi_getdata(int line, int col, int maxcol, char *prompt, char *buf, int len, int maxline, int clearlabel, int textmode)
{
	extern int RMSG;
	extern int msg_num;
	int ch, x, y, startx, starty, curr, i, k, chk, cursorx, cursory, size;
	bool init = true;
	char tmp[MAX_MSG_SIZE+1];
	int ingetdata = true;

	if (clearlabel == YEA)
		memset(buf, 0, sizeof(buf));
	move(line, col);
	if (prompt)
		prints("%s", prompt);
	getyx(&starty, &startx);
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
		//ÒÔÏÂ±éÀúbufµÄ¹¦ÄÜÊÇÏÔÊ¾³öÃ¿´ÎigetkeyµÄ¶¯×÷¡£
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
				//Ctrl('H')ÖÐÍËÐÐbug
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
			if(i == curr - 1) { //´òÓ¡µ½buf×îºóÒ»¸ö×Ö·ûÊ±µÄxºÍyÊÇÏÂÒ»²½³õÊ¼xy
				cursory = y;
				cursorx = x;
			}
		}
		clrtoeol();
		move(cursory, cursorx);
		ch = igetkey();

		if ((RMSG == YEA) && msg_num == 0) 
		{
			if (ch == Ctrl('Z') ) 
			{
				buf[0] = Ctrl('Z');
				x = 1;
				break; //¿ÉÒÔ¸Ä³Éreturn Ä³¸öÐÐÊÔÊÔ
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
					//²ÉÓÃÏÈ²åÈëºó¼ì²éÊÇ·ñ³¬¹ýmaxline£¬Èç¹û³¬¹ý£¬ÄÇÃ´É¾È¥Õâ¸ö×Ö·ûµ÷Õû
					if (y - starty + 1 > maxline) {
						memmove(buf + curr -1, buf + curr, size - curr + 1);
						curr--;
					}
				}
				init=false;
				break;
		}
	}

	ingetdata = false;
	return y-starty+1;
}
#endif
