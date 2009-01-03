#include <bbs.h>

int changeboard(struct boardheader *bp, char *cboard, const char *board)
{
	bp = getbcache(board);
	strcpy(cboard, board);
}

int chkBM(const struct boardheader *bp, const struct userec *up)
{
	char *ptr;
	char BMstrbuf[BM_LEN - 1];

	if (bp->flag & BOARD_CLUB_FLAG) {
		if(up->userlevel & PERM_OCLUB)
			return YEA;
	}
	else {
		if (up->userlevel & PERM_BLEVELS)
			return YEA;
	}
	if (!(up->userlevel & PERM_BOARDS))
		return NA;

	strcpy(BMstrbuf, bp->BM);
	ptr = strtok(BMstrbuf, ",: ;|&()\0\n");
	while (ptr) {
		if (!strcmp(ptr, up->userid))
			return YEA;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	return NA;
}
