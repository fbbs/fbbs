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

int num_ans_chr(const char *str);
int seekthestr(const char *str, int num);

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

int usercomplete(char *prompt, char *data);
int namecomplete(char *prompt, char *data);
int t_query(const char *user);
int vedit(char *filename, int write_header_to_file, int modifyheader);

// Following function declarations are put here solely to eliminate warnings
int Postfile(char *filename, char *nboard, char *posttitle, int mode);
char *setuserfile(char *buf, char *filename);
void securityreport(char *str, int save, int mode);
int set_safe_record(void);
void abort_bbs(int nothing);
int bm_log(const char *user, const char *name, int type, int value);
void stand_title(char *title);
int autoreport(char *title,char *str,int toboard,char *userid,int mode);
int	check_systempasswd(void);
int deltree(const char *dst);
int gettheuserid(int x, char *title, int *id);
void i_read(int cmdmode, char *direct, int (*dotitle) (), char *(*doentry) (), struct one_key *rcmdlist, int ssize);
void list_text(const char *file,
		void (*title_show)(void),
		int (*key_deal)(const char *, int, char *),
		int (*check)(const char *));
int m_new(void);
int set_ann_path(char *title, char *path, int mode);
struct MENU;
int a_a_Import(struct MENU *pm, int msg, int menuitem);
int add_grp(char group[STRLEN], char gname[STRLEN], char bname[STRLEN],
		char title[STRLEN]);
int a_Import(char *path, char* key, int ent, struct fileheader *fileinfo,
		char * direct, int nomsg);
int Goodbye(void);
char *sethomepath(char *buf, const char *userid);
int show_online_followings(void);
int show_online_users(void);
int show_users_in_board(void);
int fill_shmfile(int mode, char* fname, char * shmkey);
void showstuff(char *buf);
int vote_flag(char *bname, char val, int mode);
void check_register_info(void);
void check_title(char *title);
void CreateNameList(void);
int digest_post(int ent, struct fileheader *fhdr, char *direct);
int do_reply(struct fileheader *fh);
int fill_date(void);
int is_birth(const struct userec *user);
void keep_fail_post(void);
int locate_the_post(struct fileheader *fileinfo, char *query, int offset, int aflag, int newflag);
int mail_del(int ent, struct fileheader *fileinfo, char *direct);
int m_send(const char *userid);
void new_register(void);
int post_header(struct postheader *header);
int post_reply(int ent, struct fileheader *fileinfo, char *direct);
void Poststring(char *str, char *nboard, char *posttitle, int mode);
char *setbdir(char *buf, char *boardname);
int domenu(const char *menu_name);
void setqtitle(char *stitle, int gid);
void setquotefile(const char *filepath);
void setvfile(char *buf, const char *bname, const char *filename);
void show_issue(void);
void show_message(const char *msg);
void shownotepad(void);
int show_statshm(char* fh, int mode);
int s_msg(void);
int sread(int readfirst, int auser, struct fileheader *ptitle);
void disply_userinfo(const struct userec *u);
void a_menu(char *maintitle, char* path, int lastlevel, int lastbmonly);
int a_menusearch(char *path, char* key, char * found);
int a_repair(struct MENU *pm);
int a_Save(char *path, char* key, struct fileheader *fileinfo, int nomsg,
		int full);
int board_read(void);
int catnotepad(FILE *fp, const char *fname);
int clear_ann_path(void);
int countlogouts(char *filename);
int del_grp(char grp[STRLEN], char bname[STRLEN], char title[STRLEN]);
void uinfo_query(struct userec *u, int real, int unum);
int edit_grp(char bname[STRLEN], char grp[STRLEN], char title[STRLEN],
		char newtitle[STRLEN]);
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) ());
int show_file_info(int ent, struct fileheader *fileinfo, char *direct);
int chk_currBM(char *BMstr, int isclub);
bool garbage_line(const char *str);
int Q_Goodbye(void);
int _UndeleteArticle(int ent, struct fileheader *fileinfo, char *direct,
		int response);
int acction_mode(int ent, struct fileheader *fileinfo, char *direct);
void Add_Combine(char *board, struct fileheader *fileinfo, int has_cite);
void add_crossinfo(char *filepath, int mode);
int AddNameList(const char *name);
int marked_all(int type);
int post_article(char *postboard, char *mailid);
void write_header(FILE *fp, int mode);
int outgo_post(struct fileheader *fh, char *board);
void get_noticedirect(char *curr, char *notice);
void fixkeep(char *s, int first, int last);
int Personal(const char *userid);
void user_display(char *filename, int number, int mode);
void show_goodbyeshm(void);
void u_exit(void);
struct keeploc * getkeep(char *s, int def_topline, int def_cursline);
void do_quote(const char *orig, const char *file, char mode);
int del_range(int ent, struct fileheader *fileinfo, char *direct);
int x_cloak(void);
int show_online(void);
void do_report(const char *filename, const char *s);
void set_numofsig(void);
int _del_post(int ent, struct fileheader *fileinfo, char *direct,
		int subflag, int hasjudge);
int mark_post(int ent, struct fileheader *fileinfo, char *direct);
int do_post(void);
int makeDELETEDflag(int ent, struct fileheader *fileinfo, char *direct);
int underline_post(int ent, struct fileheader *fileinfo, char *direct);
int _combine_thread(int ent, struct fileheader *fileinfo, char *direct, int gid);
int del_post(int ent, struct fileheader *fileinfo, char *direct);
int post_cross(char islocal, int mode);
int check_notespasswd(void);

#endif // FB_TERMINAL_H
