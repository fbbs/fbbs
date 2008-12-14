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

char seccode[SECNUM][6];
char secname[SECNUM][2][20];

struct boardheader *getbcache();
int junkboard(char *board);

struct post_log {
	char	author[IDLEN+1];
	char	board[18];
	char	title[66];
	time_t	date;
	int	number;
};

int loginok;

struct userec currentuser;
struct user_info *u_info;
struct UTMPFILE *shm_utmp;
struct BCACHE *shm_bcache;
struct UCACHE *shm_ucache;
struct boardheader *bcache;
#ifdef CERTIFYMODE
	struct KEYWORDS_SHM *keywords_shm;
#endif

char fromhost[256];

int flock(int fd, int op);
struct userec *getuser();
char *strcasestr();
char *ModeType();
char *anno_path_of();
void sort_friend(int left, int right);

static int shm_lock(char *lockname);
static void shm_unlock(int fd);

int strncasecmp2(char *s1, char *s2,int n);
long get_num_records(char *filename, int size);
char *setmdir(char *buf, char *userid);
int mailnum_under_limit(char *userid);
int mailsize_under_limit(char *userid);
int getmailboxsize(unsigned int userlevel);
int getmailsize(char *userid);
char * entity_char(char *s);
int file_has_word(char *file, char *word);
int f_append(const char *file, char *buf);
struct stat *f_stat(char *file);

int get_record(void *buf, int size, int num, char *file);
int put_record(void *buf, int size, int num, char *file);
int append_record(void *buf, int size, char *file);
int del_record(char *file, int size, int num);
char *cn_Ctime(time_t t);
char *Ctime(time_t t);
char *noansi(char *s);
char *nohtml(char *s);

char *strright(char *s, int len);
char *strcasestr(char *s1, char *s2);
int strsncpy(char *s1, char *s2, int n);
int strnncpy(char *s, int *l, char *s2);
char *const_ltrim(char *s);
char *ltrim(char *s);
char *rtrim(char *s);

char *get_new_shm(int key, int size);
char *get_old_shm(int key, int size);
char *get_shm(int key, int size);

char *getsenv(char *s);
int http_quit();
int http_fatal(char *fmt, ...);
int hsprintf(char *s, char *fmt, ...);
int hprintf(char *fmt, ...);
int hhprintf(char *fmt, ...);

char parm_name[256][80], *parm_val[256];
int parm_num;
int parm_add(char *name, char *val);
int http_init();

int __to16(char c);
int __unhcode(char *s);

char *getparm(char *var);
int get_shmkey(char *s);

int shm_init();
int user_init(struct userec *x, struct user_info **y);

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);
int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);

#ifdef CERTIFYMODE
void Certify(char *board, struct fileheader *fh);
#endif

int getlastpost(char *board, int *lastpost, int *total);
int updatelastpost(char *board);

static void bcache_setreadonly(int readonly);
static int bcache_lock();
static void bcache_unlock(int fd);

int get_nextid(char* boardname);
int get_nextid_bid(int bid);
int post_article(char *board, char *title, char *file, char *id, char *nickname, char *ip, int o_id, int o_gid, int sig);

void check_title(char *title);
int sig_append(FILE *fp, char *id, int sig);
char* anno_path_of(char *board);
int has_BM_perm(struct userec *user, char *board);
int has_read_perm(struct userec *user, char *board);
int has_post_perm(struct userec *user, char *board);
int getbnum(char *board);
struct boardheader *getbcache(char *board);
int count_mails(char *id, int *total, int *unread);
int findnextutmp(char *id, int from);
int sethomefile(char *buf, char *id, char *file);
int send_msg(char *myuserid, int mypid, char *touserid, int topid, char *msg);
char *horoscope(int month, int day);
char *ModeType(int mode);
char *cexpstr(int exp);
void iconexp(int exp);
char *cperf(int perf);
int countexp(struct userec *x);
int countperf(struct userec *x);
int modify_mode(struct user_info *x, int newmode);
int save_user_data(struct userec *x);
int is_bansite(char *ip);
int user_perm(struct userec *x, int level);
int uhashkey(char *userid, char *a1, char *a2);
int getusernum(char *userid);
struct userec *getuser(char *id);

int checkpasswd(char *pw_crypted, char *pw_try);
int checkuser(char *id, char *pw);
int count_id_num(char *id);
int count_online();
int count_online2();

struct override fff[MAXFRIENDS];
int friendnum;

int loadfriend(char *id);
int isfriend(char *id);
int friend_search(char *id,struct override *fff,int tblsize);
void swap_friend(int a,int b);
int compare_user_record(struct override *left,struct override *right);
void sort_friend(int left, int right);
struct override bbb[MAXREJECTS];
int badnum;
int loadbad(char *id);
int isbad(char *id);

int init_all();
int init_no_http();
char *sec(char c);
char *flag_str(int access);
char *flag_str2(int access, int has_read);
char *userid_str(char *s);
int fprintf2(FILE *fp, char *s);
struct fileheader *get_file_ent(char *board, char *file);
char *getbfroma(char *path);
int set_my_cookie();
int has_fill_form();

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

void do_report(const char* fname, const char* content);
void trace(const char* content);

int specialboard(char* board);
void SpecialID(const char* uid, char* host);

int dashf(char *fname);
int dashd(char *fname);
void printpretable();
void printposttable();
void printpretable_lite();
void printposttable_lite();

int showcontent(char *filename);

void printpremarquee(char *width, char *height);
void printpostmarquee();

void showheadline(char *board);
void showrecommend(char *board, int showall, int showborder);
void showrawcontent(char *filename);
void showbrdlist(char *path,int withbr);

int strtourl(char * url, char * str);
int urltostr(char * str, char * url);

int isclubmember(char *member, char *board);

//crypt.c
char *crypt1(char *buf, char *salt);
