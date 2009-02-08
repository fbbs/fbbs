// Function declarations.

#ifndef _FUNC_H_
#define _FUNC_H_

//uinfo.c
char *cexpstr(int exp);
#ifdef ALLOWGAME
char *cnummedals(int num);
char *cmoney(int num);
#endif
char *cperf(int perf);
int countexp(struct userec *udata);
int countperf(struct userec *udata);
int julian_day(int year, int month, int day);
int days_elapsed(int year, int month, int day, time_t now);

//brdcache.c (bcache.c)
extern struct BCACHE *brdshm;
extern struct boardheader *bcache;
extern int numboards;
int updatelastpost(const char *board);
int resolve_boards(void);
void flush_bcache(void);
void rebuild_brdshm(void);
int get_nextid(const char* boardname);
int getblankbnum(void);
struct boardheader *getbcache(const char *bname);
struct bstat *getbstat(const char *bname);
int getbnum(const char *bname, const struct userec *cuser);
int apply_boards(int (*func) (), const struct userec *cuser);
#ifdef NEWONLINECOUNT
void bonlinesync(time_t now);
#endif

//ucache.c (bcache.c)
extern struct UCACHE *uidshm;
extern struct UTMPFILE *utmpshm;
extern struct userec lookupuser;
int cmpuids(void *uid, void *up);
int dosearchuser(const char *userid, struct userec *user, int *unum);
int uhashkey(const char *userid, int *a1, int *a2);
int del_uidshm(int num, char *userid);
int load_ucache(void);
int substitut_record(char *filename, void *rptr, size_t size, int id);
int flush_ucache(void);
int resolve_ucache(void);
void setuserid(int num, char *userid);
int getuserid(char *userid, int uid, size_t len);
int searchuser(const char *userid);
int getuserec(const char *userid, struct userec *u);
int getuser(const char *userid);
int getuserbyuid(struct userec *u, int uid);
void resolve_utmp(void);
int allusers(void);
int get_online(void);
int get_status(int uid);
int refresh_utmp(void);
int getnewutmpent(struct user_info *up);
int apply_ulist(int (*fptr)());
int search_ulist(struct user_info *uentp, int (*fptr)(), int farg);
int search_ulistn(struct user_info *uentp, int (*fptr)(), int farg, int unum);
void update_ulist(struct user_info *uentp, int uent);
int who_callme(struct user_info *uentp, int (*fptr)(), int farg, int me);
int count_online(void);

//log.c
void report(const char *s, const char *userid);
void log_usies(const char *mode, const char *mesg, const struct userec *user);

//sysconf.c
extern char *sysconf_buf;
extern int sysconf_menu, sysconf_key, sysconf_len;
extern struct smenuitem *menuitem;
extern struct sdefine *sysvar;
char *sysconf_str(const char *key);
int sysconf_eval(const char *key);
void build_sysconf(const char *configfile, const char *imgfile);

//stuffs.c
char *sethomefile(char *buf, const char *userid, const char *filename);
char *setbpath(char *buf, const char *boardname);
char *setwbdir(char *buf, const char *boardname);
char *setbfile(char *buf, const char *boardname, const char *filename);
char *setmfile(char *buf, const char *userid, const char *filename);
char *setmdir(char *buf, const char *userid);
extern sigjmp_buf bus_jump;
void sigbus(int signo);
int bbskill(const struct user_info *user, int sig);
void SpecialID(const char *uid, char *host, int len);

//board.c
int changeboard(struct boardheader **bp, char *cboard, const char *board);
int chkBM(const struct boardheader *bp, const struct userec *up);
int isclubmember(const char *member, const char *board);
int hasreadperm(const struct userec *user, const struct boardheader *bp);
int junkboard(const struct boardheader *bp);

//mail.c
int check_maxmail(void);

//io.c
int egetch(void);
void update_endline(void);

#endif // _FUNC_H_
