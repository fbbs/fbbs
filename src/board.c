#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/status.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct {
	board_t board;
	int folder;
} board_extra_t;

typedef struct {
	board_extra_t *boards;      ///< Array of boards.
	board_extra_t **indices;
	comparator_t cmp;     ///< Compare function pointer.
	int count;            ///< Number of boards loaded.
	int fcount;           ///< Number of favorite boards indexed.
	int sector;
	int parent;
	int *zapbuf;          ///< Subscribing record.
	bool yank;            ///< True if hide unsubscribed boards.
	bool newflag;         ///< True if jump to unread board.
	bool favorite;        ///< True if reading favorite boards.
	bool recursive;       ///< True if called recursively.
//	int copy_bnum;        ///< Copy/paste buffer.
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
		restart_close(fd);
	}

	for (int i = n / sizeof(*l->zapbuf); i < MAXBOARD; ++i) {
		l->zapbuf[i] = 1;
	}
	return 0;
}

int save_zapbuf(const board_list_t *l)
{
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");
	int fd = open(file, O_WRONLY | O_CREAT, 0600);
	if (fd >= 0) {
		write(fd, l->zapbuf, sizeof(*l->zapbuf) * MAXBOARD);
		restart_close(fd);
		return 0;
	}
	return -1;
}

#if 0
/**
 *
 */
static void load_default_board(choose_board_t *cbrd)
{
	if (cbrd->gnum == 0) {
		cbrd->gnum = 1;
		int i = getbnum(DEFAULTBOARD, &currentuser);
		if (i == 0)
			i = getbnum(currboard, &currentuser);
		cbrd->gbrds->id = 1;
		cbrd->gbrds->pid = 0;
		cbrd->gbrds->pos = i - 1;
		cbrd->gbrds->flag = bcache[i - 1].flag;
		strlcpy(cbrd->gbrds->filename, bcache[i - 1].filename,
				sizeof(cbrd->gbrds->filename));
		strlcpy(cbrd->gbrds->title, bcache[i - 1].title,
				sizeof(cbrd->gbrds->title));
	}
}

/**
 *
 */
static void goodbrd_load(choose_board_t *cbrd)
{
	cbrd->gnum = 0;

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	FILE *fp = fopen(file, "r");
	if (fp) {
		while (fread(cbrd->gbrds + cbrd->gnum, sizeof(*cbrd->gbrds), 1, fp)) {
			if (cbrd->gbrds[cbrd->gnum].flag & BOARD_CUSTOM_FLAG) {
				cbrd->gnum++;
			} else {
				if (hasreadperm(&currentuser,
						bcache + cbrd->gbrds[cbrd->gnum].pos)) {
					cbrd->gnum++;
				}
			}
			if (cbrd->gnum == GOOD_BRC_NUM)
				break;
		}
		fclose(fp);
	}

	load_default_board(cbrd);
}

/**
 *
 */
static int goodbrd_save(choose_board_t *cbrd)
{
	load_default_board(cbrd);

	char file[HOMELEN];
	setuserfile(file, ".goodbrd");
	FILE *fp = fopen(file, "w");
	if (fp) {
		fwrite(cbrd->gbrds, sizeof(*cbrd->gbrds), cbrd->gnum, fp);
		fclose(fp);
		return 0;
	}
	return -1;
}

/**
 * 
 */
static int goodbrd_add(choose_board_t *cbrd, int bnum, int pid)
{
	if (cbrd->gnum >= GOOD_BRC_NUM)
		return -1;

	if (--bnum < 0 || inGoodBrds(cbrd, bnum))
		return -1;

	if (!hasreadperm(&currentuser, bcache + bnum))
		return -1;

	gbrdh_t *ptr = cbrd->gbrds + cbrd->gnum;
	ptr->pid = pid;
	ptr->pos = bnum;
	strlcpy(ptr->filename, bcache[bnum].filename, sizeof(ptr->filename));
	strlcpy(ptr->title, bcache[bnum].title, sizeof(ptr->title));
	ptr->flag = bcache[bnum].flag;

	if (cbrd->gnum)
		ptr->id = (ptr - 1)->id + 1;
	else
		ptr->id = 1;

	cbrd->gnum++;
	goodbrd_save(cbrd);
	return 0;
}

// TODO: pid暂时是个不使用的参数，因为不打算建二级目录（删除目录的代码目录仍不完善）
static int goodbrd_mkdir(choose_board_t *cbrd, const char *name,
		const char *title, int pid)
{
	if (cbrd->gnum < GOOD_BRC_NUM) {
		gbrdh_t *ptr = cbrd->gbrds + cbrd->gnum;
		ptr->pid = 0;
		strlcpy(ptr->filename, name, sizeof(ptr->filename));
		strlcpy(ptr->title, "~[收藏] ○ ", sizeof(ptr->title));
		if (title[0] != '\0')
			strlcpy(ptr->title + 11, title, sizeof(ptr->title) - 11);
		else
			strlcpy(ptr->title + 11, "自定义目录", sizeof(ptr->title) - 11);
		ptr->flag = BOARD_DIR_FLAG | BOARD_CUSTOM_FLAG;
		ptr->pos = -1;
		if (cbrd->gnum)
			ptr->id = (ptr - 1)->id + 1;
		else
			ptr->id = 1;
		cbrd->gnum++;
		return goodbrd_save(cbrd);
	}
	return -1;
}

//目录没有对二级目录嵌套删除的功能，也因为这个限制，收藏夹目录不允许建立二级目录
static void goodbrd_rmdir(choose_board_t *cbrd, int id)
{
	int i, n = 0;
	for (i = 0; i < cbrd->gnum; i++) {
		gbrdh_t *ptr = cbrd->gbrds + i;
		if (((ptr->flag & BOARD_CUSTOM_FLAG) && (ptr->id == id))
				|| (ptr->pid == id)) {
			continue;
		} else {
			if (i != n)
				memcpy(cbrd->gbrds + n, cbrd->gbrds + i, sizeof(cbrd->gbrds[0]));
			n++;
		}
	}
	cbrd->gnum = n;
	goodbrd_save(cbrd);
}

/**
 *
 */
static int tui_goodbrd_add(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;

	if (!HAS_PERM(PERM_LOGIN))
		return DONOTHING;

	if (cbrd->goodbrd && (cbrd->parent == -1))
		return DONOTHING;

	if (cbrd->goodbrd) {
		if (cbrd->parent == -1)
			return DONOTHING;

		if (cbrd->gnum >= GOOD_BRC_NUM) {
			presskeyfor("收藏夹已满", t_lines - 1);
			return MINIUPDATE;
		}

		int pos;
		char bname[STRLEN];
		struct boardheader fh;
		if (gettheboardname(1, "输入讨论区名 (按空白键自动搜寻): ",
				&pos, &fh, bname, 1)) {
			if (goodbrd_add(cbrd, pos, cbrd->parent) == 0)
				cp->valid = false;
		}
		return FULLUPDATE;
	} else {
		goodbrd_load(cbrd);
		if (cbrd->gnum >= GOOD_BRC_NUM) {
			presskeyfor("收藏夹已满", t_lines - 1);
			return MINIUPDATE;
		} else {
			char buf[STRLEN];
			snprintf(buf, sizeof(buf), "您确定要添加 %s 到收藏夹吗?",
					cbrd->brds[cp->cur].name);
			if (askyn(buf, false, true)) {
				if (goodbrd_add(cbrd, cbrd->brds[cp->cur].pos + 1, 0) == 0) {
					cp->valid = false;
					return PARTUPDATE;
				}
			}
			return MINIUPDATE;
		}
	}
}

/**
 * Copy board when browsing favorites.
 * @param cp browsing status.
 * @return update status.
 */
static int tui_goodbrd_copy(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;

	if (!HAS_PERM(PERM_LOGIN) || !cbrd->goodbrd)
		return DONOTHING;

	if (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG)
		return DONOTHING;

	cbrd->copy_bnum = cbrd->brds[cp->cur].pos + 1;
	presskeyfor("版面已复制 请按P粘贴", t_lines - 1);
	return MINIUPDATE;
}

/**
 * Paste copied board when browsing favorites.
 * @param cp browsing status.
 * @return update status.
 */
static int tui_goodbrd_paste(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;

	if (!HAS_PERM(PERM_LOGIN) || !cbrd->goodbrd)
		return DONOTHING;

	if (goodbrd_add(cbrd, cbrd->copy_bnum, cbrd->parent) == 0) {
		cbrd->copy_bnum = 0;
		cp->valid = false;
		return PARTUPDATE;
	}
	return DONOTHING;
}

/**
 * Create directory when browsing favorites.
 * @param cp browsing status.
 * @return update status.
 */
static int tui_goodbrd_mkdir(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;

	if (!HAS_PERM(PERM_LOGIN) || !cbrd->goodbrd)
		return DONOTHING;

	if (cbrd->parent == 0) {
		if (cbrd->gnum >= GOOD_BRC_NUM) {
			presskeyfor("收藏夹已满", t_lines - 1);
			return MINIUPDATE;
		}

		char name[STRLEN];
		char title[STRLEN];
		name[0] = '\0';
		getdata(t_lines - 1, 0, "创建自定义目录: ", name, 17,
				DOECHO, NA);
		if (name[0] != '\0') {
			strlcpy(title, "自定义目录", sizeof(title));
			getdata(t_lines - 1, 0, "自定义目录描述: ", title, 21,
					DOECHO, NA);
			if (goodbrd_mkdir(cbrd, name, title, 0) == 0) {
				cp->valid = false;
				return PARTUPDATE;
			}
		}
		return MINIUPDATE;
	}
	return DONOTHING;
}

/**
 * Rename directory when browsing favorites.
 * @param cp browsing status.
 * @return update status.
 */
static int tui_goodbrd_rename(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;

	if (!HAS_PERM(PERM_LOGIN) || !cbrd->goodbrd)
		return DONOTHING;

	if ((cbrd->parent == 0) && cbrd->num
			&& (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG)) {

		int gbid;
		for (gbid = 0; gbid < cbrd->gnum; gbid++) {
			if (cbrd->gbrds[gbid].id == cbrd->brds[cp->cur].pos)
				break;
		}
		if (gbid == cbrd->gnum)
			return DONOTHING;

		gbrdh_t *gbp = cbrd->gbrds + gbid;
		char name[STRLEN];
		char title[STRLEN];
		strlcpy(name, gbp->filename, sizeof(name));
		getdata(t_lines - 1, 0, "修改自定义目录名: ", name, 17, DOECHO, NA);
		if (name[0] != '\0') {
			strlcpy(title, gbp->title + 11, sizeof(title));
			getdata(t_lines - 1, 0, "自定义目录描述: ", title, 21, DOECHO, NA);
			if (title[0] == '\0')
				strlcpy(title, gbp->title + 11, sizeof(title));
			strlcpy(gbp->filename, name, sizeof(gbp->filename));
			strlcpy(gbp->title + 11, title, sizeof(gbp->title) - 11);
			if (goodbrd_save(cbrd) == 0)
				return PARTUPDATE;
		}
		return MINIUPDATE;
	}
	return DONOTHING;
}

/**
 * Remove board/directory from favorites.
 * @param cp browsing status.
 * @return update status.
 */
static int tui_goodbrd_rm(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;

	if (cbrd->goodbrd && cbrd->num > 0) {
		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "要把 %s 从收藏夹中去掉？",
				cbrd->brds[cp->cur].name);
		if (askyn(buf, false, true)) {
			if (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG) {
				goodbrd_rmdir(cbrd, cbrd->brds[cp->cur].pos);
			} else {
				int pos = inGoodBrds(cbrd, cbrd->brds[cp->cur].pos);
				if (pos) {
					memmove(cbrd->gbrds + pos - 1, cbrd->gbrds + pos,
							sizeof(gbrdh_t) * (cbrd->gnum - pos));
					cbrd->gnum--;
					goodbrd_save(cbrd);
				}
			}
			cp->valid = false;
			return PARTUPDATE;
		}
		return MINIUPDATE;
	}
	return DONOTHING;
}
#endif

static bool check_newpost(board_t *board)
{
	if (!brc_initial(currentuser.userid, board->name)) {
		return true;
	} else {
		if (brc_unread1((brdshm->bstatus[board->id]).lastpost)) {
			return true;
		}
	}
	return false;
}

static void res_to_board_array(board_list_t *l, db_res_t *r1, db_res_t *r2)
{
	int rows = db_res_rows(r1) + db_res_rows(r2);
	l->boards = malloc(sizeof(*l->boards) * rows);

	l->count = 0;

	for (int i = 0; i < db_res_rows(r1); ++i) {
		board_t *board = &(l->boards + l->count)->board;
		res_to_board(r1, i, board);
		board_to_gbk(board);

		if (l->favorite)
			((board_extra_t *)board)->folder = db_get_integer(r1, i, 8);

		if (has_read_perm(&currentuser, board))
			++l->count;
	}

	if (r2) {
		for (int i = 0; i < db_res_rows(r2); ++i) {
			board_t *board = &(l->boards + l->count)->board;

			board->id = db_get_integer(r2, i, 0);
			convert_u2g(db_get_value(r2, i, 1), board->name);
			board->flag |= BOARD_CUSTOM_FLAG;

			++l->count;
		}
	}
}

static void load_favorite_boards(board_list_t *l)
{
	if (!l->recursive) {
		db_res_t *r1 = db_query("SELECT "BOARD_BASE_FIELDS", f.folder "
				"FROM "BOARD_BASE_TABLES" JOIN fav_boards f ON b.id = f.board "
				"WHERE f.user_id = %"PRIdUID, session.uid);
		db_res_t *r2 = db_query("SELECT id, name FROM fav_board_folders "
				"WHERE user_id = %"PRIdUID, session.uid);

		res_to_board_array(l, r1, r2);

		db_clear(r1);
		db_clear(r2);
	}
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
		res = db_exec_query(env.d, true, BOARD_SELECT_QUERY_BASE);
	}
	res_to_board_array(l, res, NULL);
}

static void index_favorite_boards(board_list_t *l)
{
	if (!l->parent)
		l->parent = 1;

	l->fcount = 0;

	for (int i = 0; i < l->count; ++i) {
		board_extra_t *p = l->boards + i;
		if (p->folder == l->parent)
			l->indices[l->fcount++] = p;
	}
}

static void index_boards(board_list_t *l)
{
	for (int i = 0; i < l->count; ++i) {
		l->indices[i] = l->boards + i;
	}
	qsort(l->indices, l->count, sizeof(*l->indices), l->cmp);
}

static tui_list_loader_t board_list_load(tui_list_t *p)
{
	board_list_t *l = p->data;
	free(l->indices);
	free(l->boards);

	if (l->favorite)
		load_favorite_boards(l);
	else
		load_boards(l);

	if (l->count) {
		l->indices = malloc(sizeof(*l->indices) * l->count);
		if (l->favorite)
			index_favorite_boards(l);
		else
			index_boards(l);
	}

	p->all = l->favorite ? l->fcount : l->count;
	p->eod = true;
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
		move(t_lines - 1, 0);
		clrtoeol();
		prints("请输入要查找的版面名称：%s", bname);
		ch = egetch();

		if (isprint2(ch)) {
			bname[i++] = ch;
			for (n = 0; n < cbrd->num; n++) {
				if (!strncasecmp(cbrd->brds[n].name, bname, i)) {
					tmpn = YEA;
					*num = n;
					if (!strcmp(cbrd->brds[n].name, bname))
						return 1 /* 找到类似的版，画面重画
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
		move(t_lines - 1, 0);
		clrtoeol();
		return 2 /* 结束了 */;
	}
	return 1;
}

int unread_position(char *dirfile, board_data_t *ptr)
{
	struct fileheader fh;
	char filename[STRLEN];
	int fd, offset, step, num;
	num = ptr->total + 1;
	if (ptr->unread && (fd = open (dirfile, O_RDWR))> 0) {
		if (!brc_initial (currentuser.userid, ptr->name)) {
			num = 1;
		} else {
			offset = (int) ((char *) &(fh.filename[0]) - (char *) &(fh));
			num = ptr->total - 1;
			step = 4;
			while (num> 0) {
				lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
				if (read (fd, filename, STRLEN) <= 0 || !brc_unread (filename))
				break;
				num -= step;
				if (step < 32)
				step += step / 2;
			}
			if (num < 0)
			num = 0;
			while (num < ptr->total) {
				lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
				if (read (fd, filename, STRLEN) <= 0 || brc_unread (filename))
				break;
				num++;
			}
		}
		close (fd);
	}
	if (num < 0)
	num = 0;
	return num;
}
#endif

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
		prints("  目录");
	else
		prints(" %5d", (brdshm->bstatus[board->id]).total);

	if (board->flag & BOARD_DIR_FLAG) {
		prints("  ＋");
	} else {
		bool unread = check_newpost(board);
		prints("  %s", unread ? "◆" : "◇");
	}

	char descr[24];
	strlcpy(descr, board->descr, sizeof(descr));
	ellipsis(descr, 20);

	prints("%c%-17s %s%s%6s %-20s %c ",
			(is_zapped(l, board)) ? '*' : ' ', board->name,
			(board->flag & BOARD_VOTE_FLAG) ? "\033[1;31mV\033[m" : " ",
			(board->flag & BOARD_CLUB_FLAG) ? (board->flag & BOARD_READ_FLAG)
				? "\033[1;31mc\033[m" : "\033[1;33mc\033[m" : " ",
			board->categ, descr, HAS_PERM(PERM_POST) ? property(board) : ' ');

	if (board->flag & BOARD_DIR_FLAG) {
		prints("[目录]\n");
	} else {
		char bms[IDLEN + 1];
		strlcpy(bms, board->bms, sizeof(bms));
		prints("%-12s %4d\n",
				bms[0] <= ' ' ? "诚征版主中" : strtok(bms, " "),
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
	board_t *b1 = *(board_t * const *)p1;
	board_t *b2 = *(board_t * const *)p2;
	int type = strcasecmp(b1->categ, b2->categ);
	if (type == 0)
		type = strcasecmp(b1->name, b2->name);
	return type;
}

static int show_board_info(board_t *board)
{
	if (!board || board->flag & BOARD_CUSTOM_FLAG)
		return DONOTHING;

	board_t parent;
	if (board->parent)
		get_board_by_bid(board->parent, &parent);

	struct bstat *bs = getbstat(board->name);
	clear();
	prints("版面详细信息:\n\n");
	prints("ID      :     %d\n", board->id);
	prints("英文名称:     %s\n", board->name);
	prints("中文名称:     %s\n", board->descr);
	prints("版    主:     %s\n", board->bms);
	prints("所属讨论区:   %s\n", board->parent ? parent.name : "无");
	prints("是否目录:     %s\n", (board->flag & BOARD_DIR_FLAG) ? "目录" : "版面");
	prints("可以 ZAP:     %s\n", (board->flag & BOARD_NOZAP_FLAG) ? "不可以" : "可以");

	if (!(board->flag & BOARD_DIR_FLAG)) {
		prints("在线人数:     %d 人\n", bs->inboard);
		prints("文 章 数:     %s\n", (board->flag & BOARD_JUNK_FLAG) ? "不计算" : "计算");
		prints("可以回复:     %s\n", (board->flag & BOARD_NOREPLY_FLAG) ? "不可以" : "可以");
		prints("匿 名 版:     %s\n", (board->flag & BOARD_ANONY_FLAG) ? "是" : "否");
#ifdef ENABLE_PREFIX
		prints ("强制前缀:     %s\n", (board->flag & BOARD_PREFIX_FLAG) ? "必须" : "不必");
#endif
		prints("俱 乐 部:     %s\n", (board->flag & BOARD_CLUB_FLAG) ?
				(board->flag & BOARD_READ_FLAG) ? "读限制俱乐部" : "普通俱乐部"
				: "非俱乐部");
		prints("now id  :     %d\n", bs->nowid);
		prints("读写限制:     %s\n", (board->flag & BOARD_POST_FLAG) ? "限制发文" :
				(board->perm == 0) ? "没有限制" : "限制阅读");
	}
	if (HAS_PERM(PERM_SPECIAL0) && board->perm) {
		prints("权    限:     ");
		char secu[] = "ltmprbBOCAMURS#@XLEast0123456789";
		for (int i = 0; i < 32; i++) {
			if (!(board->perm & (1 << i)))
				secu[i] = '-';
			else {
				prints("%s\n              ", permstrings[i]);
			}

		}
		prints("\n权 限 位:     %s\n", secu);
	}

	prints("URL 地址:     http://"BBSHOST"/bbs/doc?bid=%d\n", board->id);
	pressanykey();
	return FULLUPDATE;
}

static int show_hotspot(void)
{
	char ans[2];
	getdata(t_lines - 1, 0, "您选择? (1) 本日十大  (2) 系统热点 [1]",
			ans, 2, DOECHO, YEA);
	if (ans[0] == '2')
		show_help("etc/hotspot");
	else
		show_help("0Announce/bbslist/day");
	return FULLUPDATE;
}

static int read_board(tui_list_t *p)
{
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

#if 0
static int choose_board_read(tui_list_t *cp)
{
	choose_board_t *cbrd = cp->data;
	board_data_t *ptr = cbrd->brds + cp->cur;
	if (ptr->flag & BOARD_DIR_FLAG) {
		int parent = cbrd->parent;
		const char *prefix = cbrd->prefix;
		bool recursive = cbrd->recursive;
		bool goodbrd = cbrd->goodbrd;
		int cur = cp->cur;

		cbrd->parent = ptr->pos;
		cbrd->prefix = NULL;
		cbrd->recursive = true;
		cbrd->goodbrd = (ptr->flag & BOARD_CUSTOM_FLAG) ? true : false;

		choose_board(cbrd);

		cbrd->parent = parent;
		cbrd->prefix = prefix;
		cbrd->recursive = recursive;
		cbrd->goodbrd = goodbrd;
		cp->cur = cur;
	} else {
		brc_initial(currentuser.userid, ptr->name);
		changeboard(&currbp, currboard, ptr->name);
		memcpy(currBM, ptr->BM, BM_LEN - 1);

		char buf[STRLEN];
		if (DEFINE(DEF_FIRSTNEW)) {
			setbdir(buf, currboard);
			int tmp = unread_position(buf, ptr);
			int page = tmp - t_lines / 2;
			getkeep(buf, page > 1 ? page : 1, tmp + 1);
		}
		board_read();
		brc_zapbuf(cbrd->zapbuf + ptr->pos);
		ptr->total = -1;
		currBM[0] = '\0';
	}
	return FULLUPDATE;
}
#endif

static int board_list_init(board_list_t *p)
{
	if (p->recursive)
		return 0;

	memset(p, 0, sizeof(*p));
	
	load_zapbuf(p);

	if (streq(currentuser.userid, "guest"))
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
	board_list_t *l = p->data;

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
	docmdtitle(buf, " \033[m主选单[\033[1;32m←\033[m,\033[1;32me\033[m] 阅读"
			"[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 选择[\033[1;32m↑\033[m,"
			"\033[1;32m↓\033[m] 列出[\033[1;32my\033[m] 排序[\033[1;32ms"
			"\033[m] 搜寻[\033[1;32m/\033[m] 切换[\033[1;32mc\033[m] 求助"
			"[\033[1;32mh\033[m]\n");
	prints("\033[1;44;37m %s 讨论区名称        V  类别  %-20s S 版  主        "
			"在线 \033[m\n",
			l->newflag ? "全 部  未" : "编 号  未", "中  文  叙  述");
}

static tui_list_handler_t board_list_handler(tui_list_t *p, int key)
{
	board_list_t *l = p->data;
	bool st_changed = false;

	switch (key) {
		case 'c':
			l->newflag = !l->newflag;
			return PARTUPDATE;
		case 'L':
			m_read();
			p->valid = false;
			st_changed = true;
			break;
		case 'M':
			m_new();
			p->valid = false;
			st_changed = true;
			break;
		case 'u':
			set_user_status(ST_QUERY);
			t_query();
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
			free(l->zapbuf);
			free(l->indices);
			free(l->boards);
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
			online_users_show_override();
			st_changed = true;
			break;
	}

	if (p->cur >= p->all)
		return DONOTHING;

	board_t *board = &(l->indices[p->cur]->board);

	switch (key) {
		case '*':
			return show_board_info(board);
		case '/':
			// TODO: search.
			break;
		case 's':
			return sort_boards(p);
#if 0
		case 'y':
			if (cbrd->gnum)
				return DONOTHING;
			l->yank = !l->yank;
			p->valid = false;
			return PARTUPDATE;
		case 'z':
			if (cbrd->gnum)
				return DONOTHING;
			if (HAS_PERM(PERM_LOGIN)
					&& !(cbrd->brds[cp->cur].flag & BOARD_NOZAP_FLAG)) {
				ptr = cbrd->brds + cp->cur;
				ptr->zap = !ptr->zap;
				ptr->total = -1;
				cbrd->zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
			}
			p->valid = false;
			return PARTUPDATE;
		case 'a':
			return tui_goodbrd_add(cp);
		case 'A':
			return tui_goodbrd_mkdir(cp);
		case 'T':
			return tui_goodbrd_rename(cp);
		case 'd':
			return tui_goodbrd_rm(cp);
		case 'C':
			return tui_goodbrd_copy(cp);
		case 'P':
			return tui_goodbrd_paste(cp);
#endif
		case '\r':
		case '\n':
		case KEY_RIGHT:
			read_board(p);
//			cp->valid = false;
			st_changed = true;
			break;
		default:
			return DONOTHING;
	}

	if (st_changed)
		set_user_status(l->newflag ? ST_READNEW : ST_READBRD);

	return FULLUPDATE;
}

int tui_board_list(board_list_t *l)
{
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

	free(l->boards);
	free(l->indices);

	if (!l->recursive) {
		clear();
		save_zapbuf(l);
		free(l->zapbuf);
	}

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

	// TODO: figure out how to integrate with terminal menu

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
