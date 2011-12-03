#ifndef FB_HELPER_H
#define FB_HELPER_H

#ifdef ENABLE_BANK
# define TO_CENTS(y)  (y * 100)
# define TO_YUAN(c)  (c / 100.0L)
# define TO_YUAN_INT(c)  ((int)(c / 100.0L))
# define PERCENT_RANK(r)  (((int)(r * 1000)) / 10.0)
#endif // ENABLE_BANK

extern sigjmp_buf bus_jump;

extern char *sethomefile(char *buf, const char *userid, const char *filename);
extern char *setbpath(char *buf, const char *boardname);
extern char *setwbdir(char *buf, const char *boardname);
extern char *setbfile(char *buf, const char *boardname, const char *filename);
extern char *setmfile(char *buf, const char *userid, const char *filename);
extern char *setmdir(char *buf, const char *userid);
extern void sigbus(int signo);
extern int bbskill(struct user_info *user, int sig);
extern void SpecialID(const char *uid, char *host, int len);
extern bool seek_in_file(const char *filename, const char *seekstr);
extern const char *mask_host(const char *host);
extern void add_signature(FILE *fp, const char *user, int sig);
extern int valid_gbk_file(const char *file, int replace);
extern char *valid_title(char *title);
extern void initialize_convert_env(void);

#endif // FB_HELPER_H
