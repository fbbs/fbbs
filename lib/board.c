#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"

// Copies 'board' to 'cboard' and sets 'bpp' accordingly.
// Returns 0 on success, -1 on error.
int changeboard(struct boardheader **bpp, char *cboard, const char *board)
{
	if (bpp == NULL || cboard == NULL || board == NULL)
		return -1;

	*bpp = getbcache(board);
	if (*bpp == NULL)
		return -1;

	strcpy(cboard, board);
	return 0;
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
		strlcpy(uident, line, sizeof(uident));
		strtok(uident, " \r\n\t");
		if (strcasecmp(member, uident)== 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

// Checks if 'user' have read permission to board 'bp'.
// Returns 1 for true, 0 for false or NULL pointers.
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

bool haspostperm(const struct userec *user, const struct boardheader *bp)
{
	if (bp == NULL || user == NULL || !HAS_PERM2(PERM_POST, user)
			|| !HAS_PERM2(bp->level, user))
		return false;
	if ((bp->flag & BOARD_CLUB_FLAG)
			&& !chkBM(bp, user)
			&& !isclubmember(user->userid, bp->filename))
		return false;
	char buf[STRLEN];
	setbfile(buf, bp->filename, "deny_users");
	return !seek_in_file(buf, user->userid);
}

// Check if board 'bp' is JUNK (posts not counted).
// Returns 1 if 'bp' is JUNK, 0 if 'bp' is NULL or not JUNK.
int junkboard(const struct boardheader *bp)
{
	if (bp == NULL)
		return 0;
	if (bp->flag & BOARD_JUNK_FLAG)
		return 1;

	return 0;
}

bool is_board_dir(const struct boardheader *bp)
{
	return (bp->flag & BOARD_DIR_FLAG);
}

/**
 * Get board description.
 * @param bp pointer to board header.
 * @return board description.
 */
const char *get_board_desc(const struct boardheader *bp)
{
	return (bp->title + 11);
}

static void res_to_board(db_res_t *res, board_t *bp)
{
	bp->id = db_get_integer(res, 0, 0);
	bp->parent = db_get_integer(res, 0, 3);
	bp->flag = (uint_t) db_get_integer(res, 0, 4);
	bp->perm = (uint_t) db_get_integer(res, 0, 5);
	strlcpy(bp->name, db_get_value(res, 0, 1), sizeof(bp->name));
	strlcpy(bp->bms, db_get_value(res, 0, 6), sizeof(bp->bms));
	strlcpy(bp->descr, db_get_value(res, 0, 2), sizeof(bp->descr));
}

int get_board(const char *name, board_t *bp)
{
	bp->id = 0;
	db_res_t *res = db_query("SELECT id, name, descr, parent, flag, perm, bms"
			" FROM boards WHERE lower(name) = lower(%s)", name);
	if (res && db_res_rows(res) > 0)
		res_to_board(res, bp);
	db_clear(res);
	return bp->id;
}

int get_board_by_bid(int bid, board_t *bp)
{
	bp->id = 0;
	db_res_t *res = db_query("SELECT id, name, descr, parent, flag, perm, bms"
			" FROM boards WHERE id = %d", bid);
	if (res && db_res_rows(res) > 0)
		res_to_board(res, bp);
	db_clear(res);
	return bp->id;
}

int get_board_gbk(const char *name, board_t *bp)
{
	if (get_board(name, bp) == 0)
		return 0;
	GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
	convert_u2g(bp->descr, gbk_descr);
	strlcpy(bp->descr, gbk_descr, sizeof(bp->descr));
	return bp->id;
}

bool is_board_manager(const struct userec *up, const board_t *bp)
{
	if ((bp->flag & BOARD_CLUB_FLAG) && (up->userlevel & PERM_OCLUB))
		return true;
	if (up->userlevel & PERM_BLEVELS)
		return true;
	if (!(up->userlevel & PERM_BOARDS))
		return false;

	// TODO: load from database when uid is ready
	char buf[BOARD_BM_LEN + 1];
	strlcpy(buf, bp->bms, sizeof(buf));
	const char *ptr = strtok(buf, ",: ;|&()\0\n");
	while (ptr) {
		if (streq(ptr, up->userid))
			return true;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	return false;
}

bool has_read_perm(const struct userec *up, const board_t *bp)
{
	// Read restricted club
	if ((bp->flag & BOARD_CLUB_FLAG) && (bp->flag & BOARD_READ_FLAG)
			&& !is_board_manager(up, bp)
			&& !isclubmember(up->userid, bp->name))
		return false;

	if (bp->perm == 0)
		return true;
	if (bp->flag & (BOARD_POST_FLAG | BOARD_NOZAP_FLAG))
		return true;
	if (up->userlevel & bp->perm)
		return true;

	return false;
}

bool has_post_perm(const struct userec *up, const board_t *bp)
{
	if (!HAS_PERM2(PERM_POST, up) || !HAS_PERM2(bp->perm, up))
		return false;

	if ((bp->flag & BOARD_CLUB_FLAG) && !is_board_manager(up, bp)
			&& !isclubmember(up->userid, bp->name))
		return false;

	char buf[STRLEN];
	setbfile(buf, bp->name, "deny_users");
	return !seek_in_file(buf, up->userid);
}
