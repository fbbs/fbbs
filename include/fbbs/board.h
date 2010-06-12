#ifndef FB_BOARD_H
#define FB_BOARD_H

#include "fbbs/util.h"

typedef struct board_t {
	seq_t bid;
	seq_t parent;
	seq_t current;
	uint_t flag;
	uint_t perm;
	char name[18];
	char title[62];
} board_t;

typedef struct bm_t {
	seq_t bid;
	seq_t uid;
	fb_time_t date;
} bm_t;

#endif // FB_BOARD_H
