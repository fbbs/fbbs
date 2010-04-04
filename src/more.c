#include "bbs.h"

static time_t calltime = 0;
void R_monitor();
//Added by Ashinmarch to support multi-line msg
extern void show_data(char *buf, int maxcol, int line, int col);

struct ACSHM {
	char    data[ACBOARD_MAXLINE][ACBOARD_BUFSIZE];
	int     movielines;
	int     movieitems;
	time_t  update;
};

static struct ACSHM *movieshm = NULL;
static int nnline = 0;
char    more_buf[MORE_BUFSIZE];
int     more_size, more_num;

/*Added by Ashinmarch on 2007.12.01*/
extern int RMSG;

void ActiveBoard_Init( void )
{
	struct fileheader fh;
	FILE   *fp;
	char   *ptr;
	char    buf[1024], buf2[1024];
	struct stat st;
	int     max = 0, i = 0, j = 0, x, y = 0;
	int     flag; /* flag = 1 ¼´Îª¹ýÂÇµô "--\n" ÒÔááÖ®ÈÎºÎÄÚÈÝ */ 

	if( movieshm == NULL )
		movieshm = (void *) attach_shm("ACBOARD_SHMKEY", 4123, sizeof(*movieshm));
	if (movieshm == NULL)
		exit(1);

	if (stat("boards/Notepad/.DIGEST", &st) < 0) {
		empty_movie(1);
		return ;
	}
	if (movieshm->update > st.st_mtime) return ; 

	for (i = 0; i < ACBOARD_MAXLINE; i++)
		movieshm->data[i][0] = 0;

	max = get_num_records("boards/Notepad/.DIGEST", sizeof(fh)); 

	i = 1;
	j = 0;
	while (i <= max && j < ACBOARD_MAXLINE) {
		get_record("boards/Notepad/.DIGEST", &fh, sizeof(fh), i++);
		sprintf(buf, "boards/Notepad/%s", fh.filename);

		fp = fopen(buf, "r"); 
		if (fp == NULL) continue; 

		y++;		/* record how many files have been append */ 

		if (fh.title[0] == '$') flag = (int) (fh.title[1] - '0');
		else flag = 4;
		for (x = 0; x < flag; x++)  // Ìø¹ýÍ·²¿ÐÅÏ¢
			fgets(buf, 1024, fp); 

		flag = 0;
		for (x = 0; x < MAXMOVIE - 1 && j < ACBOARD_MAXLINE; x++) {
			if (fgets(buf, 1024, fp) != 0) {
				buf[ACBOARD_BUFSIZE-4] = '\0';
				if (flag == 1 || strcmp(buf, "--\n") == 0) {
					strcpy(buf2, "[K");
					flag = 1;
				}
				ptr = movieshm->data[j]; 
				if (flag == 0) {
					strcpy(buf2, "[K");
					strcat(buf2, buf); 
				}
				buf2[ACBOARD_BUFSIZE-1] = '\0';
				memcpy(ptr, buf2, ACBOARD_BUFSIZE);
			} else { /* no data handling */
				strcpy(movieshm->data[j], "[K");
			}
			j++;
		}
		fclose(fp);
	} 
	if (j == 0) {
		empty_movie(3);
		return ;
	}
	movieshm->movielines = j;
	movieshm->movieitems = y;
	movieshm->update = time(0); 

	sprintf(buf, "»î¶¯¿´°å¸üÐÂ, ¹² %d ÐÐ, %d ²¿·Ý.", j, y);
	report(buf, currentuser.userid); 
	return ;
}

int empty_movie(int x)
{
	sprintf(genbuf, "Empty Movie!!! (error = %d)", x);
	report(genbuf, currentuser.userid); 

	strcpy(movieshm->data[2], "[K      ** ÉÐÎ´Éè¶¨»î¶¯¿´°å ** ");
	strcpy(movieshm->data[3], "[K         ÇëÏê¼û°²×°ËµÃ÷Êé Firebird-2000 ");
	strcpy(movieshm->data[4], "[K         Éè¶¨ notepad °æ"); 

	movieshm->movielines = MAXMOVIE;
	movieshm->movieitems = 1;
	movieshm->update = time(0);

}

void setcalltime( void )
{
	char    ans[6];
	int     ttt;
	move(1, 0);
	clrtoeol();
	getdata(1, 0, "¼¸·ÖÖÓºóÒªÏµÍ³ÌáÐÑÄú: ", ans, 3, DOECHO, YEA);
	ttt = atoi(ans);
	if (ttt <= 0) return;
	calltime = time(0) + ttt * 60;
}

int readln(int fd, char *buf)
{
	int     len, bytes, in_esc, ch;
	len = bytes = in_esc = 0;
	while (1) {
		if (more_num >= more_size) {
			more_size = read(fd, more_buf, MORE_BUFSIZE);
			if (more_size == 0)  break;
			more_num = 0;
		}
		ch = more_buf[more_num++];
		bytes++;
		if (ch == '\n' || bytes > 255) break;
		else if (ch == '\t') {
			do {
				len++, *buf++ = ' ';
			} while ((len % 4) != 0);
		} else if (ch == '') {
			if (showansi) *buf++ = ch;
			in_esc = 1;
		} else if (in_esc) {
			if (showansi) *buf++ = ch;
			if (strchr("[0123456789;,", ch) == NULL) in_esc = 0;
		} else if (isprint2(ch)) {
			if (len > 79) break;
			len++;
			*buf++ = ch;
		}
	}
	*buf++ = ch;
	*buf = '\0';
	return bytes;
}

int morekey( void )
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

int seek_nth_line(int fd, int no)  // ´ÓÎÄ¼þÍ·¶Áµ½µÚ no ÐÐ( ¶¨Î»µ½ no ÐÐ )
{
	int     n_read, line_count, viewed;
	char   *p, *end;
	lseek(fd, 0, SEEK_SET);
	line_count = viewed = 0;

	if (no > 0)
		while (1) {
			n_read = read(fd, more_buf, MORE_BUFSIZE);
			if(n_read<=0)break;
			p = more_buf;
			end = p + n_read;
			for (; p < end && line_count < no; p++)
				if (*p == '\n')line_count++;

			if (line_count >= no) {
				viewed += (p - more_buf);
				lseek(fd, (off_t) viewed, SEEK_SET);
				break;
			} else
				viewed += n_read;
		} 

	more_num = MORE_BUFSIZE + 1;	/* invalidate the readln()'s buffer */

	return viewed;
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

/* below added by netty  *//* Rewrite by SmallPig */
void netty_more(void)
{
	char    buf[256];
	int     ne_row = 1;
	int     x, y;
	time_t  thetime = time(0);

	getyx(&y, &x);
	update_endline();
	if (!DEFINE(DEF_ACBOARD)) return;
	nnline = (thetime / 10 % movieshm->movieitems) * (MAXMOVIE - 1);

	while ((nnline < movieshm->movielines)) {
#ifdef BIGGER_MOVIE
		move(1 + ne_row, 0);
#else
		move(2 + ne_row, 0);
#endif
		clrtoeol();
		strcpy(buf, movieshm->data[nnline]);
		showstuff(buf);
		nnline = nnline + 1;
		ne_row = ne_row + 1;
		if (nnline == movieshm->movielines) {
			nnline = 0;
			break;
		}
		if (ne_row > MAXMOVIE - 1) {
			break;
		}
	}
	move(y, x);
}

void printacbar(void)
{
#ifndef BIGGER_MOVIE
	struct boardheader *bp;
	int x,y;

	getyx(&y,&x);
	bp = getbcache(DEFAULTBOARD);
	move(2,0);
	if(bp->flag&VOTE_FLAG) prints(" [1;36m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[37mÏµÍ³Í¶Æ±ÖÐ [ Config->Vote ] [36m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´ [m\n");
	else prints(" [1;36m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[37m»î  ¶¯  ¿´  °å[36m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´ [m\n");
	move(2+MAXMOVIE,0);
	prints(" [1;36m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m\n");
	move (y,x);
#endif
	refresh();
}

int check_calltime(void)
{
	int     line;
	if ( calltime != 0 && time(0) >= calltime ) {
		if (uinfo.mode == TALK)
			line = t_lines / 2 - 1;
		else
			line = 0;

		saveline(line, 0);	/* restore line */
		bell();
		bell();
		bell();
		move(line, 0);
		clrtoeol();
		prints("[1;44;32mÏµÍ³Í¨¸æ: [37m%-65s[m", "ÏµÍ³ÄÖÖÓ Áå¡«¡«¡«¡«¡«¡«");
		igetkey();
		move(line, 0);
		clrtoeol();
		saveline(line, 1);
		calltime = 0;
	}
	return 0;
}

void R_monitor()
{
	if (uinfo.mode != MMENU)
		return;

	/* Added by Ashinmarch on 2007.12.01
	 * used to support multi-line msgs
	 */
	if (uinfo.mode == LOOKMSGS || uinfo.mode == MSG || RMSG == YEA)
		return;
	/*end*/
	if (!DEFINE(DEF_ACBOARD) && !DEFINE(DEF_ENDLINE))
		return;

	alarm(0);
	signal(SIGALRM, R_monitor);
	netty_more();
	printacbar();
	if (!DEFINE(DEF_ACBOARD))
		alarm(55);
	else
		alarm(10);
}

enum {
	/** A ::linenum_t will be assigned for every block. */
	LINENUM_BLOCK_SIZE = 4096, 
	DEFAULT_TERM_WIDTH = 80,   ///< Default terminal width.
	TAB_STOP = 4,     ///< Columns of a tab stop.
	IS_QUOTE = 0x1,   ///< The line is part of quotation if this bit is set.
	HAS_TABSTOP = 0x2 ///< The line has tab stop(s) if this bit is set.
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
} more_file_t;

typedef int (*more_open_t)(const char *, more_file_t *);

typedef int (*more_prompt_t)(more_file_t *);

typedef int (*more_handler_t)(more_file_t *, int);

/**
 * Open a file as more stream.
 * @param file File to open.
 * @param width Line width.
 * @param func A function to fill the stream with file content.
 * @return an ::more_file_t pointer on success, NULL on error.
 */
static more_file_t *more_open(const char *file, int width, more_open_t func)
{
	more_file_t *more = malloc(sizeof(*more));
	if (more == NULL)
		return NULL;

	memset(more, 0, sizeof(*more));
	more->total = -1;
	more->width = width;

	if ((*func)(file, more) != 0) {
		free(more);
		return NULL;
	}
	return more;
}

/**
 * Close more stream.
 * @param d The more stream.
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
	static const char code[] = "[0123456789;";
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
			if (!memchr(code, *ptr, sizeof(code) - 1))
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
			while (++ptr < end && memchr(code, *ptr, sizeof(code) - 1))
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

/**
 *
 */
static int more_open_file(const char *file, more_file_t *more)
{
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return -1;

	more->size = m.size;
	more->ln_size = m.size / LINENUM_BLOCK_SIZE + 1; // at least one block
	more->buf = malloc(m.size + more->ln_size * sizeof(linenum_t));
	if (more->buf != NULL) {
		memcpy(more->buf, m.ptr, m.size);
		mmap_close(&m);
		more->ln = (linenum_t *)(more->buf + m.size);
		memset(more->ln, 0, more->ln_size * sizeof(linenum_t));
		return 0;
	}

	mmap_close(&m);
	return -1;
}

static int more_prompt_file(more_file_t *more)
{
	prints("\033[0;1;44;32mÏÂÃæ»¹ÓÐà¸(%d%%) µÚ(%d-%d)ÐÐ \033[33m|"
			" l ÉÏÆª | b e ¿ªÍ·Ä©Î² | g Ìø×ª | h °ïÖú\033[K\033[m",
			(more->end - more->buf) * 100 / more->size,
			more->line - t_lines + 2, more->line);
	return 0;
}

static int is_emphasize(const char *str)
{
	return (!strncmp(str, "¡¾ ÔÚ", sizeof("¡¾ ÔÚ"))
			|| !strncmp(str, "==>", 3)
			|| !strncmp(str, "¡õ ÒýÓÃ", sizeof("¡õ ÒýÓÃ"))
			|| !strncmp(str, "¡ù ÒýÊö", sizeof("¡ù ÒýÊö")));
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

	clrtobot();
	// TODO: stuffmode
	while (true) {
		is_quote = more->prop & IS_QUOTE;
		while (i++ < t_lines - 1) {
			// Get a line
			if (more_getline(more) <= 0) {
				if (prompt)
					pressanykey();
				return ch;
			}
			lines_read++;
			if (lines == 0 || lines_read <= lines) {
				if (is_emphasize(more->begin)) {
					outs("\033[1;33m");
					more_puts(more);
					colored = true;
				} else {
					is_wrapped = (more->begin != more->buf)
							&& (*(more->begin - 1) != '\n');
					if (is_quote || (!is_wrapped && is_quotation(more->begin))) {
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
				if (++pos == t_lines) {
					scroll();
					--pos;
				}
			} 			else {
				if (promptend)
					pressanykey();
				refresh();
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
		move(t_lines - 1, 0);
		clrtoeol();
		(*prompt)(more);

		ch = morekey();
		move(t_lines - 1, 0);
		clrtoeol();
		refresh();
		switch (ch) {
			case KEY_LEFT:
				return ch;
				break;
			case KEY_RIGHT:
				i = 1;
				break;
			case KEY_DOWN:
				i = t_lines - 2;
				break;
			case KEY_PGUP:
				clear();
				i = pos = 0;
				new_row = more->line - (2 * t_lines - 3);
				if (new_row < 0)
					return ch;
				more_seek(more, new_row);
				break;
			case KEY_UP:
				clear();
				i = pos = 0;
				new_row = more->line - t_lines;
				if (new_row < 0) {
					return ch;
				}
				more_seek(more, new_row);
				break;
			case KEY_HOME:
				clear();
				i = pos = 0;
				more_seek(more, 0);
				break;
			case 'R':
			case KEY_END:
				more_countline(more);
				i = t_lines - 1 - (more->total - more->line);
				if (i < 0)
					i = 0;
				if (i == t_lines - 1)
					break;
				more_seek(more, more->total - (t_lines - 1) + i);
				break;
			case 'G':
				getdata(t_lines - 1, 0, "Ìø×ªµ½µÄÐÐºÅ:", linebuf,
						sizeof(linebuf), true, true);
				new_row = strtol(linebuf, NULL, 10) - 1;
				if (new_row < 0)
					new_row = 0;
				more_seek(more, new_row);
				if (more->total >= 0 && new_row >= more->total)
					more_seek(more, more->total - 1);
				clear();
				i = pos = 0;
				break;
			case 'H':
				show_help("help/morehelp");
				i = pos = 0;
				new_row = more->line - t_lines + 1;
				if (new_row < 0)
					new_row = 0;
				more_seek(more, new_row);
				break;
			case 'L':
				return KEY_PGUP;
				break;
			default:
				if (handler)
					(*handler)(more, ch);
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
	more_file_t *more = more_open(file, DEFAULT_TERM_WIDTH, more_open_file);
	if (more == NULL)
		return -1;
	int ch = more_main(more, promptend, line, numlines, stuffmode,
			more_prompt_file, NULL);
	more_close(more);
	return ch;
}

int mesgmore(char *filename, int promptend, int row, int numlines)
{
	extern int t_lines;
	struct stat st;
	int     fd, tsize;
	char    buf[256];
	int     i, ch, viewed, pos, isin = NA;
	int     numbytes;
	int     curr_row = row;
	int     linesread = 0;
	char    title[256];

	if ((fd = open(filename, O_RDONLY)) == -1)  return -1;
	if (fstat(fd, &st)) return -1;

	tsize = st.st_size;
	more_size = more_num = 0; 

	clrtobot();
	i = pos = viewed = 0;
	numbytes = readln(fd, buf);
	curr_row++;
	linesread++;
	while (numbytes) {
		if (linesread <= numlines || numlines == 0) {
			viewed += numbytes; 
			if (check_stuffmode())
				showstuff(buf);
			else {
				/* Modified by Ashinmarch on 2007.12.01, used to support multi-line msg
				 * msghead(with ansi) and msg content(no ansi but multi-line) should be
				 * differentiated.
				 */
				if(buf[0] == '\033') //msg head
					prints("%s",buf);
				else   //msg
					show_data(buf, LINE_LEN - 1, curr_row, 0);
			}
			isin = YEA;
			i++;
			pos++;
			if (pos == t_lines) {
				scroll();
				pos--;
			}
			numbytes = readln(fd, buf);
			curr_row++;
			linesread++;
			if (numbytes == 0) break;
			if (i == t_lines - 1) {
				if (showansi) {
					move(t_lines - 1, 0);
					prints("[0m[m");
					refresh();
				}
				move(t_lines - 1, 0);
				clrtoeol();
				prints("[0m[1;44;32m(%d%%) ÊÇ·ñ¼ÌÐø [[1;37mY/n[1;32m]   ±£Áô <[1;37mr[32m>    Çå³ý <[1;37mc[1;32m>   ¼Ä»ØÐÅÏä <[1;37mm[1;32m>                      [m", (viewed * 100) / tsize);
				ch = morekey();
				move(t_lines - 1, 0);
				clrtoeol();
				refresh();
				if (ch == KEY_LEFT) {
					close(fd);
					return ch;
				} else if (ch == KEY_RIGHT) {
					i = 1;
				} else if (ch == KEY_DOWN) {
					i = t_lines - 2;
				} else if (ch == KEY_PGUP || ch == KEY_UP) {
					clear();
					i = pos = 0;
					curr_row -= (ch == KEY_PGUP)?(2*t_lines - 2) : (t_lines + 1);
					if (curr_row < 0) {
						close(fd);
						return ch;
					}
					viewed = seek_nth_line(fd, curr_row);
					numbytes = readln(fd, buf);
					curr_row++;
				} else if (ch == 'H') {
					show_help("help/msghelp");
					i = pos = 0;
					curr_row -= (t_lines);
					if (curr_row < 0)
						curr_row = 0;
					viewed = seek_nth_line(fd, curr_row);
					numbytes = readln(fd, buf);
					curr_row++;
				} else if (ch == 'C') {
					if (askyn("È·¶¨ÒªÇå³ýÂð£¿", NA, YEA) == YEA) {
						close(fd);
						unlink(filename);
					}
					return ch;
				} else if (ch == 'M') {
					if (askyn("È·¶¨Òª¼Ä»ØÂð£¿", NA, YEA) == YEA) {
						close(fd);
						sprintf(title, "[%s] ËùÓÐÑ¶Ï¢±¸·Ý", getdatestring(time(NULL), DATE_ZH));
						mail_file(filename, currentuser.userid, title);
						unlink(filename);
					}
					return ch;
				} else if (ch == 'N' || ch == 'R') {
					close(fd);
					return ch;
				} else if (ch == 'Y') {
					i = 1;
				}
			}
		} else break;	/* More Than Want */
	} 

	close(fd); 
	if (promptend == YEA || pos < t_lines)
		msgmorebar(filename);

	return ch;
}

int ansimore4(char *filename, int promptend, char *board, char *path, int ent)
{
	clear();
	return rawmore2(filename, promptend, 0, 0, NA);
}

int ansimore(char *filename, int promptend)
{
	int ch;
	clear();
	ch = rawmore2(filename, promptend, 0, 0, NA);
	move(t_lines - 1, 0);
	prints("\033[0m\033[m");
	refresh();
	return ch;
}

int ansimore2(char *filename, int promptend, int row, int numlines)
{
	int     ch;
	ch = rawmore2(filename, promptend, row, numlines, NA);
	refresh();
	return ch;
}
/* edwardc.990624 ÏÈÔÝÊ±ÓÃ ansimore3() ´úÌæ ... */

int ansimore3(char *filename, int promptend)
{
	int     ch;
	clear();
	ch = rawmore2(filename, promptend, 0, 0, YEA);
	move(t_lines - 1, 0);
	prints("[0m[m");
	refresh();
	return ch;
}

// deardragon 2000.08.28  over
