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

