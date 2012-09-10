#ifndef FB_HELPER_H
#define FB_HELPER_H

#include "fbbs/session.h"

#ifdef ENABLE_BANK
# define TO_CENTS(y)  (y * 100)
# define TO_YUAN(c)  (c / 100.0L)
# define TO_YUAN_INT(c)  ((int)(c / 100.0L))
# define PERCENT_RANK(r)  (((int)(r * 1000)) / 10.0)
#endif // ENABLE_BANK

enum {
	INIT_CONV = 1,
	INIT_DB = 2,
	INIT_MDB = 4,
};

extern char *sethomefile(char *buf, const char *userid, const char *filename);
extern char *setbpath(char *buf, const char *boardname);
extern char *setwbdir(char *buf, const char *boardname);
extern char *setbfile(char *buf, const char *boardname, const char *filename);
extern char *setmfile(char *buf, const char *userid, const char *filename);
extern char *setmdir(char *buf, const char *userid);
extern void sigbus(int signo);
extern int bbs_kill(session_id_t sid, int pid, int sig);
extern void SpecialID(const char *uid, char *host, int len);
extern bool seek_in_file(const char *filename, const char *seekstr);
extern int add_to_file(const char *file, const char *str, size_t len, bool overwrite,
		bool (*equal)(const char *, size_t, const char *, size_t));
extern int del_from_file(const char *file, const char *str);
extern const char *mask_host(const char *host);
extern void add_signature(FILE *fp, const char *user, int sig);
extern int valid_gbk_file(const char *file, int replace);
extern char *valid_title(char *title);

extern void initialize_convert_env(void);
extern void initialize_db(void);
extern void initialize_mdb(void);
extern void initialize_environment(int flags);

extern char *genpasswd(const char *pw);
extern bool passwd_match(const char *pw_crypted, const char *pw_try);
extern bool passwd_check(const char *name, const char *pw_try);
extern int passwd_set(const char *name, const char *pw);

#endif // FB_HELPER_H
