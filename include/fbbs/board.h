#ifndef FB_BOARD_H
#define FB_BOARD_H

#include "fbbs/dbi.h"

enum {
	BOARD_NAME_LEN = 17,
	BOARD_DESCR_CCHARS = 24,
	BOARD_BM_LEN = 55,
	BOARD_CATEG_CCHARS = 2,
	BOARD_SECTOR_NAME_CCHARS = 4,
	BOARD_SECTOR_SHORT_CCHARS = 2,

	BOARD_VOTE_FLAG = 0x1,
	BOARD_NOZAP_FLAG = 0x2,
	BOARD_OUT_FLAG = 0x4,
	BOARD_ANONY_FLAG = 0x8,
	BOARD_NOREPLY_FLAG = 0x10,
	BOARD_JUNK_FLAG = 0x20,
	BOARD_CLUB_FLAG = 0x40,
	BOARD_READ_FLAG = 0x80,
	BOARD_POST_FLAG = 0x100,
	BOARD_DIR_FLAG = 0x200,
	BOARD_PREFIX_FLAG = 0x400,
	BOARD_RECOMMEND_FLAG = 0x800,

	FAV_BOARD_ROOT_FOLDER = 1,
	FAV_BOARD_LIMIT = 70,
};

typedef struct {
	int id;
	int parent;
	uint_t flag;
	uint_t perm;
	char name[BOARD_NAME_LEN + 1];
	char bms[BOARD_BM_LEN + 1];
	char descr[BOARD_DESCR_CCHARS * 4 + 1];
	char categ[BOARD_CATEG_CCHARS * 4 + 1];
} board_t;

#define BOARD_BASE_FIELDS \
	"b.id, b.name, b.descr, b.parent, b.flag, b.perm, b.bms, c.name "
#define BOARD_BASE_TABLES \
	"boards b JOIN board_categs c ON b.categ = c.id "
#define BOARD_SELECT_QUERY_BASE \
	"SELECT " BOARD_BASE_FIELDS "FROM " BOARD_BASE_TABLES

extern int get_board(const char *name, board_t *bp);
extern int get_board_by_bid(int bid, board_t *bp);
extern void res_to_board(db_res_t *res, int row, board_t *bp);
extern void board_to_gbk(board_t *bp);
extern bool is_board_manager(const struct userec *up, const board_t *bp);
extern bool has_read_perm(const struct userec *up, const board_t *bp);
extern bool has_post_perm(const struct userec *up, const board_t *bp);

extern bool fav_board_add(user_id_t uid, const char *bname, int bid, int folder);

#endif // FB_BOARD_H
