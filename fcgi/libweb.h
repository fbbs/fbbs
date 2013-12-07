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
};

enum {
	TLINES = 20,
	NEWMAIL_EXPIRE = 30,
};

enum {
	PREF_NOSIG = 0x10,
	PREF_NOSIGIMG = 0x20,
};

enum {
	MAX_PARAMS = 256,    /**< Max number of parameter pairs */
	PARAM_NAMELEN = 80,  /**< Max length of a parameter name */
};

const char *getsenv(const char *s);
const char *get_referer(void);

void refreshto(int second, const char *url);

extern struct userec currentuser;
extern char fromhost[];

void xml_fputs(const char *s);
void xml_fputs2(const char *s, size_t size);
size_t xml_fputs3(const char *s, size_t size, FILE *stream);
void xml_fputs4(const char *s, size_t size);
int xml_printfile(const char *file);

void xml_header(const char *xslfile);
void http_header(void);

int save_user_data(struct userec *x);

int maxlen(const char *board);
time_t getfiletime(const struct fileheader *f);
struct fileheader *bbsmail_search(const void *ptr, size_t size, const char *file);
bool valid_mailname(const char *file);
int get_user_flag(void);
void set_user_flag(int flag);
void print_session(void);
void string_remove_non_printable_gbk(char *str);

#endif  //FB_LIBWEB_H
