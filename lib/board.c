#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/string.h"

static board_t curr_board = { .id = 0 };
board_t *currbp = &curr_board;
const char *currboard = curr_board.name;

void change_board(board_t *bp)
{
	if (!bp)
		return;
	memcpy(currbp, bp, sizeof(*currbp));
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

bool is_junk_board(const board_t *bp)
{
	if (bp && (bp->flag & BOARD_JUNK_FLAG))
		return true;
	return false;
}

bool is_board_dir(const board_t *bp)
{
	return (bp->flag & BOARD_DIR_FLAG);
}

void res_to_board(db_res_t *res, int row, board_t *bp)
{
	bp->id = db_get_integer(res, row, 0);
	bp->parent = db_get_integer(res, row, 3);
	bp->flag = (uint_t) db_get_integer(res, row, 4);
	bp->perm = (uint_t) db_get_integer(res, row, 5);
	strlcpy(bp->name, db_get_value(res, row, 1), sizeof(bp->name));
	strlcpy(bp->bms, db_get_value(res, row, 6), sizeof(bp->bms));
	strlcpy(bp->descr, db_get_value(res, row, 2), sizeof(bp->descr));
	strlcpy(bp->categ, db_get_value(res, row, 7), sizeof(bp->categ));
}

int get_board(const char *name, board_t *bp)
{
	bp->id = 0;
	db_res_t *res = db_query(
			BOARD_SELECT_QUERY_BASE "WHERE lower(b.name) = lower(%s)", name);
	if (res && db_res_rows(res) > 0)
		res_to_board(res, 0, bp);
	db_clear(res);
	return bp->id;
}

int get_board_by_bid(int bid, board_t *bp)
{
	bp->id = 0;
	db_res_t *res = db_query(
			BOARD_SELECT_QUERY_BASE "WHERE b.id = %d", bid);
	if (res && db_res_rows(res) > 0)
		res_to_board(res, 0, bp);
	db_clear(res);
	return bp->id;
}

void board_to_gbk(board_t *bp)
{
	GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
	convert_u2g(bp->descr, gbk_descr);
	strlcpy(bp->descr, gbk_descr, sizeof(bp->descr));

	GBK_BUFFER(categ, BOARD_CATEG_CCHARS);
	convert_u2g(bp->categ, gbk_categ);
	strlcpy(bp->categ, gbk_categ, sizeof(bp->categ));

	if ((bp->flag & BOARD_DIR_FLAG) && (bp->name[0] & 0x80)) {
		GBK_BUFFER(name, BOARD_NAME_LEN / 2);
		convert_u2g(bp->name, gbk_name);
		strlcpy(bp->name, gbk_name, sizeof(bp->name));
	}
}

bool is_bm(const struct userec *up, const board_t *bp)
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

bool user_has_read_perm(const struct userec *up, const board_t *bp)
{
	// Read restricted club
	if ((bp->flag & BOARD_CLUB_FLAG) && (bp->flag & BOARD_READ_FLAG)
			&& !is_bm(up, bp) && !isclubmember(up->userid, bp->name))
		return false;

	if (bp->perm == 0)
		return true;
	if (bp->flag & (BOARD_POST_FLAG | BOARD_NOZAP_FLAG))
		return true;
	if (up->userlevel & bp->perm)
		return true;

	return false;
}

bool has_read_perm(const board_t *bp)
{
	return user_has_read_perm(&currentuser, bp);
}

static bool user_has_post_perm(const struct userec *up, const board_t *bp)
{
	if (!HAS_PERM2(PERM_POST, up) || !HAS_PERM2(bp->perm, up))
		return false;

	if ((bp->flag & BOARD_CLUB_FLAG) && !is_bm(up, bp)
			&& !isclubmember(up->userid, bp->name))
		return false;

	char buf[STRLEN];
	setbfile(buf, bp->name, "deny_users");
	return !seek_in_file(buf, up->userid);
}

bool has_post_perm(const board_t *bp)
{
	return user_has_post_perm(&currentuser, bp);
}

bool fav_board_add(user_id_t uid, const char *bname, int bid, int folder, const struct userec *up)
{
	board_t b;

	if (bname) {
		if (!get_board(bname, &b))
			return false;
	} else {
		if (!get_board_by_bid(bid, &b))
			return false;
	}
	bid = b.id;
	
	if (!user_has_read_perm(up, &b))
		return false;

	if (folder <= FAV_BOARD_ROOT_FOLDER)
		folder = FAV_BOARD_ROOT_FOLDER;

	db_res_t *res = db_cmd("INSERT INTO fav_boards (user_id, board, folder)"
			" VALUES (%"DBIdUID", %d, %d)", uid, bid, folder);
	db_clear(res);
	return res;
}

bool fav_board_mkdir(user_id_t uid, const char *name, const char *descr)
{
	if (validate_utf8_input(name, BOARD_NAME_LEN) <= 0
			|| validate_utf8_input(descr, BOARD_DESCR_CCHARS) <= 0)
		return false;

	db_res_t *res = db_cmd("INSERT INTO fav_board_folders (user_id, name, descr)"
			" VALUES (%"DBIdUID", %s, %s)", uid, name, descr);
	db_clear(res);
	return res;
}

bool fav_board_rename(user_id_t uid, int id, const char *name, const char *descr)
{
	if (validate_utf8_input(name, BOARD_NAME_LEN) <= 0
			|| validate_utf8_input(descr, BOARD_DESCR_CCHARS) <= 0)
		return false;

	db_res_t *res = db_cmd("UPDATE fav_board_folders (name, descr)"
			" VALUES (%s, %s) WHERE id = %d AND user_id = %"DBIdUID,
			name, descr, id, uid);
	db_clear(res);
	return res;
}

bool fav_board_rmdir(user_id_t uid, int id)
{
	db_res_t *res = db_cmd("DELETE FROM fav_board_folders WHERE id = %d"
			" AND user_id = %"DBIdUID, id, uid);
	db_clear(res);
	return res;
}

bool fav_board_rm(user_id_t uid, int id)
{
	db_res_t *res = db_cmd("DELETE FROM fav_boards WHERE board = %d"
			" AND user_id = %"DBIdUID, id, uid);
	db_clear(res);
	return res;
}

bool fav_board_mv(user_id_t uid, int id, int parent)
{
	if (parent < FAV_BOARD_ROOT_FOLDER)
		parent = FAV_BOARD_ROOT_FOLDER;
	db_res_t *res = db_cmd("UPDATE fav_boards SET folder = %d"
			" WHERE user_id = %"DBIdUID" AND board = %d", parent, uid, id);
	db_clear(res);
	return res;
}

int count_onboard(int bid)
{
	return (int) mdb_integer(0, "ZCOUNT", "current_board %d %d", bid, bid);
}
