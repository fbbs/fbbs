#ifndef FB_TERMINAL_H
#define FB_TERMINAL_H

// src/io.c
extern int read_stdin(unsigned char *buf, size_t size);
extern int write_stdout(const unsigned char *buf, size_t len);
extern int oflush(void);
extern void ochar(int ch);
extern void output(const unsigned char *str, int size);
extern void add_io(int fd, int timeout);
extern bool inbuf_empty(void);
extern int igetch(void);
extern int igetkey(void);
extern int egetch(void);
extern int ask(const char *prompt);
extern int getdata(int line, int col, const char *prompt, char *buf, int len,
		int echo, int clearlabel);
extern void update_endline(void);
extern void showtitle(const char *title, const char *mid);
extern void firsttitle(const char *title);
extern void docmdtitle(const char *title, const char *prompt);
extern int show_data(const char *buf, int maxcol, int line, int col);
extern int multi_getdata(int line, int col, int maxcol, const char *prompt,
		char *buf, int len, int maxline, int clearlabel, int textmode);

// src/screen.c
extern void initscr(void);
extern void redoscr(void);
extern void refresh(void);
extern void move(int line, int col);
extern void getyx(int *line, int *col);
extern void clear(void);
extern void clear_whole_line(int line);
extern void clrtoeol(void);
extern void clrtobot(void);
extern int outc(int c);
extern void outs(const char *str);
extern void prints(const char *fmt, ...);
extern void scroll(void);
extern void standout(void);
extern void standend(void);
extern void saveline(int line, int mode);
extern void saveline_buf(int line, int mode);

extern void presskeyfor(const char *msg, int line);
extern void pressanykey(void);
extern int pressreturn(void);
extern bool askyn(const char *str, bool defa, bool gobottom);
extern void printdash(const char *mesg);
extern void bell(void);

// src/more.c

void ActiveBoard_Init(void);
void setcalltime(void);
int countln(char *fname);
void netty_more(void);
void printacbar(void);
int check_calltime(void);
void R_monitor(void);
int msg_more(void);
int ansimore4(char *filename, int promptend, char *board, char *path, int ent);
int ansimore(char *filename, int promptend);
int ansimore2(char *filename, int promptend, int row, int numlines);
void show_help(const char *fname);
int mainreadhelp(void);
int mailreadhelp(void);

#endif // FB_TERMINAL_H
