// Terminal I/O handlers.

#include "bbs.h"
#ifdef AIX
#include <sys/select.h>
#endif
#include <arpa/telnet.h>

#define OBUFSIZE  (4096)
#define IBUFSIZE  (256)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

extern int dumb_term;

//	¶¨ÒåÊä³ö»º³åÇø,¼°ÆäÒÑ±»Ê¹ÓÃµÄ×Ö½ÚÊý
static char outbuf[OBUFSIZE];
static int obufsize = 0;

struct user_info uinfo;

char inbuf[IBUFSIZE];
int ibufsize = 0;
int icurrchar = 0;
int KEY_ESC_arg;

static int i_mode= INPUT_ACTIVE;
extern struct screenline *big_picture;

#ifdef ALLOWSWITCHCODE

#define BtoGtablefile "etc/b2g_table"
#define GtoBtablefile "etc/g2b_table"

unsigned char* GtoB,* BtoG;

#define GtoB_count 7614
#define BtoG_count 13973

extern int convcode;
extern void redoscr();

//	½«×ÖÂë×ª»»×´Ì¬È¡Äæ, ²¢ÖØ»æÆÁÄ»
int switch_code() {
	convcode=!convcode;
	redoscr();
}

void resolve_GbBig5Files(void)
{
	int fd;
	int i;
	BtoG =(unsigned char *)attach_shm("CONV_SHMKEY", 3013,GtoB_count*2+BtoG_count*2);
	fd = open( BtoGtablefile, O_RDONLY );
	if (fd == -1)
	for (i=0;i< BtoG_count; i++) {
		BtoG[i*2]=0xA1;
		BtoG[i*2+1]=0xF5;
	}
	else
	{
		read(fd,BtoG,BtoG_count*2);
		close(fd);
	}
	fd=open(GtoBtablefile,O_RDONLY);
	if (fd==-1)
	for (i=0;i< GtoB_count; i++) {
		BtoG[BtoG_count*2+i*2]=0xA1;
		BtoG[BtoG_count*2+i*2+1]=0xBC;
	}
	else
	{
		read(fd,BtoG+BtoG_count*2,GtoB_count*2);
		close(fd);
	}
	GtoB = BtoG + BtoG_count*2;
}

//	½«str×Ö·û´®ÖÐµÄGBÂëºº×Ö×ª»»³ÉÏàÓ¦µÄBIG5Âëºº×Ö,²¢µ÷ÓÃwriteº¯ÊýÊä³ö
int write2(int port, char *str, int len) // write gb to big
{
	register int i, locate;
	register unsigned char ch1, ch2, *ptr;

	for(i=0, ptr=str; i < len;i++) {
		ch1 = (ptr+i)[0];
		if(ch1 < 0xA1 || (ch1> 0xA9 && ch1 < 0xB0) || ch1> 0xF7)
		continue;
		ch2 = (ptr+i)[1];
		i ++;
		if(ch2 < 0xA0 || ch2 == 0xFF )
		continue;
		if((ch1> 0xA0) && (ch1 < 0xAA)) //01¡«09ÇøÎª·ûºÅÊý×Ö
		locate = ((ch1 - 0xA1)*94 + (ch2 - 0xA1))*2;
		else //if((buf > 0xAF) && (buf < 0xF8)){ //16¡«87ÇøÎªºº×Ö
		locate = ((ch1 - 0xB0 + 9)*94 + (ch2 - 0xA1))*2;
		(ptr+i-1)[0] = GtoB[locate++];
		(ptr+i-1)[1] = GtoB[locate];
	}
	return write(port, str, len);
}

int read2(int port, char *str, int len) // read big from gb 
{
	/*
	 * Big-5 ÊÇÒ»¸öË«×Ö½Ú±àÂë·½°¸£¬ÆäµÚÒ»×Ö½ÚµÄÖµÔÚ 16 ½ø
	 * ÖÆµÄ A0¡«FE Ö®¼ä£¬µÚ¶þ×Ö½ÚÔÚ 40¡«7E ºÍ A1¡«FE Ö®¼ä¡£
	 * Òò´Ë£¬ÆäµÚÒ»×Ö½ÚµÄ×î¸ßÎ»ÊÇ 1£¬µÚ¶þ×Ö½ÚµÄ×î¸ßÎ»Ôò¿É
	 * ÄÜÊÇ 1£¬Ò²¿ÉÄÜÊÇ 0¡£
	 */
	register unsigned char ch1,ch2, *ptr;
	register int locate, i=0;
	if(len == 0) return 0;
	len = read(port, str, len);
	if( len < 1)
	return len;

	for(i=0,ptr = str; i < len; i++) {
		ch1 = (ptr+i)[0];
		if(ch1 < 0xA1 || ch1 == 0xFF)
		continue;
		ch2 = (ptr+i)[1];
		i ++;
		if(ch2 < 0x40 || ( ch2> 0x7E && ch2 < 0xA1 ) || ch2 == 255)
		continue;
		if( (ch2 >= 0x40) && (ch2 <= 0x7E) )
		locate = ((ch1 - 0xA1) * 157 + (ch2 - 0x40)) * 2;
		else
		locate = ((ch1 - 0xA1) * 157 + (ch2 - 0xA1) + 63) * 2;
		(ptr+i-1)[0] = BtoG[ locate++ ];
		(ptr+i-1)[1] = BtoG[ locate ];
	}
	return len;
}
#endif

//	³¬Ê±´¦Àíº¯Êý,½«·ÇÌØÈ¨ID³¬Ê±Ê±Ìß³öbbs
void hit_alarm_clock() {
	if (HAS_PERM(PERM_NOTIMEOUT))
		return;
	if (i_mode == INPUT_IDLE) {
		clear();
		prints("Idle timeout exceeded! Booting...\n");
		bbskill(getpid(), SIGHUP);
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

//Ë¢ÐÂÊä³ö»º³åÇø
void oflush()
{
	register int size;
	if (size = obufsize) {
#ifdef ALLOWSWITCHCODE
		if(convcode) write2(0, outbuf, size);
		else
#endif
		write(0, outbuf, size);
		obufsize = 0;
	}
}

//	°Ñ³¤¶ÈÎªlenµÄ×Ö·û´®s·ÅÈë»º³åÇøÖÐ,Èô»º³åÇø´æ·Å¹ý¶à×Ö½ÚÊ±,ÔòË¢ÐÂ»º³åÇø
//		1) ÈôÔÊÐíGBÓëBIG5×ª»»,ÇÒÓÃ»§Ê¹ÓÃµÄÊÇBIG5Âë,Ê¹ÓÃwrite2º¯ÊýÊä³ö
//		2)	·ñÔò,Ê¹ÓÃ writeº¯ÊýÖ±½ÓÊä³ö
//		3)	½«ÐÂ¼ÓµÄ×Ö·û´®·Åµ½»º³åÇøÖÐ
//	ÆäÖÐ0ÊÇÎÄ¼þÃèÊö·û,ÒÑ¾­±»Ó³Éäµ½socket¹ÜµÀµÄÊä³ö
void output(char *s,int len)
{
	/* Invalid if len >= OBUFSIZE */

	register int size;
	register char *data;
	size = obufsize;
	data = outbuf;
	if (size + len> OBUFSIZE) {
#ifdef ALLOWSWITCHCODE
		if(convcode)
		write2(0, data, size);
		else
#endif
		write(0, data, size);
		size = len;
	} else {
		data += size;
		size += len;
	}
	memcpy(data, s, len);
	obufsize = size;
}

// Êä³öÒ»¸ö×Ö·û?
void ochar(register int c)
{
	register char *data;
	register int size;
	data = outbuf;
	size = obufsize;

	if (size> OBUFSIZE - 2) { /* doin a oflush */
#ifdef ALLOWSWITCHCODE
		if(convcode) write2(0, data, size);
		else
#endif
		write(0, data, size);
		size = 0;
	}
	data[size++] = c;
	if (c == IAC) data[size++] = c;

	obufsize = size;
}

int i_newfd = 0;
struct timeval i_to, *i_top = NULL;
int (*flushf)() = NULL;

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

/*
 int
 num_in_buf()
 {
 return icurrchar - ibufsize;
 }
 */
//	·µ»Ø»º³åÖÐµÄ×Ö·ûÊý
int num_in_buf() {
	int n;
	if ((n = icurrchar - ibufsize) < 0)
		n=0;
	return n;
}

static int iac_count(char *current)
{
	switch (*(current + 1) & 0xff) {
		case DO:
		case DONT:
		case WILL:
		case WONT:
		return 3;
		case SB: /* loop forever looking for the SE */
		{
			register char *look = current + 2;
			for (;;) {
				if ((*look++ & 0xff) == IAC) {
					if ((*look++ & 0xff) == SE) {
						return look - current;
					}
				}
			}
		}
		default:
		return 1;
	}
}

int igetch()
{
	static int trailing = 0;
	//modified by iamfat 2002.08.21
	//static int repeats = 0;
	//static time_t timestart=0;
	//static int repeatch=0;
	//modified end
	register int ch;
	register char *data;
	data = inbuf;

	for (;;) {
		if (ibufsize == icurrchar) {
			fd_set readfds;
			struct timeval to;
			register fd_set *rx;
			register int fd, nfds;
			rx = &readfds;
			fd = i_newfd;

			igetnext:

			uinfo.idle_time = time(0);
			update_ulist(&uinfo, utmpent);

			FD_ZERO(rx);
			FD_SET(0, rx);
			if (fd) {
				FD_SET(fd, rx);
				nfds = fd + 1;
			} else
			nfds = 1;
			to.tv_sec = to.tv_usec = 0;
			if ((ch = select(nfds, rx, NULL, NULL, &to)) <= 0) {
				if (flushf)
				(*flushf) ();

				if (big_picture)
				refresh();
				else
				oflush();

				FD_ZERO(rx);
				FD_SET(0, rx);
				if (fd)
				FD_SET(fd, rx);

				while ((ch = select(nfds, rx, NULL, NULL, i_top)) < 0) {
					if (errno != EINTR)
					return -1;
				}
				if (ch == 0)
				return I_TIMEOUT;
			}
			if (fd && FD_ISSET(fd, rx))
			return I_OTHERDATA;

			for (;;) {
#ifdef ALLOWSWITCHCODE
				if( convcode ) ch = read2(0, data, IBUFSIZE);
				else
#endif
				ch = read(0, data, IBUFSIZE);

				if (ch> 0)
				break;
				if ((ch < 0) && (errno == EINTR))
				continue;
				//longjmp(byebye, -1);
				abort_bbs(0);
			}
			icurrchar = (*data & 0xff) == IAC ? iac_count(data) : 0;
			if (icurrchar >= ch)
			goto igetnext;
			ibufsize = ch;
			i_mode = INPUT_ACTIVE;
		}
		ch = data[icurrchar++];
		if (trailing) {
			trailing = 0;
			if (ch == 0 || ch == 0x0a)
			continue;
		}
		if (ch == Ctrl('L'))
		{
			redoscr();
			continue;
		}
		if (ch == 0x0d) {
			trailing = 1;
			ch = '\n';
		}
		return (ch);
	}
}

int igetkey() {
	int mode;
	int ch, last;
	extern int RMSG;
	mode = last = 0;
	while (1) {
		if ((uinfo.in_chat == YEA || uinfo.mode == TALK || uinfo.mode
				== PAGE || uinfo.mode == FIVE) && RMSG == YEA) {
			char a;
#ifdef ALLOWSWITCHCODE
			if(convcode) read2(0, &a, 1);
			else
#endif
			read(0, &a, 1);
			ch = (int) a;
		} else
			ch = igetch();
		if ((ch == Ctrl('Z')) && (RMSG == NA) && uinfo.mode != LOCKSCREEN) {
			r_msg2();
			return 0;
		}
		if (mode == 0) {
			if (ch == KEY_ESC)
				mode = 1;
			else
				return ch; /* Normal Key */
		} else if (mode == 1) { /* Escape sequence */
			if (ch == '[' || ch == 'O')
				mode = 2;
			else if (ch == '1' || ch == '4')
				mode = 3;
			else {
				KEY_ESC_arg = ch;
				return KEY_ESC;
			}
		} else if (mode == 2) { /* Cursor key */
			if (ch >= 'A' && ch <= 'D')
				return KEY_UP + (ch - 'A');
			else if (ch >= '1' && ch <= '6')
				mode = 3;
			else
				return ch;
		} else if (mode == 3) { /* Ins Del Home End PgUp PgDn */
			if (ch == '~')
				return KEY_HOME + (last - '1');
			else
				return ch;
		}
		last = ch;
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
		prints(ANSI_RESET);
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

int getdata(int line, int col, char *prompt, char * buf, int len,
		int echo, int clearlabel) {
	int ch, clen = 0, curr = 0, x, y;
	int currDEC=0, i, patch=0;
	char tmp[STRLEN];
	extern unsigned char scr_cols;
	extern int RMSG;
	extern int msg_num;
	if (clearlabel == YEA) {
		buf[0]='\0';
	}
	move(line, col);
	if (prompt)
		prints("%s", prompt);
	y = line;
	col += (prompt == NULL) ? 0 : strlen(prompt);
	x = col;
	clen = strlen(buf);
	curr = (clen >= len) ? len - 1 : clen;
	buf[curr] = '\0';
	prints("%s", buf);

	if (dumb_term || echo == NA) {
		while ((ch = igetkey()) != '\r') {
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

	char buf[255], fname[STRLEN], *ptr;
	time_t now;
	FILE *fp;
	int i, cur_sec, allstay, foo, foo2;

	move(t_lines - 1, 0);
	clrtoeol();

	if (!DEFINE(DEF_ENDLINE))
		return;

	now = time(NULL);
	cur_sec = getdatestring(now, NA); //cur_sec = tm_sec % 10
	if (cur_sec == 0) {
		nowishfile: resolve_boards();
		strlcpy(datestring, brdshm->date, 30);
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
			datestring, get_online(), count_friends,
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

