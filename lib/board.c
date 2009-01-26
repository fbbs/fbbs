#include <bbs.h>

void changeboard(struct boardheader **bpp, char *cboard, const char *board)
{
	*bpp = getbcache(board);
	strcpy(cboard, board);
}

int chkBM(const struct boardheader *bp, const struct userec *up)
{
	char *ptr;
	char BMstrbuf[BM_LEN - 1];

	if (bp == NULL || up == NULL)
		return 0;

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

	strlcpy(BMstrbuf, bp->BM, sizeof(BMstrbuf));
	ptr = strtok(BMstrbuf, ",: ;|&()\0\n");
	while (ptr) {
		if (!strcmp(ptr, up->userid))
			return YEA;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	return NA;
}

int isclubmember(const char *member, const char *board)
{
	FILE* fp;
	char uident[IDLEN + 1];
	char fname[STRLEN];
	char line[256];

	setbfile(fname, board, "club_users");
	if (!(fp = fopen(fname, "r")))
		return 0;
	while (fgets(line, 256, fp)) {
		strlcpy(uident, line, IDLEN);
		uident[IDLEN] = '\0';
		strtok(uident, " \r\n\t");
		if (strcasecmp(member, uident)== 0)
			return 1;
	}
	fclose(fp);
	return 0;
}

// Checks if 'user' have read permission to board 'bp'.
int hasreadperm(const struct userec *user, const struct boardheader *bp)
{
	if (bp == NULL || user == NULL)
		return 0;

	// Read restricted club
	if ((bp->flag & BOARD_CLUB_FLAG)
		&& (bp->flag & BOARD_READ_FLAG)
		&& !chkBM(bp, user)
		&& !isclubmember(user->userid, bp->filename))
		return 0;

	// Following lines deal with non-clubs.
	if (bp->level == 0)
		return 1;
	if (bp->flag & (BOARD_POST_FLAG | BOARD_NOZAP_FLAG))
		return 1;
	if (user->userlevel & bp->level)
		return 1;

	return 0;
}

int junkboard(const struct boardheader *bp)
{
	if (bp == NULL)
		return 0;
	if (bp->flag & BOARD_JUNK_FLAG)
		return 1;

	return 0;
}
