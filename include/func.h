// Function declarations.

#ifndef FB_FUNC_H
#define FB_FUNC_H

#include "bbs.h"

#define BADLOGINFILE   "logins.bad"

//brdcache.c (bcache.c)
extern struct BCACHE *brdshm;
int resolve_boards(void);
void rebuild_brdshm(void);
int get_nextid(const char* boardname);
unsigned int get_nextid2(int bid);
struct bstat *getbstat(int bid);

//ucache.c (bcache.c)
extern struct UCACHE *uidshm;
extern struct UTMPFILE *utmpshm;
extern struct userec lookupuser;
int cmpuids(const void *uid, const void *up);
int dosearchuser(const char *userid, struct userec *user, int *unum);
int uhashkey(const char *userid, int *a1, int *a2);
int del_uidshm(int num, char *userid);
int ucache_lock();
void ucache_unlock(int fd);
int load_ucache(void);
int substitut_record(char *filename, const void *rptr, size_t size, int id);
int flush_ucache(void);
int resolve_ucache(void);
void setuserid(int num, const char *userid);
int getuserid(char *userid, int uid, size_t len);
int searchuser(const char *userid);
int getuserec(const char *userid, struct userec *u);
int getuser(const char *userid);
int getuserbyuid(struct userec *u, int uid);
void resolve_utmp(void);
int allusers(void);
int get_online(void);
int cmpfnames(void *user, void *over);

//log.c
void report(const char *s, const char *userid);
void log_usies(const char *mode, const char *mesg, const struct userec *user);
void log_attempt(const char *name, const char *addr, const char *type);

//board.c
int isclubmember(const char *member, const char *board);

//mail.c
int check_maxmail(void);

//io.c
int igetkey(void);
int egetch(void);
void update_endline(void);

//convert.c
#ifdef ALLOWSWITCHCODE
int switch_code(void);
int resolve_gbkbig5_table(void);
int convert_g2b(int ch);
int convert_b2g(int ch);
#endif // ALLOWSWITCHCODE

#endif // FB_FUNC_H
