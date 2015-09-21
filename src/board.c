#include "bbs.h"
#include "record.h"
#include "fbbs/autocomplete.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct {
	board_t board;
	int folder;
	int sector;
	int online;
} board_extra_t;

typedef struct {
	board_extra_t *boards;      ///< Array of boards.
	board_extra_t **indices;
	comparator_t cmp;     ///< Compare function pointer.
	int count;            ///< Number of boards indexed.
	int bcount;           ///< Number of boards loaded.
	int sector;
	int parent;
	int *zapbuf;          ///< Subscribing record.
	bool yank;            ///< True if hide unsubscribed boards.
	bool newflag;         ///< True if jump to unread board.
	bool favorite;        ///< True if reading favorite boards.
	bool skip_reload;     ///< True if reload can be skipped.
	int copy_bid;         ///< Copy/paste buffer.
} board_list_t;

static int load_zapbuf(board_list_t *bl)
{
	if (!bl->zapbuf)
		bl->zapbuf = malloc(sizeof(*bl->zapbuf) * MAXBOARD);
	else
		return 0;

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");

	int n = 0;
	int fd = open(file, O_RDONLY, 0600);
	if (fd >= 0) {
		n = read(fd, bl->zapbuf, sizeof(*bl->zapbuf) * MAXBOARD);
		file_close(fd);
	}

	for (int i = n / sizeof(*bl->zapbuf); i < MAXBOARD; ++i) {
		bl->zapbuf[i] = 1;
	}
	return 0;
}

static int save_zapbuf(const board_list_t *bl)
{
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");
	int fd = open(file, O_WRONLY | O_CREAT, 0600);
	if (fd >= 0) {
		file_write(fd, bl->zapbuf, sizeof(*bl->zapbuf) * MAXBOARD);
		file_close(fd);
		return 0;
	}
	return -1;
}

static ac_list *build_board_ac_list(int mode)
{
	ac_list *acl = ac_list_new();
	if (!acl)
		return NULL;

	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	if (res) {
		for (int i = 0; i < db_res_rows(res); ++i) {
			board_t board;
			res_to_board(res, i, &board);

			if (board.flag & BOARD_FLAG_DIR) {
				if (mode == AC_LIST_BOARDS_ONLY)
					continue;
			} else {
				if (mode == AC_LIST_DIR_ONLY)
					continue;
			}

			if (has_read_perm(&board)) {
				if (board.name[0] & 0x80) {
					GBK_BUFFER(name, BOARD_NAME_LEN);
					convert_u2g(board.name, gbk_name);
					ac_list_add(acl, gbk_name);
				} else {
					ac_list_add(acl, board.name);
				}
			}
		}
	}
	db_clear(res);

	return acl;
}

void board_complete(int row, const char *prompt, char *name, size_t size, int mode)
{
	ac_list *acl = build_board_ac_list(mode);
	if (!acl)
		return;

	screen_move(row, 0);
	autocomplete(acl, prompt, name, size);

	ac_list_free(acl);
}

static int tui_favorite_add(tui_list_t *tl)
{
	if (!HAS_PERM(PERM_LOGIN))
		return DONOTHING;

	board_list_t *bl = tl->data;

	if (bl->favorite) {
		if (bl->count >= FAV_BOARD_LIMIT) {
			//% 收藏夹已满
			presskeyfor("\xca\xd5\xb2\xd8\xbc\xd0\xd2\xd1\xc2\xfa", -1);
			return MINIUPDATE;
		}

		GBK_UTF8_BUFFER(name, BOARD_NAME_LEN / 2);
		board_complete(1, "输入讨论区名 (按空白键自动搜寻): ",
				gbk_name, sizeof(gbk_name), AC_LIST_BOARDS_AND_DIR);

		if (gbk_name[0] & 0x80)
			convert_g2u(gbk_name, utf8_name);
		else
			strlcpy(utf8_name, gbk_name, sizeof(utf8_name));

		int parent = FAV_BOARD_ROOT_FOLDER;
		if (bl->favorite)
			parent = bl->parent;
		if (fav_board_add(session_get_user_id(), utf8_name, 0, parent,
					&currentuser))
			tl->valid = false;
		return FULLUPDATE;
	} else {
		const char *bname = bl->indices[tl->cur]->board.name;
		char buf[STRLEN];
		//% snprintf(buf, sizeof(buf), "您确定要添加 %s 到收藏夹吗?", bname);
		snprintf(buf, sizeof(buf), "\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xcc\xed\xbc\xd3 %s \xb5\xbd\xca\xd5\xb2\xd8\xbc\xd0\xc2\xf0?", bname);
		if (askyn(buf, false, true)) {
			fav_board_add(session_get_user_id(), bname, 0,
					FAV_BOARD_ROOT_FOLDER, &currentuser);
		}
		return MINIUPDATE;
	}
}

static int tui_favorite_copy(tui_list_t *tl)
{
	board_list_t *bl = tl->data;

	if (!HAS_PERM(PERM_LOGIN) || !bl->favorite)
		return DONOTHING;

	board_t *bp = &(bl->indices[tl->cur]->board);

	if (bp->flag & BOARD_CUSTOM_FLAG)
		return DONOTHING;

	bl->copy_bid = bp->id;
	//% 版面已剪切 请按P粘贴
	presskeyfor("\xb0\xe6\xc3\xe6\xd2\xd1\xbc\xf4\xc7\xd0 \xc7\xeb\xb0\xb4P\xd5\xb3\xcc\xf9", -1);
	return MINIUPDATE;
}

static int tui_favorite_paste(tui_list_t *tl)
{
	board_list_t *bl = tl->data;

	if (!HAS_PERM(PERM_LOGIN) || !bl->favorite)
		return DONOTHING;

	if (fav_board_mv(session_get_user_id(), bl->copy_bid, bl->parent)) {
		tl->valid = false;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int tui_favorite_mkdir(tui_list_t *tl)
{
	board_list_t *bl = tl->data;

	if (!HAS_PERM(PERM_LOGIN) || !bl->favorite
			|| (bl->parent != FAV_BOARD_ROOT_FOLDER))
		return DONOTHING;

	if (bl->count >= FAV_BOARD_LIMIT) {
		//% 收藏夹已满
		presskeyfor("\xca\xd5\xb2\xd8\xbc\xd0\xd2\xd1\xc2\xfa", -1);
		return MINIUPDATE;
	}

	GBK_UTF8_BUFFER(name, BOARD_NAME_LEN);
	GBK_UTF8_BUFFER(descr, BOARD_DESCR_CCHARS);

	//% getdata(-1, 0, "创建自定义目录: ", gbk_name, BOARD_NAME_LEN + 1,
	getdata(-1, 0, "\xb4\xb4\xbd\xa8\xd7\xd4\xb6\xa8\xd2\xe5\xc4\xbf\xc2\xbc: ", gbk_name, BOARD_NAME_LEN + 1,
			DOECHO, YEA);
	if (gbk_name[0] != '\0') {
		//% strlcpy(gbk_descr, "自定义目录", sizeof(gbk_descr));
		strlcpy(gbk_descr, "\xd7\xd4\xb6\xa8\xd2\xe5\xc4\xbf\xc2\xbc", sizeof(gbk_descr));
		//% getdata(-1, 0, "自定义目录描述: ", gbk_descr,
		getdata(-1, 0, "\xd7\xd4\xb6\xa8\xd2\xe5\xc4\xbf\xc2\xbc\xc3\xe8\xca\xf6: ", gbk_descr,
				sizeof(gbk_descr), DOECHO, NA);

		convert_g2u(gbk_name, utf8_name);
		convert_g2u(gbk_descr, utf8_descr);

		if (fav_board_mkdir(session_get_user_id(), utf8_name, utf8_descr)) {
			tl->valid = false;
			return PARTUPDATE;
		}
	}
	return MINIUPDATE;
}

static int tui_favorite_rename(tui_list_t *tl)
{
	board_list_t *bl = tl->data;

	board_t *bp = &(bl->indices[tl->cur]->board);

	if (!HAS_PERM(PERM_LOGIN) || !bl->favorite
			|| !(bp->flag & BOARD_CUSTOM_FLAG))
		return DONOTHING;

	GBK_UTF8_BUFFER(name, BOARD_NAME_LEN);
	GBK_UTF8_BUFFER(descr, BOARD_DESCR_CCHARS);

	strlcpy(gbk_name, bp->name, sizeof(gbk_name));
	//% getdata(-1, 0, "修改自定义目录名: ", gbk_name, BOARD_NAME_LEN,
	getdata(-1, 0, "\xd0\xde\xb8\xc4\xd7\xd4\xb6\xa8\xd2\xe5\xc4\xbf\xc2\xbc\xc3\xfb: ", gbk_name, BOARD_NAME_LEN,
			DOECHO, NA);
	if (gbk_name[0] != '\0' && !streq(gbk_name, bp->name)) {
		strlcpy(gbk_descr, bp->descr, sizeof(gbk_descr));
		//% getdata(-1, 0, "自定义目录描述: ", gbk_descr,
		getdata(-1, 0, "\xd7\xd4\xb6\xa8\xd2\xe5\xc4\xbf\xc2\xbc\xc3\xe8\xca\xf6: ", gbk_descr,
				BOARD_DESCR_CCHARS, DOECHO, NA);

		convert_g2u(gbk_name, utf8_name);
		convert_g2u(gbk_descr, utf8_descr);

		if (fav_board_rename(session_get_user_id(), bp->id, utf8_name,
					utf8_descr)) {
			strlcpy(bp->name, gbk_name, sizeof(bp->name));
			strlcpy(bp->descr, gbk_descr, sizeof(bp->descr));
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int tui_favorite_rm(tui_list_t *tl)
{
	board_list_t *bl = tl->data;
	board_t *bp = &(bl->indices[tl->cur]->board);

	if (bl->favorite) {
		char buf[STRLEN];
		//% snprintf(buf, sizeof(buf), "要把 %s 从收藏夹中去掉？", bp->name);
		snprintf(buf, sizeof(buf), "\xd2\xaa\xb0\xd1 %s \xb4\xd3\xca\xd5\xb2\xd8\xbc\xd0\xd6\xd0\xc8\xa5\xb5\xf4\xa3\xbf", bp->name);

		if (askyn(buf, false, true)) {
			int ok;
			if (bp->flag & BOARD_CUSTOM_FLAG)
				ok = fav_board_rmdir(session_get_user_id(), bp->id);
			else
				ok = fav_board_rm(session_get_user_id(), bp->id);
			if (ok)
				tl->valid = false;
			return PARTUPDATE;
		}
		return MINIUPDATE;
	}
	return DONOTHING;
}

static bool check_newpost(board_t *board)
{
	return brc_board_unread(currentuser.userid, board->name, board->id);
}

static void res_to_board_array(board_list_t *bl, db_res_t *r1, db_res_t *r2)
{
	int rows = db_res_rows(r1) + db_res_rows(r2);
	bl->boards = malloc(sizeof(*bl->boards) * rows);

	bl->bcount = 0;

	for (int i = 0; i < db_res_rows(r1); ++i) {
		board_extra_t *e = bl->boards + bl->bcount;
		board_t *board = &e->board;

		res_to_board(r1, i, board);
		board_to_gbk(board);

		e->folder = bl->favorite ? db_get_integer(r1, i, 8) : 0;
		e->sector = bl->favorite ? db_get_integer(r1, i, 9) : 0;

		if (has_read_perm(board))
			++bl->bcount;
	}

	if (r2) {
		for (int i = 0; i < db_res_rows(r2); ++i) {
			board_extra_t *e = bl->boards + bl->bcount;
			board_t *board = &e->board;

			board->id = db_get_integer(r2, i, 0);
			convert_u2g(db_get_value(r2, i, 1), board->name);
			convert_u2g(db_get_value(r2, i, 2), board->descr);
			//% strlcpy(board->categ, "收藏", sizeof(board->categ));
			strlcpy(board->categ, "\xca\xd5\xb2\xd8", sizeof(board->categ));

			board->flag = BOARD_CUSTOM_FLAG | BOARD_FLAG_DIR;
			e->folder = FAV_BOARD_ROOT_FOLDER;
			e->sector = 0;

			++bl->bcount;
		}
	}
}

static void load_favorite_boards(board_list_t *bl)
{
	db_res_t *r1 = db_query("SELECT "BOARD_BASE_FIELDS", f.folder, b.sector "
			"FROM "BOARD_BASE_TABLES" JOIN fav_boards f ON b.id = f.board "
			"WHERE f.user_id = %"DBIdUID, session_get_user_id());
	db_res_t *r2 = db_query("SELECT id, name, descr FROM fav_board_folders "
			"WHERE user_id = %"DBIdUID, session_get_user_id());

	res_to_board_array(bl, r1, r2);

	db_clear(r1);
	db_clear(r2);
}

static void load_boards(board_list_t *bl)
{
	db_res_t *res;
	if (bl->sector) {
		res = db_query(BOARD_SELECT_QUERY_BASE
				"WHERE b.sector = %d", bl->sector);
	} else if (bl->parent) {
		res = db_query(BOARD_SELECT_QUERY_BASE
				"WHERE b.parent = %d", bl->parent);
	} else {
		res = db_query(BOARD_SELECT_QUERY_BASE);
	}
	res_to_board_array(bl, res, NULL);
	db_clear(res);
}

static void index_favorite_boards(board_list_t *bl)
{
	if (!bl->parent)
		bl->parent = FAV_BOARD_ROOT_FOLDER;

	bl->count = 0;

	for (int i = 0; i < bl->bcount; ++i) {
		board_extra_t *p = bl->boards + i;
		if (p->folder == bl->parent)
			bl->indices[bl->count++] = p;
	}
	qsort(bl->indices, bl->count, sizeof(*bl->indices), bl->cmp);
}

static void index_boards(board_list_t *bl)
{
	bl->count = 0;
	for (int i = 0; i < bl->bcount; ++i) {
		if (!bl->yank || bl->zapbuf[bl->boards[i].board.id])
		bl->indices[bl->count++] = bl->boards + i;
	}
	qsort(bl->indices, bl->count, sizeof(*bl->indices), bl->cmp);
}

static void jump_to_first_unread(tui_list_t *tl)
{
	board_list_t *bl = tl->data;
	if (bl->newflag) {
		int i;
		for (i = tl->cur; i < tl->all; ++i) {
			board_t *bp = &(bl->indices[i]->board);
			if (!(bp->flag & BOARD_FLAG_DIR) && check_newpost(bp))
				break;
		}
		if (i < tl->all)
			tl->cur = i;
	}
}

static void load_online_data(board_list_t *bl)
{
	for (int i = 0; i < bl->bcount; ++i) {
		board_extra_t *b = bl->boards + i;
		if (b->board.flag & BOARD_FLAG_DIR)
			b->online = 0;
		else
			b->online = session_count_online_board(b->board.id);
	}
}

static tui_list_loader_t board_list_load(tui_list_t *tl)
{
	board_list_t *bl = tl->data;
	bool skip = bl->skip_reload && tl->valid;
	if (!skip) {
		free(bl->indices);
		free(bl->boards);
	}

	if (bl->favorite) {
		if (!skip)
			load_favorite_boards(bl);
	} else {
		load_boards(bl);
	}

	load_online_data(bl);

	if (bl->bcount) {
		if (!skip)
			bl->indices = malloc(sizeof(*bl->indices) * bl->bcount);
		if (bl->favorite)
			index_favorite_boards(bl);
		else
			index_boards(bl);
	}

	tl->all = bl->count;
	jump_to_first_unread(tl);
	return 0;
}

#if 0
static int search_board(const choose_board_t *cbrd, int *num)
{
	static int i = 0, find = YEA;
	static char bname[STRLEN];
	int n, ch, tmpn = NA;

	if (find == YEA) {
		bzero(bname, sizeof (bname));
		find = NA;
		i = 0;
	}

	while (1) {
		screen_move(-1, 0);
		clrtoeol();
		//% prints("请输入要查找的版面名称：%s", bname);
		prints("\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xb2\xe9\xd5\xd2\xb5\xc4\xb0\xe6\xc3\xe6\xc3\xfb\xb3\xc6\xa3\xba%s", bname);
		ch = egetch();

		if (isprint2(ch)) {
			bname[i++] = ch;
			for (n = 0; n < cbrd->num; n++) {
				if (!strncasecmp(cbrd->brds[n].name, bname, i)) {
					tmpn = YEA;
					*num = n;
					if (!strcmp(cbrd->brds[n].name, bname))
						//% return 1 /* 找到类似的版，画面重画
						return 1 /* \xd5\xd2\xb5\xbd\xc0\xe0\xcb\xc6\xb5\xc4\xb0\xe6\xa3\xac\xbb\xad\xc3\xe6\xd6\xd8\xbb\xad
						 */;
				}
			}
			if (tmpn)
				return 1;
			if (find == NA) {
				bname[--i] = '\0';
			}
			continue;
		} else if (ch == Ctrl('H') || ch == KEY_LEFT || ch == KEY_DEL
				|| ch == '\177') {
			i--;
			if (i < 0) {
				find = YEA;
				break;
			} else {
				bname[i] = '\0';
				continue;
			}
		} else if (ch == '\t') {
			find = YEA;
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_RIGHT) {
			find = YEA;
			break;
		}
		bell(1);
	}
	if (find) {
		screen_move(-1, 0);
		clrtoeol();
		//% return 2 /* 结束了 */;
		return 2 /* \xbd\xe1\xca\xf8\xc1\xcb */;
	}
	return 1;
}
#endif

static bool is_zapped(board_list_t *bl, board_t *board)
{
	return !bl->zapbuf[board->id] && !(board->flag & BOARD_FLAG_NOZAP);
}

static char property(board_t *board)
{
	if (board->flag & BOARD_FLAG_DIR) {
		if (board->perm)
			return 'r';
	} else {
		if (board->flag & BOARD_FLAG_NOZAP)
			return 'z';
		else if (board->flag & BOARD_FLAG_POST)
			return 'p';
		else if (board->flag & BOARD_FLAG_NOREPLY)
			return 'x';
		else if (board->perm)
			return 'r';
	}
	return ' ';
}

static tui_list_display_t board_list_display(tui_list_t *tl, int n)
{
	board_list_t *bl = tl->data;
	board_extra_t *be = bl->indices[n];
	board_t *board = &be->board;

	if (!bl->newflag)
		screen_printf(" %5d", n + 1);
	else if (board->flag & BOARD_FLAG_DIR)
		screen_printf("  目录");
	else {
		int count = post_get_board_count(board->id);
		if (count < 100000) {
			screen_printf(" %5d", count);
		} else {
			screen_printf(" %4dk", count / 1000);
		}
	}

	prints(" ");
	if (board->flag & BOARD_FLAG_DIR) {
		screen_printf("＋");
	} else {
		bool unread = check_newpost(board);
		screen_printf(unread ? "◆" : "◇");
	}

	char descr[24];
	strlcpy(descr, board->descr, sizeof(descr));
	ellipsis(descr, 20);

	prints("%c%-17s %s%s[%4s] %-20s %c ",
			(is_zapped(bl, board)) ? '*' : ' ', board->name,
			(board->flag & BOARD_FLAG_VOTE) ? "\033[1;31mV\033[m" : " ",
			(board->flag & BOARD_FLAG_CLUB) ? (board->flag & BOARD_FLAG_READ)
				? "\033[1;31mc\033[m" : "\033[1;33mc\033[m" : " ",
			board->categ, descr, HAS_PERM(PERM_POST) ? property(board) : ' ');

	if (board->flag & BOARD_FLAG_DIR) {
		screen_printf("[目录]\n");
	} else {
		char bms[IDLEN + 1];
		strlcpy(bms, board->bms, sizeof(bms));
		prints("%-12s  %4d\n",
				//% bms[0] <= ' ' ? "诚征版主中" : strtok(bms, " "),
				bms[0] <= ' ' ? "\xb3\xcf\xd5\xf7\xb0\xe6\xd6\xf7\xd6\xd0" : strtok(bms, " "),
				be->online);
	}

	return 0;
}

static int board_cmp_flag(const void *p1, const void *p2)
{
	const board_extra_t *b1 = *(const board_extra_t **) p1;
	const board_extra_t *b2 = *(const board_extra_t **) p2;
	return strcasecmp(b1->board.name, b2->board.name);
}

static int board_cmp_online(const void *p1, const void *p2)
{
	const board_extra_t *b1 = *(const board_extra_t **) p1;
	const board_extra_t *b2 = *(const board_extra_t **) p2;
	return b2->online - b1->online;
}

static int board_cmp_default(const void *p1, const void *p2)
{
	const board_extra_t *b1 = *(const board_extra_t **) p1;
	const board_extra_t *b2 = *(const board_extra_t **) p2;
	int diff = b1->sector - b2->sector;
	if (!diff)
		diff = strcasecmp(b1->board.categ, b2->board.categ);
	if (!diff)
		diff = strcasecmp(b1->board.name, b2->board.name);
	return diff;
}

static int show_board_info(board_t *board)
{
	if (!board || board->flag & BOARD_CUSTOM_FLAG)
		return DONOTHING;

	board_t parent;
	if (board->parent) {
		get_board_by_bid(board->parent, &parent);
		if (parent.name[0] & 0x80)
			board_to_gbk(&parent);
	}

	screen_clear();
	//% prints("版面详细信息:\n\n");
	prints("\xb0\xe6\xc3\xe6\xcf\xea\xcf\xb8\xd0\xc5\xcf\xa2:\n\n");
	prints("ID      :     %d\n", board->id);
	//% prints("英文名称:     %s\n", board->name);
	prints("\xd3\xa2\xce\xc4\xc3\xfb\xb3\xc6:     %s\n", board->name);
	//% prints("中文名称:     %s\n", board->descr);
	prints("\xd6\xd0\xce\xc4\xc3\xfb\xb3\xc6:     %s\n", board->descr);
	//% prints("版    主:     %s\n", board->bms);
	prints("\xb0\xe6    \xd6\xf7:     %s\n", board->bms);
	//% prints("所属讨论区:   %s\n", board->parent ? parent.name : "无");
	prints("\xcb\xf9\xca\xf4\xcc\xd6\xc2\xdb\xc7\xf8:   %s\n", board->parent ? parent.name : "\xce\xde");
	//% prints("是否目录:     %s\n", (board->flag & BOARD_FLAG_DIR) ? "目录" : "版面");
	prints("\xca\xc7\xb7\xf1\xc4\xbf\xc2\xbc:     %s\n", (board->flag & BOARD_FLAG_DIR) ? "\xc4\xbf\xc2\xbc" : "\xb0\xe6\xc3\xe6");
	//% prints("可以 ZAP:     %s\n", (board->flag & BOARD_FLAG_NOZAP) ? "不可以" : "可以");
	prints("\xbf\xc9\xd2\xd4 ZAP:     %s\n", (board->flag & BOARD_FLAG_NOZAP) ? "\xb2\xbb\xbf\xc9\xd2\xd4" : "\xbf\xc9\xd2\xd4");

	if (!(board->flag & BOARD_FLAG_DIR)) {
		//% "在线人数:     %d 人\n"
		prints("\xd4\xda\xcf\xdf\xc8\xcb\xca\xfd:     %d \xc8\xcb\n", session_count_online_board(board->id));
		//% prints("文 章 数:     %s\n", (board->flag & BOARD_FLAG_JUNK) ? "不计算" : "计算");
		prints("\xce\xc4 \xd5\xc2 \xca\xfd:     %s\n", (board->flag & BOARD_FLAG_JUNK) ? "\xb2\xbb\xbc\xc6\xcb\xe3" : "\xbc\xc6\xcb\xe3");
		//% prints("可以回复:     %s\n", (board->flag & BOARD_FLAG_NOREPLY) ? "不可以" : "可以");
		prints("\xbf\xc9\xd2\xd4\xbb\xd8\xb8\xb4:     %s\n", (board->flag & BOARD_FLAG_NOREPLY) ? "\xb2\xbb\xbf\xc9\xd2\xd4" : "\xbf\xc9\xd2\xd4");
		//% prints("匿 名 版:     %s\n", (board->flag & BOARD_FLAG_ANONY) ? "是" : "否");
		prints("\xc4\xe4 \xc3\xfb \xb0\xe6:     %s\n", (board->flag & BOARD_FLAG_ANONY) ? "\xca\xc7" : "\xb7\xf1");
#ifdef ENABLE_PREFIX
		//% prints ("强制前缀:     %s\n", (board->flag & BOARD_FLAG_PREFIX) ? "必须" : "不必");
		prints ("\xc7\xbf\xd6\xc6\xc7\xb0\xd7\xba:     %s\n", (board->flag & BOARD_FLAG_PREFIX) ? "\xb1\xd8\xd0\xeb" : "\xb2\xbb\xb1\xd8");
#endif
		//% prints("俱 乐 部:     %s\n", (board->flag & BOARD_FLAG_CLUB) ?
		prints("\xbe\xe3 \xc0\xd6 \xb2\xbf:     %s\n", (board->flag & BOARD_FLAG_CLUB) ?
				//% (board->flag & BOARD_FLAG_READ) ? "读限制俱乐部" : "普通俱乐部"
				(board->flag & BOARD_FLAG_READ) ? "\xb6\xc1\xcf\xde\xd6\xc6\xbe\xe3\xc0\xd6\xb2\xbf" : "\xc6\xd5\xcd\xa8\xbe\xe3\xc0\xd6\xb2\xbf"
				//% : "非俱乐部");
				: "\xb7\xc7\xbe\xe3\xc0\xd6\xb2\xbf");
		//% prints("读写限制:     %s\n", (board->flag & BOARD_FLAG_POST) ? "限制发文" :
		prints("\xb6\xc1\xd0\xb4\xcf\xde\xd6\xc6:     %s\n", (board->flag & BOARD_FLAG_POST) ? "\xcf\xde\xd6\xc6\xb7\xa2\xce\xc4" :
				//% (board->perm == 0) ? "没有限制" : "限制阅读");
				(board->perm == 0) ? "\xc3\xbb\xd3\xd0\xcf\xde\xd6\xc6" : "\xcf\xde\xd6\xc6\xd4\xc4\xb6\xc1");
	}
	if (HAS_PERM(PERM_SPECIAL0) && board->perm) {
		//% prints("权    限:     ");
		prints("\xc8\xa8    \xcf\xde:     ");
		char secu[] = "ltmprbBOCAMURS#@XLEast0123456789";
		for (int i = 0; i < 32; i++) {
			if (!(board->perm & (1 << i)))
				secu[i] = '-';
		}
		//% prints("\n权 限 位:     %s\n", secu);
		prints("\n\xc8\xa8 \xcf\xde \xce\xbb:     %s\n", secu);
	}

	//% prints("URL 地址:     http://"BBSHOST"/bbs/doc?bid=%d\n", board->id);
	prints("URL \xb5\xd8\xd6\xb7:     http://"BBSHOST"/bbs/doc?bid=%d\n", board->id);
	pressanykey();
	return FULLUPDATE;
}

int show_hotspot(void)
{
	char ans[2];
	//% getdata(-1, 0, "您选择? (1) 本日十大  (2) 系统热点 [1]",
	getdata(-1, 0, "\xc4\xfa\xd1\xa1\xd4\xf1? (1) \xb1\xbe\xc8\xd5\xca\xae\xb4\xf3  (2) \xcf\xb5\xcd\xb3\xc8\xc8\xb5\xe3 [1]",
			ans, 2, DOECHO, YEA);
	if (ans[0] == '2')
		show_help("etc/hotspot");
	else
		show_help("0Announce/bbslist/day");
	return FULLUPDATE;
}

static int tui_board_list(board_list_t *bl);

static int read_board(tui_list_t *tl)
{
	board_list_t *bl = tl->data;
	board_t *bp = &(bl->indices[tl->cur]->board);

	if (bp->flag & BOARD_FLAG_DIR) {
		if (bp->flag & BOARD_CUSTOM_FLAG) {
			bl->skip_reload = true;
			bl->parent = bp->id;
			tui_board_list(bl);

			bl->skip_reload = false;
			bl->parent = FAV_BOARD_ROOT_FOLDER;
			index_favorite_boards(bl);
			return PARTUPDATE;
		} else {
			board_list_t nl = {
				.skip_reload = false,
				.favorite = false,
				.newflag = bl->newflag,
				.cmp = bl->cmp,
				.parent = bp->id,
				.sector = 0,
				.zapbuf = bl->zapbuf,
				.boards = NULL,
				.indices = NULL,
				.yank = bl->yank,
			};
			tui_board_list(&nl);
			return PARTUPDATE;
		}
	} else {
		brc_init(currentuser.userid, bp->name);

		board_t orig_board = *bp;
		convert_g2u(bp->descr, orig_board.descr);
		convert_g2u(bp->name, orig_board.name);
		change_board(&orig_board);

		memcpy(currBM, bp->bms, BM_LEN - 1);

		board_read();

		brc_zapbuf(bl->zapbuf + bp->id);
		currBM[0] = '\0';
	}

	return FULLUPDATE;
}

static int sort_boards(tui_list_t *tl)
{
	board_list_t *bl = tl->data;

	if (currentuser.flags[0] & BRDSORT_FLAG) {
		currentuser.flags[0] ^= BRDSORT_FLAG;
		currentuser.flags[0] |= BRDSORT_ONLINE;
		bl->cmp = board_cmp_online;
	} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
		currentuser.flags[0] ^= BRDSORT_ONLINE;
		bl->cmp = board_cmp_default;
	} else {
		currentuser.flags[0] |= BRDSORT_FLAG;
		bl->cmp = board_cmp_flag;
	}
	
	qsort(bl->indices, tl->all, sizeof(*bl->indices), bl->cmp);

	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	return FULLUPDATE;
}

static int board_list_init(board_list_t *bl)
{
	memset(bl, 0, sizeof(*bl));

	if (!streq(currentuser.userid, "guest"))
		bl->yank = true;

	char flag = currentuser.flags[0];
	if (flag & BRDSORT_FLAG)
		bl->cmp = board_cmp_flag;
	else if (flag & BRDSORT_ONLINE)
		bl->cmp = board_cmp_online;
	else if (flag & BRDSORT_UDEF)
		bl->cmp = board_cmp_default;
	else if (flag & BRDSORT_UPDATE)
		bl->cmp = board_cmp_default;
	else
		bl->cmp = board_cmp_default;

	return 0;
}

static tui_list_title_t board_list_title(tui_list_t *tl)
{
	const char *sort = "分类";
	char flag = currentuser.flags[0];
	if (flag & BRDSORT_FLAG) {
		sort = "字母";
	} else if (flag & BRDSORT_ONLINE) {
		sort = "在线";
	} else if (flag & BRDSORT_UDEF) {
		sort = "自定";
	} else if (flag & BRDSORT_UPDATE) {
		sort = "更新";
	}

	char buf[32];
	snprintf(buf, sizeof(buf), "[讨论区列表] [%s]", sort);
	tui_header_line(buf, true);

	screen_printf(" \033[m主选单[\033[1;32m←\033[m,\033[1;32me\033[m] 阅读"
			"[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 选择[\033[1;32m↑\033[m,"
			"\033[1;32m↓\033[m] 列出[\033[1;32my\033[m] 排序[\033[1;32ms"
			"\033[m] 搜寻[\033[1;32m/\033[m] 切换[\033[1;32mc\033[m] 求助"
			"[\033[1;32mh\033[m]\n"
			"\033[1;44;37m  编号 未 讨论区名称        V  类别  %-20s S 版  主"
			"        在线  \033[m\n", "中  文  叙  述");
}

static tui_list_handler_t board_list_handler(tui_list_t *tl, int key)
{
	board_list_t *bl = tl->data;
	bool st_changed = false, handled = true;

	switch (key) {
		case 'c':
			bl->newflag = !bl->newflag;
			return PARTUPDATE;
		case 'L':
			m_read();
			st_changed = true;
			break;
		case 'M':
			m_new();
			st_changed = true;
			break;
		case 'u':
			set_user_status(ST_QUERY);
			t_query(NULL);
			st_changed = true;
			break;
		case 'H':
			return show_hotspot();
		case 'l':
			msg_more();
			st_changed = true;
			break;
		case '!':
			save_zapbuf(bl);
			Goodbye();
			return -1;
		case 'h':
			show_help("help/boardreadhelp");
			break;
		case 'S':
			if (!HAS_PERM(PERM_TALK))
				return DONOTHING;
			s_msg();
			st_changed = true;
			break;
		case 'o':
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			show_online_followings();
			st_changed = true;
			break;
		case 'a':
			return tui_favorite_add(tl);
		case 'A':
			return tui_favorite_mkdir(tl);
		case Ctrl('T'):
			return tui_check_notice(NULL);
		default:
			handled = false;
	}

	if (tl->cur >= tl->all)
		return READ_AGAIN;

	board_t *board = &(bl->indices[tl->cur]->board);

	switch (key) {
		case '*':
			return show_board_info(board);
		case '/':
			// TODO: search.
			break;
		case 's':
			return sort_boards(tl);
		case 'y':
			if (bl->favorite)
				return DONOTHING;
			bl->yank = !bl->yank;
			index_boards(bl);
			tl->all = bl->count;
			return PARTUPDATE;
		case 'z':
			if (bl->favorite)
				return DONOTHING;
			if (HAS_PERM(PERM_LOGIN)
					&& !(board->flag & BOARD_FLAG_NOZAP)) {
				//% if (l->zapbuf[board->id] && !askyn("确实要隐藏吗?", NA, YEA))
				if (bl->zapbuf[board->id] && !askyn("\xc8\xb7\xca\xb5\xd2\xaa\xd2\xfe\xb2\xd8\xc2\xf0?", NA, YEA))
					return MINIUPDATE;
				bl->zapbuf[board->id] = !bl->zapbuf[board->id];
			}
			index_boards(bl);
			tl->all = bl->count;
			return PARTUPDATE;
		case 'T':
			return tui_favorite_rename(tl);
		case 'd':
			return tui_favorite_rm(tl);
		case 'C':
			return tui_favorite_copy(tl);
		case 'P':
			return tui_favorite_paste(tl);
		case '\r':
		case '\n':
		case KEY_RIGHT:
			read_board(tl);
			jump_to_first_unread(tl);
			st_changed = true;
			return FULLUPDATE;
		default:
			if (!handled)
				return READ_AGAIN;
	}

	if (st_changed)
		set_user_status(bl->newflag ? ST_READNEW : ST_READBRD);
	return FULLUPDATE;
}

static int tui_board_list(board_list_t *bl)
{
	bool alloc_zapbuf = !bl->zapbuf;
	if (alloc_zapbuf)
		load_zapbuf(bl);

	tui_list_t tl = {
		.data = bl,
		.loader = board_list_load,
		.title = board_list_title,
		.display = board_list_display,
		.handler = board_list_handler,
		.query = NULL,
	};

	set_user_status(bl->newflag ? ST_READNEW : ST_READBRD);

	tui_list(&tl);
	
	if (!bl->skip_reload) {
		free(bl->boards);
		free(bl->indices);
	}

	if (alloc_zapbuf) {
		save_zapbuf(bl);
		free(bl->zapbuf);
	}

	screen_clear();
	return 0;
}

int tui_all_boards(const char *cmd)
{
	board_list_t bl;
	board_list_init(&bl);
	return tui_board_list(&bl);
}

int tui_unread_boards(const char *cmd)
{
	board_list_t bl;
	board_list_init(&bl);
	bl.newflag = true;
	return tui_board_list(&bl);
}

int tui_read_sector(const char *cmd)
{
	board_list_t bl;
	board_list_init(&bl);
	bl.newflag = true;

#ifdef FDQUAN
	char s[] = "a";
	s[0] = *cmd;
	db_res_t *res = db_query("SELECT id FROM board_sectors"
			" WHERE name = %s", s);
	if (res && db_res_rows(res) > 0)
		bl.sector = db_get_integer(res, 0, 0);
	db_clear(res);
#else
	bl.sector = *cmd;
#endif
	return tui_board_list(&bl);
}

int tui_favorite_boards(const char *cmd)
{
	board_list_t bl;
	board_list_init(&bl);

	bl.favorite = true;
	bl.newflag = true;

	return tui_board_list(&bl);
}
