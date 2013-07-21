#ifndef FB_LIBWEB_H
#define FB_LIBWEB_H

#include "../include/bbs.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "stdarg.h"
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#include <fcgi_stdio.h>  //Should be included last.
#include "fbbs/web.h"

#define CHARSET		"gb18030"
//#define SQUID

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
	NEWMAIL_EXPIRE = 30,
};

enum {
	MODE_THREAD = 2,
	MODE_TOPICS = 4,
	MODE_FORUM = 9,

	PREF_NOSIG = 0x10,
	PREF_NOSIGIMG = 0x20,
};

enum {
	MAX_PARAMS = 256,    /**< Max number of parameter pairs */
	PARAM_NAMELEN = 80,  /**< Max length of a parameter name */
};

const char *getsenv(const char *s);
const char *get_referer(void);

void setcookie(const char *a, const char *b);
void refreshto(int second, const char *url);

#define FLOCK(x,y) flock(x,y)

extern struct userec currentuser;
extern char fromhost[];

void xml_fputs(const char *s, FILE *stream);
void xml_fputs2(const char *s, size_t size, FILE *stream);
size_t xml_fputs3(const char *s, size_t size, FILE *stream);
int xml_printfile(const char *file);

void xml_header(const char *xslfile);
void http_header(void);

int iconexp(int exp, int *repeat);

int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);

int maxlen(const char *board);
time_t getfiletime(const struct fileheader *f);
struct fileheader *bbsmail_search(const void *ptr, size_t size, const char *file);
bool valid_mailname(const char *file);
int get_user_flag(void);
void set_user_flag(int flag);
const char *get_doc_mode_str(void);
void print_session(void);
void printable_filter(char *str);

#endif  //FB_LIBWEB_H
