#ifndef FB_LIBBBS_H
#define FB_LIBBBS_H

#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include "bbs.h"
#include "hash.h"

//mmdecode.c
void _mmdecode(unsigned char *str);

//modetype.c
int get_raw_mode(int mode);
const char *mode_type(int mode);
bool is_web_user(int mode);
int get_web_mode(int mode);

//boardrc.c
void brc_update(const char *userid, const char *board);
int brc_initial(const char* userid, const char *board);
void brc_addlist(const char *filename);
int brc_unread(const char *filename);
int brc_unread1(int ftime);
int brc_clear(int ent, const char *direct, int clearall);
void brc_zapbuf(int *zbuf);
int brc_fcgi_init(const char *user, const char *board);
bool brc_board_unread(const char *user, const struct boardheader *bp);

//pass.c
char *genpasswd(const char *pw);
int checkpasswd(const char *pw_crypted, const char *pw_try);

//shm.c
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);

#endif // FB_LIBBBS_H
