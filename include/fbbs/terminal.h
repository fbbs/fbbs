#ifndef FB_TERMINAL_H
#define FB_TERMINAL_H

/** ANSI指令，清屏 */
#define ANSI_CMD_CL "\033[H\033[J"

/** ANSI指令，从光标处删除到行尾 */
#define ANSI_CMD_CE "\033[K"

/** ANSI指令，向上滚动一行 */
#define ANSI_CMD_SR "\033M"

/** ANSI指令，进入反色模式 */
#define ANSI_CMD_SO "\033[7m"

/** ANSI指令，退出反色模式 */
#define ANSI_CMD_SE "\033[m"

extern int terminal_read(unsigned char *buf, size_t size);
extern int terminal_write(const unsigned char *buf, size_t len);
extern int terminal_flush(void);
extern void terminal_putchar(int ch);
extern void terminal_write_cached(const unsigned char *str, int size);
extern bool terminal_input_buffer_empty(void);
extern void terminal_schedule_exit(int);

extern int igetch(void);
extern int terminal_getchar(void);
extern int egetch(void);
extern int getdata(int line, int col, const char *prompt, char *buf, int len,
		int echo, int clearlabel);
extern void tui_repeat_char(int c, int repeat);
extern void tui_update_status_line(void);
extern void tui_suppress_notice(bool suppress_notice);
extern int tui_check_notice(const char *board_name);
extern void showtitle(const char *title, const char *mid);
extern void firsttitle(const char *title);
extern void docmdtitle(const char *title, const char *prompt);
extern int show_data(const char *buf, int maxcol, int line, int col);
extern int multi_getdata(int line, int col, int maxcol, const char *prompt,
		char *buf, int len, int maxline, int clearlabel, int textmode);

// src/screen.c
extern int screen_lines(void);
extern void screen_negotiate_size(void);
extern void screen_init(void);
extern void screen_redraw(void);
extern void screen_flush(void);
extern void move(int line, int col);
extern void screen_coordinates(int *line, int *col);
extern void screen_clear(void);
extern void screen_clear_line(int line);
extern void screen_move_clear(int line);
extern void clrtoeol(void);
extern void screen_clrtobot(void);
extern int outc(int c);
extern void outs(const char *str);
extern void prints(const char *fmt, ...);
extern void screen_scroll(void);
extern void screen_save_line(int line, bool save);
extern void saveline_buf(int line, int mode);

extern void presskeyfor(const char *msg, int line);
extern void pressanykey(void);
extern int pressreturn(void);
extern bool askyn(const char *str, bool defa, bool gobottom);
extern void printdash(const char *mesg);
extern void bell(void);

int num_ans_chr(const char *str);

// src/more.c

int countln(char *fname);
int msg_more(void);
int ansimore4(char *filename, int promptend, char *board, char *path, int ent);
int ansimore_buffer(const char *buf, size_t size, int promptend);
int ansimore(const char *filename, int promptend);
int ansimore2(char *filename, int promptend, int row, int numlines);
void show_help(const char *fname);
int mainreadhelp(void);
int mailreadhelp(void);

int usercomplete(char *prompt, char *data);
int namecomplete(char *prompt, char *data);
int t_query(const char *user);
int vedit(char *filename, int write_header_to_file, int modifyheader, struct postheader *header);

#include "fbbs/post.h"
typedef enum {
	POST_FILE_NORMAL = 0,
	POST_FILE_DELIVER = 1,
	POST_FILE_AUTO = 2,
	POST_FILE_BMS = 3,
	POST_FILE_CP_ANN = 4,
} post_file_e;
// Following function declarations are put here solely to eliminate warnings
post_id_t Postfile(const char *file, const char *bname, const char *title, post_file_e mode);
char *setuserfile(char *buf, const char *filename);
void securityreport(char *str, int save, int mode);
int set_safe_record(void);
void abort_bbs(int nothing);
void stand_title(const char *title);
void autoreport(const char *board, const char *title, const char *str, const char *uname, int mode);
int	check_systempasswd(void);
int deltree(const char *dst);
int gettheuserid(int x, char *title, int *id);
void i_read(int cmdmode, const char *direct, int (*dotitle) (), char *(*doentry) (), struct one_key *rcmdlist, int ssize);
void list_text(const char *file,
		void (*title_show)(void),
		int (*key_deal)(const char *, int, const char *),
		int (*check)(const char *));
int m_new(void);
int set_ann_path(const char *title, const char *path, int mode);
struct MENU;
int a_a_Import(struct MENU *pm, int msg, int menuitem);
int add_grp(const char *group, const char *gname,
		const char *bname, const char *title);
int a_Import(const char *title, const char *file, int nomsg);
int Goodbye(void);
char *sethomepath(char *buf, const char *userid);
int show_online_followings(void);
int show_online_users(void);
int show_users_in_board(void);
int fill_shmfile(int mode, char* fname, char * shmkey);
void showstuff(char *buf);
int vote_flag(const char *bname, char val, int mode);
void check_register_info(void);
void check_title(char *title, size_t size);
void CreateNameList(void);
int digest_post(int ent, struct fileheader *fhdr, char *direct);
int do_reply(struct fileheader *fh);
int fill_date(void);
void keep_fail_post(void);
int mail_del(int ent, struct fileheader *fileinfo, char *direct);
int m_send(const char *userid);
void new_register(void);
int post_header(struct postheader *header);
int post_reply(const char *owner, const char *title, const char *file);
void Poststring(const char *str, const char *nboard, const char *posttitle, int mode);
int domenu(const char *menu_name);
void setqtitle(char *stitle, int gid);
void setquotefile(const char *filepath);
void setvfile(char *buf, const char *bname, const char *filename);
void show_issue(void);
void show_message(const char *msg);
int shownotepad(void);
int show_statshm(char* fh, int mode);
int s_msg(void);
void disply_userinfo(const struct userec *u);
void a_menu(char *maintitle, char* path, int lastlevel, int lastbmonly);
int a_menusearch(const char *key, char *found);
int a_repair(struct MENU *pm);
int a_Save(const char *gbk_title, const char *file, int nomsg, int full);
int board_read(void);
int catnotepad(FILE *fp, const char *fname);
int clear_ann_path(void);
int countlogouts(char *filename);
int del_grp(char grp[STRLEN], char bname[STRLEN], char title[STRLEN]);
void uinfo_query(struct userec *u, int real, int unum);
int edit_grp(char bname[STRLEN], char grp[STRLEN], char title[STRLEN],
		char newtitle[STRLEN]);
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) ());
int chk_currBM(char *BMstr, int isclub);
bool garbage_line(const char *str);
int Q_Goodbye(void);
void Add_Combine(const char *board, struct fileheader *fileinfo, int has_cite);
void add_crossinfo(const char *filepath, bool post);
int AddNameList(const char *name);
int post_article(char *postboard, char *mailid);
void write_header(FILE *fp, const struct postheader *header, bool _in_mail);
int outgo_post(struct fileheader *fh, char *board);
void fixkeep(char *s, int first, int last);
int Personal(const char *userid);
void user_display(char *filename, int number, int mode);
void show_goodbyeshm(void);
void u_exit(void);
struct keeploc * getkeep(char *s, int def_topline, int def_cursline);
void do_quote(const char *orig, const char *file, char mode, bool anony);
int del_range(int ent, struct fileheader *fileinfo, char *direct);
int x_cloak(void);
int show_online(void);
void do_report(const char *filename, const char *s);
void set_numofsig(void);
int _del_post(int ent, struct fileheader *fileinfo, char *direct,
		int subflag, int hasjudge);
int mark_post(int ent, struct fileheader *fileinfo, char *direct);
int do_post(void);
int del_post(int ent, struct fileheader *fileinfo, char *direct);
int check_notespasswd(void);

#endif // FB_TERMINAL_H
