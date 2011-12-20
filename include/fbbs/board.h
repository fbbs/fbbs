#ifndef FB_BOARD_H
#define FB_BOARD_H

#include "fbbs/dbi.h"

typedef struct board_t {
	int id;
	int parent;
	uint_t flag;
	uint_t perm;
	char bms[BM_LEN];
	char descr[STRLEN];
} board_t;

#endif // FB_BOARD_H
