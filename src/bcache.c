#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

void setoboard(char *bname) {
	register int i;

	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG || HAS_PERM(bcache[i].level)
				|| (bcache[i].flag & BOARD_NOZAP_FLAG)) {
			if (bcache[i].filename[0] != 0 && bcache[i].filename[0] != ' ') {
				strcpy(bname, bcache[i].filename);
				return;
			}
		}
	}
}

int haspostperm(char *bname) {
	register int i;
	/*	if (digestmode==TRASH_MODE||digestmode==JUNK_MODE){
	 return 0;
	 }
	 */
	if (deny_me(bname)) {
		return 0;
	}

	/*
	 if (strcmp(bname, DEFAULTBOARD) == 0)
	 return 1;
	 *///added by roly 02.01.27 disable postperm in sysop of that has no perm_post
	if ((i = getbnum(bname, &currentuser)) == 0) {
		return 0;
	}
	set_safe_record();
	if (!HAS_PERM(PERM_POST))
		return 0;
	return HAS_PERM(bcache[i - 1].level);
}

int
normal_board(bname)
char *bname;
{
	register int i;
	if (strcmp(bname, DEFAULTBOARD) == 0)
	return 1;
	if ((i = getbnum(bname, &currentuser)) == 0)//版面不可见
	return 0;
	if (bcache[i - 1].flag & BOARD_NOZAP_FLAG)
	return 1;
	if ((bcache[i - 1].flag & BOARD_POST_FLAG) || bcache[i - 1].level)
	return 0;
	return 1;
}
