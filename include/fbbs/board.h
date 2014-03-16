#ifndef FB_BOARD_H
#define FB_BOARD_H

#include "fbbs/dbi.h"
#include "fbbs/user.h"

enum {
	BOARD_NAME_LEN = 17,
	BOARD_DESCR_CCHARS = 24,
	BOARD_BM_LEN = 55,
	BOARD_CATEG_CCHARS = 2,
	BOARD_SECTOR_NAME_CCHARS = 4,
	BOARD_SECTOR_SHORT_CCHARS = 2,

	FAV_BOARD_ROOT_FOLDER = 1,
	FAV_BOARD_LIMIT = 70,
};

typedef enum {
	BOARD_FLAG_VOTE = 1,
	BOARD_FLAG_NOZAP = 1 << 1,
	BOARD_FLAG_OUT = 1 << 2,
	BOARD_FLAG_ANONY = 1 << 3,
	BOARD_FLAG_NOREPLY = 1 << 4,
	BOARD_FLAG_JUNK = 1 << 5,
	BOARD_FLAG_CLUB = 1 << 6,
	BOARD_FLAG_READ = 1 << 7,
	BOARD_FLAG_POST = 1 << 8,
	BOARD_FLAG_DIR = 1 << 9,
	BOARD_FLAG_PREFIX = 1 << 10,
	BOARD_FLAG_RECOMMEND = 1 << 11,
} board_flag_e;

enum {
	AC_LIST_BOARDS_AND_DIR,
	AC_LIST_BOARDS_ONLY,
	AC_LIST_DIR_ONLY,
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
	"b.id, b.name, b.descr, b.parent, b.flag, b.perm, b.bms, b.categ "
#define BOARD_BASE_TABLES \
	"boards b "
#define BOARD_SELECT_QUERY_BASE \
	"SELECT " BOARD_BASE_FIELDS "FROM " BOARD_BASE_TABLES

extern int get_board(const char *name, board_t *bp);
extern int get_board_by_bid(int bid, board_t *bp);
extern void res_to_board(db_res_t *res, int row, board_t *bp);
extern void board_to_gbk(board_t *bp);
extern bool is_bm(const struct userec *up, const board_t *bp);
#define am_bm(bp)  is_bm(&currentuser, bp)
#define am_curr_bm()  is_bm(&currentuser, currbp)
extern bool has_read_perm(const board_t *bp);
extern bool user_has_read_perm(const struct userec *up, const board_t *bp);
extern bool has_post_perm(const board_t *bp);

extern bool fav_board_add(user_id_t uid, const char *bname, int bid, int folder, const struct userec *up);
extern bool fav_board_mkdir(user_id_t uid, const char *name, const char *descr);
extern bool fav_board_rename(user_id_t uid, int id, const char *name, const char *descr);
extern bool fav_board_rmdir(user_id_t uid, int id);
extern bool fav_board_rm(user_id_t uid, int id);
extern bool fav_board_mv(user_id_t uid, int id, int parent);

extern const char *currboard;
extern board_t *currbp;
extern void change_board(board_t *bp);
extern bool board_is_junk(const board_t *bp);
extern bool is_board_dir(const board_t *bp);

extern void board_complete(int row, const char *prompt, char *name, size_t size, int mode);

extern int tui_all_boards(const char *cmd);
extern int tui_unread_boards(const char *cmd);
extern int tui_read_sector(const char *cmd);
extern int tui_favorite_boards(const char *cmd);

#endif // FB_BOARD_H
