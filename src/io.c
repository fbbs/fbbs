// Terminal I/O handlers.

#ifdef AIX
#include <sys/select.h>
#endif
#include <arpa/telnet.h>
#ifdef SSHBBS
#include <libssh/libssh.h>
#endif // SSHBBS
#include "bbs.h"

enum {
	INPUT_ACTIVE = 0,
	INPUT_IDLE = 1,
};

/** ESC process status */
enum {
	ESCST_BEG,  ///< begin
	ESCST_CUR,  ///< Cursor keys
	ESCST_FUN,  ///< Function keys
	ESCST_ERR,  ///< Parse error
};

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif
extern struct screenline *big_picture;
#ifdef SSHBBS
extern ssh_channel ssh_chan;
#endif // SSHBBS

static iobuf_t inbuf;   ///< Input buffer.
static iobuf_t outbuf;  ///< Output buffer.

int KEY_ESC_arg;
static int i_mode= INPUT_ACTIVE;

// TODO: why here..
struct user_info uinfo;

//	³¬Ê±´¦Àíº¯Êý,½«·ÇÌØÈ¨ID³¬Ê±Ê±Ìß³öbbs
void hit_alarm_clock() {
	if (HAS_PERM(PERM_NOTIMEOUT))
		return;
	if (i_mode == INPUT_IDLE) {
		clear();
		prints("Idle timeout exceeded! Booting...\n");
		kill(getpid(), SIGHUP);
	}
	i_mode = INPUT_IDLE;
	if (uinfo.mode == LOGIN)
		alarm(LOGIN_TIMEOUT);
	else
		alarm(IDLE_TIMEOUT);
}

//³õÊ¼»¯³¬Ê±Ê±ÖÓÐÅºÅ,½«hit_alarm_clockº¯Êý¹ÒÔÚ´ËÐÅºÅ´¦Àí¾ä±úÉÏ
void init_alarm() {
	signal(SIGALRM, hit_alarm_clock);
	alarm(IDLE_TIMEOUT);
}

/**
 *
 */
int read_stdin(char *buf, size_t size)
{
#ifdef SSHBBS
	return channel_read(ssh_chan, buf, size, 0);
#else // SSHBBS
	return read(STDIN_FILENO, buf, size);
#endif // SSHBBS
}

/**
 *
 */
int write_stdout(const char *buf, size_t len)
{
#ifdef SSHBBS
	return channel_write(ssh_chan, buf, len);
#else // SSHBBS
	return write(STDIN_FILENO, buf, len);
#endif // SSHBBS

}

/**
 * Flush output buffer.
 * @return 0 on success, -1 on error.
 */
int oflush(void)
{
	int ret = 0;
	if (outbuf.size > 0)
		ret = write_stdout(outbuf.buf, outbuf.size);
	outbuf.size = 0;
	return ret;
}

/**
 * Put a byte into output buffer.
 * @param ch byte to put.
 */
static void put_raw_ch(int ch)
{
	outbuf.buf[outbuf.size++] = ch;
	if (outbuf.size == sizeof(outbuf.buf))
		oflush();
}

/**
 * Put a byte into output buffer. Do translation if needed.
 * @param ch byte to put.
 */
void ochar(int ch)
{
#ifdef ALLOWSWITCHCODE
	if (convcode) {
		ch = convert_g2b(ch);
		while (ch > 0) {
			put_raw_ch(ch);
			ch = convert_g2b(-1);
		}
	} else {
		put_raw_ch(ch);
	}
#else
	put_raw_ch(ch);
#endif // ALLOWSWITCHCODE
}

/**
 * Put bytes into output buffer.
 * @param str pointer to the first byte.
 * @param size bytes to output.
 * @note IAC is not handled.
 */
void output(const unsigned char *str, int size)
{
	int convert = 0;
#ifdef ALLOWSWITCHCODE
	convert = convcode;
#endif // ALLOWSWITCHCODE
	if (convert) {
		while (size-- > 0)
			ochar(*str++);
	} else {
		while (size > 0) {
			int len = sizeof(outbuf.buf) - outbuf.size;
			if (size > len) {
				memcpy(outbuf.buf + outbuf.size, str, len);
				outbuf.size += len;
				oflush();
				size -= len;
				str += len;
			} else {
				memcpy(outbuf.buf + outbuf.size, str, size);
				outbuf.size += size;
				return;
			}
		}
	}
}

static int i_newfd = 0;
static struct timeval i_to, *i_top = NULL;
static int (*flushf)() = NULL;

void add_io(int fd, int timeout) {
	i_newfd = fd;
	if (timeout) {
		i_to.tv_sec = timeout;
		i_to.tv_usec = 0;
		i_top = &i_to;
	} else
		i_top = NULL;
}

//	½«flushfº¯ÊýÖ¸ÕëÖ¸Ïòº¯Êýflushfunc
void add_flush(int (*flushfunc)()) {
	flushf = flushfunc;
}

bool inbuf_empty(void)
{
	return (inbuf.cur >= inbuf.size);
}

/**
 * Get raw byte from stdin.
 * @return next byte from stdin
 */
static int get_raw_ch(void)
{
	if (inbuf.cur >= inbuf.size) {
		fd_set rset;
		struct timeval to;
		int fd = i_newfd;
		int nfds, ret;

		FD_ZERO(&rset);
		FD_SET(STDIN_FILENO, &rset);
		if (fd) {
			FD_SET(fd, &rset);
			nfds = fd + 1;
		} else {
			nfds = 1;
		}

		// TODO: simplify it
		uinfo.idle_time = time(0);
		update_ulist(&uinfo, utmpent);

		to.tv_sec = to.tv_usec = 0;
		if ((ret = select(nfds, &rset, NULL, NULL, &to)) <= 0) {
			if (flushf)
				(*flushf) ();
			if (big_picture)
				refresh();
			else
				oflush();

			FD_ZERO(&rset);
			FD_SET(0, &rset);
			if (fd)
				FD_SET(fd, &rset);
			while ((ret = select(nfds, &rset, NULL, NULL, i_top)) < 0) {
				if (errno != EINTR)
					return -1;
			}
			if (ret == 0)
				return I_TIMEOUT;
		}
		if (fd && FD_ISSET(fd, &rset))
			return I_OTHERDATA;

		while (1) {
			ret = read_stdin(inbuf.buf, sizeof(inbuf.buf));
			if (ret > 0)
				break;
			if ((ret < 0) && (errno == EINTR))
				continue;
			abort_bbs(0);
		}
		inbuf.cur = 0;
		inbuf.size = ret;
		i_mode = INPUT_ACTIVE;
	}
	return inbuf.buf[inbuf.cur++];
}

/**
 * Handle telnet option negotiation.
 * @return the first character after IAC sequence.
 */
static int iac_handler(void)
{
	int status = TELST_IAC;
	while (1) {
		int ch = get_raw_ch();
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
			case TELST_END:
				return ch;
			default:
				break;
		}	
	}
	return 0;
}

/**
 * Handle ANSI ESC sequences.
 * @return converted key on success, next key on error.
 */
static int esc_handler(void)
{
	int status = ESCST_BEG, ch, last = 0;
	while (1) {
		ch = get_raw_ch();
		if (ch < 0)
			return ch;
		switch (status) {
			case ESCST_BEG:
				if (ch == '[' || ch == 'O')
					status = ESCST_CUR;
				else if (ch == '1' || ch == '4')
					status = ESCST_FUN;
				else {
					KEY_ESC_arg = ch;  // TODO:...
					return KEY_ESC;
				}
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
 * Get next byte from stdin, with special byte interpreted.
 * @return next byte from stdin
 */
int igetch(void)
{
	static bool cr = 0;
	int ch;
	while (1) {
		ch = get_raw_ch();
		switch (ch) {
			case IAC:
				ch = iac_handler();
				break;
			case KEY_ESC:
				ch = esc_handler();
				break;
			case Ctrl('L'):
				redoscr();
				continue;
			case '\r':
				ch = '\n';
				cr = true;
				break;
			case '\n':
				if (cr) {
					cr = false;
					continue;
				}
				break;
			case '\0':
				cr = false;
				continue;
			default:
				cr = false;
#ifdef ALLOWSWITCHCODE
				if (convcode) {
					ch = convert_b2g(ch);
					if (ch >= 0)
						return ch;
				}
#endif // ALLOWSWITCHCODE
				break;
		}
		break;
	}
	return ch;
}

/**
 * Get next byte from stdin in gbk encoding (if conversion is needed).
 */
int igetkey(void)
{
	extern int RMSG;
	int ch;
#ifdef ALLOWSWITCHCODE
	if (convcode) {
		ch = convert_b2g(-1); // If there is a byte left.
		while (ch < 0)
			ch = igetch();
	} else {
		ch = igetch();
	}
#else
	ch = igetch();
#endif // ALLOWSWITCHCODE
	// TODO:...
	if ((ch == Ctrl('Z')) && (RMSG == NA) && uinfo.mode != LOCKSCREEN) {
		r_msg2();
		return 0;
	} else {
		return ch;
	}
}

int egetch(void)
{
	extern int talkrequest; //main.c
	extern int refscreen; //main.c
	int rval;

	check_calltime();
	if (talkrequest) {
		talkreply();
		refscreen = YEA;
		return -1;
	}
	while (1) {
		rval = igetkey();
		if (talkrequest) {
			talkreply();
			refscreen = YEA;
			return -1;
		}
		if (rval != Ctrl('L'))
			break;
		redoscr();
	}
	refscreen = NA;
	return rval;
}

void top_show(char *prompt) {
	if (editansi) {
		outs(ANSI_RESET);
		refresh();
	}
	move(0, 0);
	clrtoeol();
	standout();
	prints("%s", prompt);
	standend();
}

int ask(char *prompt) {
	int ch;
	top_show(prompt);
	ch = igetkey();
	move(0, 0);
	clrtoeol();
	return (ch);
}

extern int enabledbchar;

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
	int y, x;
	getyx(&y, &x);
	bool chk = false;
	size_t len = strlen(buf);
	int i;
	for (i = 0; i < len; i++) {
		if (chk) {
			chk = false;
		} else {
			if(buf[i] < 0)
				chk = true;
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
			outc(buf[i]);
			x++;
		} else {
			clrtoeol();
			x = 0;
			y++;
			move(y, x);
		}
	}
    return y;
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

