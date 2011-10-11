#ifndef FB_LIBWEB_H
#define FB_LIBWEB_H

#include "../include/bbs.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "stdarg.h"
#include <crypt.h>
#include <fcgi_stdio.h>  //Should be included last.
#include "fbbs/web.h"

#define CHARSET		"gb18030"
//#define SQUID

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
	TLINES = 20,
	POSTS_PER_PAGE = 20,
	NEWMAIL_EXPIRE = 30,
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
	MODE_NOTICE = 8,
	MODE_FORUM = 9,

	PREF_NOSIG = 0x10,
	PREF_NOSIGIMG = 0x20,
};

enum {
	MAX_PARAMS = 256,    /**< Max number of parameter pairs */
	PARAM_NAMELEN = 80,  /**< Max length of a parameter name */
};

enum {
	POST_FIRST = 0x2,
	POST_LAST = 0x4,
	THREAD_FIRST_POST = 0x8,
	THREAD_LAST_POST = 0x10,
	THREAD_LAST = 0x20,
	THREAD_FIRST = 0x40,
};

const char *getsenv(const char *s);
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

void xml_header(const char *xslfile);
void http_header(void);

int iconexp(int exp, int *repeat);

int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);

int maxlen(const char *board);
time_t getfiletime(const struct fileheader *f);
struct fileheader *bbsmail_search(const void *ptr, size_t size, const char *file);
bool valid_mailname(const char *file);
int fcgi_init_loop(int mode);
int get_user_flag(void);
void set_user_flag(int flag);
const char *get_doc_mode_str(void);
void print_session(void);
void printable_filter(char *str);

extern int xml_print_post(const char *file, int option);
extern int xml_print_file(http_req_t *r, const char *file);

extern int bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp, bool extra);
#endif  //FB_LIBWEB_H

