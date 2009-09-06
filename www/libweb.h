#ifndef FDUBBS_LIBWEB_H
#define FDUBBS_LIBWEB_H

#include "../include/bbs.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "stdarg.h"
#include <crypt.h>
#include <fcgi_stdio.h>  //Should be included last.

#define CHARSET		"gb2312"
#define SQUID

#ifdef FDQUAN
	#define SECNUM 9
#else
	#define SECNUM 12
#endif

enum HTTP_STATUS {
	HTTP_STATUS_OK = 200,
	HTTP_STATUS_BADREQUEST = 400,
	HTTP_STATUS_FORBIDDEN = 403,
	HTTP_STATUS_NOTFOUND = 404,
	HTTP_STATUS_INTERNAL_ERROR = 500,
	HTTP_STATUS_SERVICE_UNAVAILABLE = 503
};

enum {
	UPLOAD_MAX = 1 * 1024 * 1024,
	UPLOAD_OVERHEAD = 1024,
	BBSMSG_RECORD_LENGTH = 129,
	BBSMSG_LENGTH = 51,
	BBSMSG_SPLIT_OFFSET = 111,
	BBSMSG_SENDER_OFFSET = 12,
	BBSMSG_PID_OFFSET = 122,
	BBSMSG_CONTENT_OFFSET = 23
};

enum {
	TLINES = 20
};

enum {
	MODE_NORMAL = 0,
	MODE_DIGEST = 1,
	MODE_THREAD = 2,
	MODE_REMAIN = 3,
	MODE_TOPICS = 4,
	MODE_AUTHOR_FUZZ = 5,
	MODE_AUTHOR = 6,
	MODE_KEYWORD = 7,
};

#define file_size(x) f_stat(x)->st_size
#define file_time(x) f_stat(x)->st_mtime
#define file_rtime(x) f_stat(x)->st_atime
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)

void setcookie(const char *a, const char *b);
void refreshto(int second, const char *url);

#define FLOCK(x,y) flock(x,y)

extern char seccode[][6];
extern char secname[][2][20];

struct post_log {
	char	author[IDLEN+1]; // TODO: is this too short for "id."?
	char	board[18];
	char	title[66];
	time_t	date;
	int	number;
};

extern int loginok;

extern struct userec currentuser;
extern struct user_info *u_info;
extern char fromhost[];

struct stat *f_stat(char *file);

char *getsenv(const char *s);
int http_fatal(const char *prompt);
int http_fatal2(enum HTTP_STATUS status, const char *prompt);
void xml_fputs(const char *s, FILE *stream);
void xml_fputs2(const char *s, size_t size, FILE *stream);
int xml_printfile(const char *file, FILE *stream);

extern char parm_name[][80];
extern char *parm_val[];
extern int parm_num;

int user_init(struct userec *x, struct user_info **y);
void xml_header(const char *xslfile);
void http_header(void);

int post_article(const struct userec *user, const struct boardheader *bp, 
		const char *title, const char *content, const char *ip, 
		const struct fileheader *o_fp);

int iconexp(int exp, int *repeat);

int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);

char *getbfroma(const char *path);
int set_my_cookie(void);

struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid);
bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp);
int maxlen(const char *board);
time_t getfiletime(const struct fileheader *f);
struct fileheader *bbsmail_search(const void *ptr, size_t size, const char *file);
bool valid_mailname(const char *file);
char *get_permission(void);
const char *get_referer(void);

char *getparm(const char *name);
void http_parm_init(void);
void parse_post_data(void);

int fcgi_init_all(void);
int fcgi_init_loop(void);

#endif  //FDUBBS_LIBWEB_H
