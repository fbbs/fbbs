#ifndef _LIBWEB_H_

#define _LIBWEB_H_

#include "../telnet/include/bbs.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "stdarg.h"

#define FIRST_PAGE	"/bbsmain.html"
#define CSS_FILE 	"/css/bbs%d.css"
#define CHARSET		"gb2312"
#define UCACHE_SHMKEY	get_shmkey("UCACHE_SHMKEY")
#define UTMP_SHMKEY	get_shmkey("UTMP_SHMKEY")
#define BCACHE_SHMKEY	get_shmkey("BCACHE_SHMKEY")
#define ACACHE_SHMKEY get_shmkey("ACACHE_SHMKEY")

#define TLINES 18
#define SQUID

//Goodbye.c
#define MAX_PERF (1000)
#define MAX_LUCKY (20)

#ifdef FDQUAN
	#define MAX_LARGEMAIL_UPLIMIT   300
	#define MAX_BOARDSMAIL_UPLIMIT	120
	#define MAX_MAIL_UPLIMIT	60
#else
	#define MAX_LARGEMAIL_UPLIMIT   20000
	#define MAX_BOARDSMAIL_UPLIMIT  20000
	#define MAX_MAIL_UPLIMIT        20000
#endif

#ifdef FDQUAN
	#define SECNUM 9
#else
	#define SECNUM 12
#endif

#define file_size(x) f_stat(x)->st_size
#define file_time(x) f_stat(x)->st_mtime
#define file_rtime(x) f_stat(x)->st_atime
#define file_exist(x) (file_time(x)!=0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)

#define trim(s) ltrim(rtrim(s))
#define const_trim(s) const_ltrim(rtrim(s))

#define setcookie(a, b)	printf("<script>document.cookie='%s=%s'</script>\n", a, b)
#define redirect(x)	printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t)	printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()	printf("Content-type: text/html; charset=%s\n\n", CHARSET)

#define FLOCK(x,y) flock(x,y)

extern char seccode[][6];
extern char secname[][2][20];

struct post_log {
	char	author[IDLEN+1];
	char	board[18];
	char	title[66];
	time_t	date;
	int	number;
};

extern int loginok;

extern struct userec currentuser;
extern struct user_info *u_info;
extern struct UTMPFILE *shm_utmp;
extern struct BCACHE *shm_bcache;
extern struct UCACHE *shm_ucache;
extern struct boardheader *bcache;
#ifdef CERTIFYMODE
	struct KEYWORDS_SHM *keywords_shm;
#endif

extern char fromhost[];

int flock(int fd, int op);

char *anno_path_of(char *board);

int mailnum_under_limit(char *userid);
int mailsize_under_limit(char *userid);

char * entity_char(char *s);
int file_has_word(char *file, char *word);
int f_append(const char *file, char *buf);
struct stat *f_stat(char *file);


int del_record(char *file, int size, int num);



char *getsenv(char *s);
int http_quit(void);
int http_fatal(char *fmt, ...);
int hsprintf(char *s, char *fmt, ...);
int hprintf(char *fmt, ...);
int hhprintf(char *fmt, ...);

extern char parm_name[][80];
extern char *parm_val[];
extern int parm_num;

char *getparm(char *var);
int get_shmkey(char *s);

int user_init(struct userec *x, struct user_info **y);

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);

#ifdef CERTIFYMODE
void Certify(char *board, struct fileheader *fh);
#endif

int post_article(char *board, char *title, char *file, char *id, char *nickname, char *ip, int o_id, int o_gid, int sig);

void check_title(char *title);
char* anno_path_of(char *board);

int getbnum(char *board);
struct boardheader *getbcache(char *board);
int count_mails(char *id, int *total, int *unread);

int send_msg(char *myuserid, int mypid, char *touserid, int topid, char *msg);

void iconexp(int exp);

int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);

int getusernum(char *userid);

extern struct override fff[];
extern int friendnum;

int loadfriend(char *id);
int isfriend(char *id);
void sort_friend(int left, int right);

int init_all();
char *sec(char c);
char *flag_str(int access);
char *flag_str2(int access, int has_read);
char *userid_str(char *s);

struct fileheader *get_file_ent(char *board, char *file);
char *getbfroma(char *path);
int set_my_cookie(void);

struct dir {
	char	filename[STRLEN-8];     /* the DIR files */
	unsigned int	id;
	unsigned int  	gid;
	char 	owner[STRLEN];
	char 	title[STRLEN-IDLEN-1];
	char 	szEraser[IDLEN+1];
	unsigned 	level;
	unsigned char 	accessed[4];   /* struct size = 256 bytes */
	unsigned int reid;
	time_t 	timeDeleted;
};

void trace(const char* content);

int specialboard(char* board);

void printpretable(void);
void printposttable(void);
void printpretable_lite(void);
void printposttable_lite(void);

int showcontent(char *filename);

void printpremarquee(char *width, char *height);
void printpostmarquee(void);

void showheadline(char *board);
void showrecommend(char *board, int showall, int showborder);
void showrawcontent(char *filename);
void showbrdlist(char *path,int withbr);

int strtourl(char * url, char * str);

//crypt.c
char *crypt1(char *buf, char *salt);
// Bcache.c
struct boardheader *getbcache(char *board);
struct userec *getuser(char *id);
int updatelastpost(char *board);
int count_online(void);
// bbs.c
int junkboard(char *board);
int has_BM_perm(struct userec *user, char *board);
int has_read_perm(struct userec *user, char *board);
int has_post_perm(struct userec *user, char *board);
int sethomefile(char *buf, char *id, char *file);
void do_report(const char* fname, const char* content);
// ModeType.c
char *ModeType(int mode);
// libBBS/string.c
char *cn_Ctime(time_t t);
char *Ctime(time_t t);
char *noansi(char *s);
char *nohtml(char *s);
char *strcasestr(char *s1, char *s2);
int strsncpy(char *s1, char *s2, int n);
char *const_ltrim(char *s);
char *ltrim(char *s);
char *rtrim(char *s);
// Record.c
long get_num_records(char *filename, int size);
int get_record(void *buf, int size, int num, char *file);
int append_record(void *buf, int size, char *file);
// stuff.c
char *horoscope(int month, int day);
// Goodbye.c
char *cexpstr(int exp);
char *cperf(int perf);
int countexp(struct userec *x);
int countperf(struct userec *x);
// pass.c
int checkpasswd(char *pw_crypted, char *pw_try);
// main.c
void SpecialID(const char* uid, char* host);
// libBBS/fileio.c
int dashf(char *fname);
int dashd(char *fname);
// bm.c
int isclubmember(char *member, char *board);

#endif
