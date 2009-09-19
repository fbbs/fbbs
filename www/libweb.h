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

enum {
	MAX_PARAMS = 256,    /**< Max number of parameter pairs */
	PARAM_NAMELEN = 80,  /**< Max length of a parameter name */
	MAX_CONTENT_LENGTH = 5 * 1024 * 1024, /**< Max content length*/
};

extern char param_name[][PARAM_NAMELEN];
extern char *param_val[];
extern int param_num;
const char *getsenv(const char *s);
int parse_post_data(void);
char *getparm(const char *name);
const char *get_referer(void);

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


void xml_fputs(const char *s, FILE *stream);
void xml_fputs2(const char *s, size_t size, FILE *stream);
int xml_printfile(const char *file, FILE *stream);

int user_init(struct userec *x, struct user_info **y);
void xml_header(const char *xslfile);
void http_header(void);

int iconexp(int exp, int *repeat);

int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);

char *getbfroma(const char *path);
struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid);
int maxlen(const char *board);
time_t getfiletime(const struct fileheader *f);
struct fileheader *bbsmail_search(const void *ptr, size_t size, const char *file);
bool valid_mailname(const char *file);
char *get_permission(void);
int fcgi_init_loop(void);

#endif  //FDUBBS_LIBWEB_H
