#ifndef FB_LIBBBS_H
#define FB_LIBBBS_H

#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include "bbs.h"
#include "fbbs/board.h"

//modetype.c
int get_raw_mode(int mode);
const char *mode_type(int mode);
bool is_web_user(int mode);
int get_web_mode(int mode);

//shm.c
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);

#endif // FB_LIBBBS_H
