#ifndef FB_BOARD_H
#define FB_BOARD_H

#include "fbbs/dbi.h"

enum {
	BOARD_NAME_LEN = 17,
	BOARD_DESCR_CCHARS = 24,
	BOARD_BM_LEN = 55,
};

typedef struct board_t {
	int id;
	int parent;
	uint_t flag;
	uint_t perm;
	char name[BOARD_NAME_LEN + 1];
	char bms[BOARD_BM_LEN + 1];
	char descr[BOARD_DESCR_CCHARS * 4 + 1];
} board_t;

extern int get_board(const char *name, board_t *bp);
extern int get_board_gbk(const char *name, board_t *bp);

#endif // FB_BOARD_H
