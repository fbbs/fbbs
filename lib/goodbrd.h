#ifndef FDUBBS_GOODBRD_H
#define FDUBBS_GOODBRD_H

#include "../include/bbs.h"

enum {
	GOOD_BRC_NUM = 70,
};

typedef struct goodbrdheader {
	int id;   // id (order in the file, 1-based)
	int pid;  // parent id
	int pos;  // position of board in bcache (0-based, = bid - 1)
	unsigned int flag;
	char filename[STRLEN - 8];
	char title[STRLEN];
} gbrdh_t;

bool gbrd_is_custom_dir(const gbrdh_t *bh);

#endif //FDUBBS_GOODBRD_H
