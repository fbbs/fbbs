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

static int load_zapbuf(board_list_t *l)
{
	if (!l->zapbuf)
		l->zapbuf = malloc(sizeof(*l->zapbuf) * MAXBOARD);
	else
		return 0;

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");

	int n = 0;
	int fd = open(file, O_RDONLY, 0600);
	if (fd >= 0) {
		n = read(fd, l->zapbuf, sizeof(*l->zapbuf) * MAXBOARD);
		file_close(fd);
	}

	for (int i = n / sizeof(*l->zapbuf); i < MAXBOARD; ++i) {
		l->zapbuf[i] = 1;
	}
	return 0;
}

static int save_zapbuf(const board_list_t *l)
{
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");
	int fd = open(file, O_WRONLY | O_CREAT, 0600);
	if (fd >= 0) {
		file_write(fd, l->zapbuf, sizeof(*l->zapbuf) * MAXBOARD);
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

			if (board.flag & BOARD_DIR_FLAG) {
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

	move(row, 0);
	autocomplete(acl, prompt, name, size);

	ac_list_free(acl);
}

static int tui_favorite_add(tui_list_t *p)
{
	if (!HAS_PERM(PERM_LOGIN))
		return DONOTHING;

	board_list_t *l = p->data;

	if (l->favorite) {
		if (l->count >= FAV_BOARD_LIMIT) {
			//% 收藏夹已满
			presskeyfor("\xca\xd5\xb2\xd8\xbc\xd0\xd2\xd1\xc2\xfa", -1);
			return MINIUPDATE;
		}

		GBK_UTF8_BUFFER(name, BOARD_NAME_LEN / 2);
		//% board_complete(1, "输入讨论区名 (按空白键自动搜寻): ",
		board_complete(1, "\xca\xe4\xc8\xeb\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb (\xb0\xb4\xbf\xd5\xb0\xd7\xbc\xfc\xd7\xd4\xb6\xaf\xcb\xd1\xd1\xb0): ",
				gbk_name, sizeof(gbk_name), AC_LIST_BOARDS_AND_DIR);

		if (gbk_name[0] & 0x80)
			convert_g2u(gbk_name, utf8_name);
		else
			strlcpy(utf8_name, gbk_name, sizeof(utf8_name));

		int parent = FAV_BOARD_ROOT_FOLDER;
		if (l->favorite)
			parent = l->parent;
		if (fav_board_add(session.uid, utf8_name, 0, parent, &currentuser))
			p->valid = false;
		return FULLUPDATE;
	} else {
		const char *bname = l->indices[p->cur]->board.name;
		char buf[STRLEN];
		//% snprintf(buf, sizeof(buf), "您确定要添加 %s 到收藏夹吗?", bname);
		snprintf(buf, sizeof(buf), "\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xcc\xed\xbc\xd3 %s \xb5\xbd\xca\xd5\xb2\xd8\xbc\xd0\xc2\xf0?", bname);
		if (askyn(buf, false, true)) {
			fav_board_add(session.uid, bname, 0,
					FAV_BOARD_ROOT_FOLDER, &currentuser);
		}
		return MINIUPDATE;
	}
}

static int tui_favorite_copy(tui_list_t *p)
{
	board_list_t *l = p->data;

	if (!HAS_PERM(PERM_LOGIN) || !l->favorite)
		return DONOTHING;

	board_t *bp = &(l->indices[p->cur]->board);

	if (bp->flag & BOARD_CUSTOM_FLAG)
		return DONOTHING;

	l->copy_bid = bp->id;
	//% 版面已剪切 请按P粘贴
	presskeyfor("\xb0\xe6\xc3\xe6\xd2\xd1\xbc\xf4\xc7\xd0 \xc7\xeb\xb0\xb4P\xd5\xb3\xcc\xf9", -1);
	return MINIUPDATE;
}

static int tui_favorite_paste(tui_list_t *p)
{
	board_list_t *l = p->data;

	if (!HAS_PERM(PERM_LOGIN) || !l->favorite)
		return DONOTHING;

	if (fav_board_mv(session.uid, l->copy_bid, l->parent)) {
		p->valid = false;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int tui_favorite_mkdir(tui_list_t *p)
{
	board_list_t *l = p->data;

	if (!HAS_PERM(PERM_LOGIN) || !l->favorite
			|| (l->parent != FAV_BOARD_ROOT_FOLDER))
		return DONOTHING;

	if (l->count >= FAV_BOARD_LIMIT) {
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

		if (fav_board_mkdir(session.uid, utf8_name, utf8_descr)) {
			p->valid = false;
			return PARTUPDATE;
		}
	}
	return MINIUPDATE;
}

static int tui_favorite_rename(tui_list_t *p)
{
	board_list_t *l = p->data;

	board_t *bp = &(l->indices[p->cur]->board);

	if (!HAS_PERM(PERM_LOGIN) || !l->favorite
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

		if (fav_board_rename(session.uid, bp->id, utf8_name, utf8_descr)) {
			strlcpy(bp->name, gbk_name, sizeof(bp->name));
			strlcpy(bp->descr, gbk_descr, sizeof(bp->descr));
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int tui_favorite_rm(tui_list_t *p)
{
	board_list_t *l = p->data;
	board_t *bp = &(l->indices[p->cur]->board);

	if (l->favorite) {
		char buf[STRLEN];
		//% snprintf(buf, sizeof(buf), "要把 %s 从收藏夹中去掉？", bp->name);
		snprintf(buf, sizeof(buf), "\xd2\xaa\xb0\xd1 %s \xb4\xd3\xca\xd5\xb2\xd8\xbc\xd0\xd6\xd0\xc8\xa5\xb5\xf4\xa3\xbf", bp->name);

		if (askyn(buf, false, true)) {
			int ok;
			if (bp->flag & BOARD_CUSTOM_FLAG)
				ok = fav_board_rmdir(session.uid, bp->id);
			else
				ok = fav_board_rm(session.uid, bp->id);
			if (ok)
				p->valid = false;
			return PARTUPDATE;
		}
		return MINIUPDATE;
	}
	return DONOTHING;
}

static bool check_newpost(board_t *board)
{
	if (!brc_init(currentuser.userid, board->name)) {
		return true;
	} else {
		if (brc_unread(get_last_post_time(board->id))) {
			return true;
		}
	}
	return false;
}

static void res_to_board_array(board_list_t *l, db_res_t *r1, db_res_t *r2)
{
	int rows = db_res_rows(r1) + db_res_rows(r2);
	l->boards = malloc(sizeof(*l->boards) * rows);

	l->bcount = 0;

	for (int i = 0; i < db_res_rows(r1); ++i) {
		board_extra_t *e = l->boards + l->bcount;
		board_t *board = &e->board;

		res_to_board(r1, i, board);
		board_to_gbk(board);

		e->folder = l->favorite ? db_get_integer(r1, i, 8) : 0;
		e->sector = l->favorite ? db_get_integer(r1, i, 9) : 0;

		if (has_read_perm(board))
			++l->bcount;
	}

	if (r2) {
		for (int i = 0; i < db_res_rows(r2); ++i) {
			board_extra_t *e = l->boards + l->bcount;
			board_t *board = &e->board;

			board->id = db_get_integer(r2, i, 0);
			convert_u2g(db_get_value(r2, i, 1), board->name);
			convert_u2g(db_get_value(r2, i, 2), board->descr);
			//% strlcpy(board->categ, "收藏", sizeof(board->categ));
			strlcpy(board->categ, "\xca\xd5\xb2\xd8", sizeof(board->categ));

			board->flag = BOARD_CUSTOM_FLAG | BOARD_DIR_FLAG;
			e->folder = FAV_BOARD_ROOT_FOLDER;
			e->sector = 0;

			++l->bcount;
		}
	}
}

static void load_favorite_boards(board_list_t *l)
{
	db_res_t *r1 = db_query("SELECT "BOARD_BASE_FIELDS", f.folder, b.sector "
			"FROM "BOARD_BASE_TABLES" JOIN fav_boards f ON b.id = f.board "
			"WHERE f.user_id = %"DBIdUID, session.uid);
	db_res_t *r2 = db_query("SELECT id, name, descr FROM fav_board_folders "
			"WHERE user_id = %"DBIdUID, session.uid);

	res_to_board_array(l, r1, r2);

	db_clear(r1);
	db_clear(r2);
}

static void load_boards(board_list_t *l)
{
	db_res_t *res;
	if (l->sector) {
		res = db_query(BOARD_SELECT_QUERY_BASE
				"WHERE b.sector = %d", l->sector);
	} else if (l->parent) {
		res = db_query(BOARD_SELECT_QUERY_BASE
				"WHERE b.parent = %d", l->parent);
	} else {
		res = db_query(BOARD_SELECT_QUERY_BASE);
	}
	res_to_board_array(l, res, NULL);
	db_clear(res);
}

static void index_favorite_boards(board_list_t *l)
{
	if (!l->parent)
		l->parent = FAV_BOARD_ROOT_FOLDER;

	l->count = 0;

	for (int i = 0; i < l->bcount; ++i) {
		board_extra_t *p = l->boards + i;
		if (p->folder == l->parent)
			l->indices[l->count++] = p;
	}
	qsort(l->indices, l->count, sizeof(*l->indices), l->cmp);
}

static void index_boards(board_list_t *l)
{
	l->count = 0;
	for (int i = 0; i < l->bcount; ++i) {
		if (!l->yank || l->zapbuf[l->boards[i].board.id])
		l->indices[l->count++] = l->boards + i;
	}
	qsort(l->indices, l->count, sizeof(*l->indices), l->cmp);
}

static void jump_to_first_unread(tui_list_t *p)
{
	board_list_t *l = p->data;
	if (l->newflag) {
		int i;
		for (i = p->cur; i < p->all; ++i) {
			board_t *bp = &(l->indices[i]->board);
			if (!(bp->flag & BOARD_DIR_FLAG) && check_newpost(bp))
				break;
		}
		if (i < p->all)
			p->cur = i;
	}
}

static tui_list_loader_t board_list_load(tui_list_t *p)
{
	board_list_t *l = p->data;
	bool skip = l->skip_reload && p->valid;
	if (!skip) {
		free(l->indices);
		free(l->boards);
	}

	if (l->favorite) {
		if (!skip)
			load_favorite_boards(l);
	} else {
		load_boards(l);
	}

	if (l->bcount) {
		if (!skip)
			l->indices = malloc(sizeof(*l->indices) * l->bcount);
		if (l->favorite)
			index_favorite_boards(l);
		else
			index_boards(l);
	}

	p->all = l->count;
	jump_to_first_unread(p);
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
		move(-1, 0);
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
		move(-1, 0);
		clrtoeol();
		//% return 2 /* 结束了 */;
		return 2 /* \xbd\xe1\xca\xf8\xc1\xcb */;
	}
	return 1;
}
#endif

// TODO: rewrite
static int unread_position(board_t *bp)
{
	struct fileheader fh;

	char file[HOMELEN];
	setbdir(file, currboard);
	int fd = open(file, O_RDONLY);
	if (fd < 0)
		return 0;

	int offset = offsetof(struct fileheader, filename);
	int total = lseek(fd, 0, SEEK_END) / sizeof(fh), num = total - 1;

	if (!brc_init(currentuser.userid, bp->name))
		return 0;

	if (brc_unread(get_last_post_time(bp->id))) {
		char filename[STRLEN];
		num = total - 1;
		int step = 4;
		while (num > 0) {
			lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
			if (read (fd, filename, STRLEN) <= 0 || !brc_unread_legacy(filename))
				break;
			num -= step;
			if (step < 32)
				step += step / 2;
		}
		if (num < 0)
			num = 0;
		while (num < total) {
			lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
			if (read (fd, filename, STRLEN) <= 0 || brc_unread_legacy(filename))
				break;
			num++;
		}
		close (fd);
	}
	if (num < 0)
		num = 0;
	return num;
}

static bool is_zapped(board_list_t *l, board_t *board)
{
	return !l->zapbuf[board->id] && !(board->flag & BOARD_NOZAP_FLAG);
}

static char property(board_t *board)
{
	if (board->flag & BOARD_DIR_FLAG) {
		if (board->perm)
			return 'r';
	} else {
		if (board->flag & BOARD_NOZAP_FLAG)
			return 'z';
		else if (board->flag & BOARD_POST_FLAG)
			return 'p';
		else if (board->flag & BOARD_NOREPLY_FLAG)
			return 'x';
		else if (board->perm)
			return 'r';
	}
	return ' ';
}

static tui_list_display_t board_list_display(tui_list_t *p, int n)
{
	board_list_t *l = p->data;
	board_t *board = &(l->indices[n]->board);

	if (!l->newflag)
		prints(" %5d", n + 1);
	else if (board->flag & BOARD_DIR_FLAG)
		//% prints("  目录");
		prints("  \xc4\xbf\xc2\xbc");
	else
		prints(" %5d", get_board_post_count(board->id));

	prints(" ");
	if (board->flag & BOARD_DIR_FLAG) {
		//% prints("＋");
		prints("\xa3\xab");
	} else {
		bool unread = check_newpost(board);
		//% prints(unread ? "◆" : "◇");
		prints(unread ? "\xa1\xf4" : "\xa1\xf3");
	}

	char descr[24];
	strlcpy(descr, board->descr, sizeof(descr));
	ellipsis(descr, 20);

	prints("%c%-17s %s%s[%4s] %-20s %c ",
			(is_zapped(l, board)) ? '*' : ' ', board->name,
			(board->flag & BOARD_VOTE_FLAG) ? "\033[1;31mV\033[m" : " ",
			(board->flag & BOARD_CLUB_FLAG) ? (board->flag & BOARD_READ_FLAG)
				? "\033[1;31mc\033[m" : "\033[1;33mc\033[m" : " ",
			board->categ, descr, HAS_PERM(PERM_POST) ? property(board) : ' ');

	if (board->flag & BOARD_DIR_FLAG) {
		//% prints("[目录]\n");
		prints("[\xc4\xbf\xc2\xbc]\n");
	} else {
		char bms[IDLEN + 1];
		strlcpy(bms, board->bms, sizeof(bms));
		prints("%-12s %4d\n",
				//% bms[0] <= ' ' ? "诚征版主中" : strtok(bms, " "),
				bms[0] <= ' ' ? "\xb3\xcf\xd5\xf7\xb0\xe6\xd6\xf7\xd6\xd0" : strtok(bms, " "),
				brdshm->bstatus[board->id].inboard);
	}

	return 0;
}

static int board_cmp_flag(const void *p1, const void *p2)
{
	board_t *b1 = *(board_t * const *)p1;
	board_t *b2 = *(board_t * const *)p2;
	return strcasecmp(b1->name, b2->name);
}

static int board_cmp_online(const void *p1, const void *p2)
{
	board_t *b1 = *(board_t * const *)p1;
	board_t *b2 = *(board_t * const *)p2;
	return brdshm->bstatus[b2->id].inboard - brdshm->bstatus[b1->id].inboard;
}

static int board_cmp_default(const void *p1, const void *p2)
{
	const board_extra_t *e1 = *(const board_extra_t **)p1;
	const board_extra_t *e2 = *(const board_extra_t **)p2;
	int diff = e1->sector - e2->sector;
	if (!diff)
		diff = strcasecmp(e1->board.categ, e2->board.categ);
	if (!diff)
		diff = strcasecmp(e1->board.name, e2->board.name);
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

	struct bstat *bs = getbstat(board->id);
	clear();
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
	//% prints("是否目录:     %s\n", (board->flag & BOARD_DIR_FLAG) ? "目录" : "版面");
	prints("\xca\xc7\xb7\xf1\xc4\xbf\xc2\xbc:     %s\n", (board->flag & BOARD_DIR_FLAG) ? "\xc4\xbf\xc2\xbc" : "\xb0\xe6\xc3\xe6");
	//% prints("可以 ZAP:     %s\n", (board->flag & BOARD_NOZAP_FLAG) ? "不可以" : "可以");
	prints("\xbf\xc9\xd2\xd4 ZAP:     %s\n", (board->flag & BOARD_NOZAP_FLAG) ? "\xb2\xbb\xbf\xc9\xd2\xd4" : "\xbf\xc9\xd2\xd4");

	if (!(board->flag & BOARD_DIR_FLAG)) {
		//% prints("在线人数:     %d 人\n", bs->inboard);
		prints("\xd4\xda\xcf\xdf\xc8\xcb\xca\xfd:     %d \xc8\xcb\n", bs->inboard);
		//% prints("文 章 数:     %s\n", (board->flag & BOARD_JUNK_FLAG) ? "不计算" : "计算");
		prints("\xce\xc4 \xd5\xc2 \xca\xfd:     %s\n", (board->flag & BOARD_JUNK_FLAG) ? "\xb2\xbb\xbc\xc6\xcb\xe3" : "\xbc\xc6\xcb\xe3");
		//% prints("可以回复:     %s\n", (board->flag & BOARD_NOREPLY_FLAG) ? "不可以" : "可以");
		prints("\xbf\xc9\xd2\xd4\xbb\xd8\xb8\xb4:     %s\n", (board->flag & BOARD_NOREPLY_FLAG) ? "\xb2\xbb\xbf\xc9\xd2\xd4" : "\xbf\xc9\xd2\xd4");
		//% prints("匿 名 版:     %s\n", (board->flag & BOARD_ANONY_FLAG) ? "是" : "否");
		prints("\xc4\xe4 \xc3\xfb \xb0\xe6:     %s\n", (board->flag & BOARD_ANONY_FLAG) ? "\xca\xc7" : "\xb7\xf1");
#ifdef ENABLE_PREFIX
		//% prints ("强制前缀:     %s\n", (board->flag & BOARD_PREFIX_FLAG) ? "必须" : "不必");
		prints ("\xc7\xbf\xd6\xc6\xc7\xb0\xd7\xba:     %s\n", (board->flag & BOARD_PREFIX_FLAG) ? "\xb1\xd8\xd0\xeb" : "\xb2\xbb\xb1\xd8");
#endif
		//% prints("俱 乐 部:     %s\n", (board->flag & BOARD_CLUB_FLAG) ?
		prints("\xbe\xe3 \xc0\xd6 \xb2\xbf:     %s\n", (board->flag & BOARD_CLUB_FLAG) ?
				//% (board->flag & BOARD_READ_FLAG) ? "读限制俱乐部" : "普通俱乐部"
				(board->flag & BOARD_READ_FLAG) ? "\xb6\xc1\xcf\xde\xd6\xc6\xbe\xe3\xc0\xd6\xb2\xbf" : "\xc6\xd5\xcd\xa8\xbe\xe3\xc0\xd6\xb2\xbf"
				//% : "非俱乐部");
				: "\xb7\xc7\xbe\xe3\xc0\xd6\xb2\xbf");
		prints("now id  :     %d\n", bs->nowid);
		//% prints("读写限制:     %s\n", (board->flag & BOARD_POST_FLAG) ? "限制发文" :
		prints("\xb6\xc1\xd0\xb4\xcf\xde\xd6\xc6:     %s\n", (board->flag & BOARD_POST_FLAG) ? "\xcf\xde\xd6\xc6\xb7\xa2\xce\xc4" :
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
			else {
				prints("%s\n              ", permstrings[i]);
			}

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

static int board_list_init(board_list_t *p);
static int tui_board_list(board_list_t *l);

static int read_board(tui_list_t *p)
{
	board_list_t *l = p->data;
	board_t *bp = &(l->indices[p->cur]->board);

	if (bp->flag & BOARD_DIR_FLAG) {
		if (bp->flag & BOARD_CUSTOM_FLAG) {
			l->skip_reload = true;
			l->parent = bp->id;
			tui_board_list(l);

			l->skip_reload = false;
			l->parent = FAV_BOARD_ROOT_FOLDER;
			index_favorite_boards(l);
			return PARTUPDATE;
		} else {
			board_list_t nl = {
				.skip_reload = false,
				.favorite = false,
				.newflag = l->newflag,
				.cmp = l->cmp,
				.parent = bp->id,
				.sector = 0,
				.zapbuf = l->zapbuf,
				.boards = NULL,
				.indices = NULL,
				.yank = l->yank,
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

		if (DEFINE(DEF_FIRSTNEW)) {
			int tmp = unread_position(bp);
			int page = tmp - t_lines / 2;

			char file[STRLEN];
			setbdir(file, currboard);
			getkeep(file, page > 1 ? page : 1, tmp + 1);
		}

		board_read();

		brc_zapbuf(l->zapbuf + bp->id);
		currBM[0] = '\0';
	}

	return FULLUPDATE;
}

static int sort_boards(tui_list_t *p)
{
	board_list_t *l = p->data;

	if (currentuser.flags[0] & BRDSORT_FLAG) {
		currentuser.flags[0] ^= BRDSORT_FLAG;
		currentuser.flags[0] |= BRDSORT_ONLINE;
		l->cmp = board_cmp_online;
	} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
		currentuser.flags[0] ^= BRDSORT_ONLINE;
		l->cmp = board_cmp_default;
	} else {
		currentuser.flags[0] |= BRDSORT_FLAG;
		l->cmp = board_cmp_flag;
	}
	
	qsort(l->indices, p->all, sizeof(*l->indices), l->cmp);

	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	return FULLUPDATE;
}

static int board_list_init(board_list_t *p)
{
	memset(p, 0, sizeof(*p));

	if (!streq(currentuser.userid, "guest"))
		p->yank = true;

	char flag = currentuser.flags[0];
	if (flag & BRDSORT_FLAG)
		p->cmp = board_cmp_flag;
	else if (flag & BRDSORT_ONLINE)
		p->cmp = board_cmp_online;
	else if (flag & BRDSORT_UDEF)
		p->cmp = board_cmp_default;
	else if (flag & BRDSORT_UPDATE)
		p->cmp = board_cmp_default;
	else
		p->cmp = board_cmp_default;

	return 0;
}

static tui_list_title_t board_list_title(tui_list_t *p)
{
	//% const char *sort = "分类";
	const char *sort = "\xb7\xd6\xc0\xe0";
	char flag = currentuser.flags[0];
	if (flag & BRDSORT_FLAG) {
		//% sort = "字母";
		sort = "\xd7\xd6\xc4\xb8";
	} else if (flag & BRDSORT_ONLINE) {
		//% sort = "在线";
		sort = "\xd4\xda\xcf\xdf";
	} else if (flag & BRDSORT_UDEF) {
		//% sort = "自定";
		sort = "\xd7\xd4\xb6\xa8";
	} else if (flag & BRDSORT_UPDATE) {
		//% sort = "更新";
		sort = "\xb8\xfc\xd0\xc2";
	}

	char buf[32];
	//% snprintf(buf, sizeof(buf), "[讨论区列表] [%s]", sort);
	snprintf(buf, sizeof(buf), "[\xcc\xd6\xc2\xdb\xc7\xf8\xc1\xd0\xb1\xed] [%s]", sort);
	//% docmdtitle(buf, " \033[m主选单[\033[1;32m←\033[m,\033[1;32me\033[m] 阅读"
	docmdtitle(buf, " \033[m\xd6\xf7\xd1\xa1\xb5\xa5[\033[1;32m\xa1\xfb\033[m,\033[1;32me\033[m] \xd4\xc4\xb6\xc1"
			//% "[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 选择[\033[1;32m↑\033[m,"
			"[\033[1;32m\xa1\xfa\033[m,\033[1;32mRtn\033[m] \xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m,"
			//% "\033[1;32m↓\033[m] 列出[\033[1;32my\033[m] 排序[\033[1;32ms"
			"\033[1;32m\xa1\xfd\033[m] \xc1\xd0\xb3\xf6[\033[1;32my\033[m] \xc5\xc5\xd0\xf2[\033[1;32ms"
			//% "\033[m] 搜寻[\033[1;32m/\033[m] 切换[\033[1;32mc\033[m] 求助"
			"\033[m] \xcb\xd1\xd1\xb0[\033[1;32m/\033[m] \xc7\xd0\xbb\xbb[\033[1;32mc\033[m] \xc7\xf3\xd6\xfa"
			"[\033[1;32mh\033[m]\n");
	//% prints("\033[1;44;37m 编号 未 讨论区名称        V  类别  %-20s S 版  主        "
	prints("\033[1;44;37m \xb1\xe0\xba\xc5 \xce\xb4 \xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6        V  \xc0\xe0\xb1\xf0  %-20s S \xb0\xe6  \xd6\xf7        "
			//% "在线 \033[m\n", "中  文  叙  述");
			"\xd4\xda\xcf\xdf \033[m\n", "\xd6\xd0  \xce\xc4  \xd0\xf0  \xca\xf6");
}

static tui_list_handler_t board_list_handler(tui_list_t *p, int key)
{
	board_list_t *l = p->data;
	bool st_changed = false, handled = true;

	switch (key) {
		case 'c':
			l->newflag = !l->newflag;
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
			save_zapbuf(l);
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
			return tui_favorite_add(p);
		case 'A':
			return tui_favorite_mkdir(p);
		default:
			handled = false;
	}

	if (p->cur >= p->all)
		return READ_AGAIN;

	board_t *board = &(l->indices[p->cur]->board);

	switch (key) {
		case '*':
			return show_board_info(board);
		case '/':
			// TODO: search.
			break;
		case 's':
			return sort_boards(p);
		case 'y':
			if (l->favorite)
				return DONOTHING;
			l->yank = !l->yank;
			index_boards(l);
			p->all = l->count;
			return PARTUPDATE;
		case 'z':
			if (l->favorite)
				return DONOTHING;
			if (HAS_PERM(PERM_LOGIN)
					&& !(board->flag & BOARD_NOZAP_FLAG)) {
				//% if (l->zapbuf[board->id] && !askyn("确实要隐藏吗?", NA, YEA))
				if (l->zapbuf[board->id] && !askyn("\xc8\xb7\xca\xb5\xd2\xaa\xd2\xfe\xb2\xd8\xc2\xf0?", NA, YEA))
					return MINIUPDATE;
				l->zapbuf[board->id] = !l->zapbuf[board->id];
			}
			index_boards(l);
			p->all = l->count;
			return PARTUPDATE;
		case 'T':
			return tui_favorite_rename(p);
		case 'd':
			return tui_favorite_rm(p);
		case 'C':
			return tui_favorite_copy(p);
		case 'P':
			return tui_favorite_paste(p);
		case '\r':
		case '\n':
		case KEY_RIGHT:
			read_board(p);
			jump_to_first_unread(p);
			st_changed = true;
			return FULLUPDATE;
		default:
			if (!handled)
				return READ_AGAIN;
	}

	if (st_changed)
		set_user_status(l->newflag ? ST_READNEW : ST_READBRD);
	return FULLUPDATE;
}

static int tui_board_list(board_list_t *l)
{
	bool alloc_zapbuf = !l->zapbuf;
	if (alloc_zapbuf)
		load_zapbuf(l);

	tui_list_t t = {
		.data = l,
		.loader = board_list_load,
		.title = board_list_title,
		.display = board_list_display,
		.handler = board_list_handler,
		.query = NULL,
	};

	set_user_status(l->newflag ? ST_READNEW : ST_READBRD);

	tui_list(&t);
	
	if (!l->skip_reload) {
		free(l->boards);
		free(l->indices);
	}

	if (alloc_zapbuf) {
		save_zapbuf(l);
		free(l->zapbuf);
	}

	clear();
	return 0;
}

int tui_all_boards(const char *cmd)
{
	board_list_t l;
	board_list_init(&l);
	return tui_board_list(&l);
}

int tui_unread_boards(const char *cmd)
{
	board_list_t l;
	board_list_init(&l);
	l.newflag = true;
	return tui_board_list(&l);
}

int tui_read_sector(const char *cmd)
{
	board_list_t l;
	board_list_init(&l);
	l.newflag = true;

#ifdef FDQUAN
	char s[] = "a";
	s[0] = *cmd;
	db_res_t *res = db_query("SELECT id FROM board_sectors"
			" WHERE name = %s", s);
	if (res && db_res_rows(res) > 0)
		l.sector = db_get_integer(res, 0, 0);
	db_clear(res);
#else
	char c = *cmd;
	if (c > 'A')
		l.sector = c - 'A' + 1;
	else
		l.sector = c - '0' + 1;
#endif
	return tui_board_list(&l);
}

int tui_favorite_boards(const char *cmd)
{
	board_list_t l;
	board_list_init(&l);

	l.favorite = true;
	l.newflag = true;

	return tui_board_list(&l);
}
