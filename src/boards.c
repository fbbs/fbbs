#include "bbs.h"
#include "list.h"

#define BBS_PAGESIZE    (t_lines - 4)

extern time_t login_start_time;

typedef struct {
	char *name;        ///< Board name.
	char *title;       ///< Board description.
	char *BM;          ///< Board masters.
	unsigned int flag; ///< Board flag. @see ::boardheader.
	int parent;        ///< Parent directory.
	int pos;           ///< Position in ::bcache, 0-based.
	int total;         ///< Number of posts in the board.
	bool unread;       ///< True if there are unread posts in the board.
	bool zap;          ///< True if the board is zapped.
	char property;     ///< A character reflects the board's property.
} board_data_t;

typedef struct {
	comparator_t cmp;     ///< Compare function pointer.
	board_data_t *brds;   ///< Array of boards.
	int *zapbuf;          ///< Subscribing record.
	char *prefix;         ///< Group by prefix if not NULL, by dir otherwise.
	int num;              ///< Number of boards loaded.
	bool yank;            ///< True if hide unsubscribed boards.
	bool newflag;         ///< True if jump to unread board.
	bool goodbrd;         ///< True if reading favorite boards.
	bool recursive;       ///< True if called recursively.
	gbrdh_t *gbrds;       ///< Array of favorite boards.
	int gnum;             ///< Number of favorite boards.
	int parent;           ///< Parent directory.
	char buf[STRLEN-8];   ///< Copy/paste buffer.
} choose_board_t;

/**
 *
 */
static int inGoodBrds(const choose_board_t *cbrd, int pos)
{
	int i;
	for (i = 0; i < cbrd->gnum && i < GOOD_BRC_NUM; i++) {
		if ((cbrd->gbrds[i].pid == cbrd->parent)
				&& (!(cbrd->gbrds[i].flag & BOARD_CUSTOM_FLAG))
				&& (pos == cbrd->gbrds[i].pos)) {
			return i + 1;
		}
	}
	return 0;
}

/**
 *
 */
static int load_zapbuf(choose_board_t *cbrd)
{
	if (cbrd->zapbuf == NULL) {
		cbrd->zapbuf = malloc(sizeof(*cbrd->zapbuf) * MAXBOARD);
		if (cbrd->zapbuf == NULL)
			return -1;
	} else {
		return 0;
	}

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");
	int n = 0;
	int fd = open(file, O_RDONLY, 0600);
	if (fd >= 0) {
		n = read(fd, cbrd->zapbuf, sizeof(*cbrd->zapbuf) * numboards);
		restart_close(fd);
	}
	int i;
	for (i = n / sizeof(*cbrd->zapbuf); i < MAXBOARD; ++i) {
		cbrd->zapbuf[i] = 1;
	}
	return 0;
}

/**
 *
 */
int save_zapbuf(const choose_board_t *cbrd)
{
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".lastread");
	int fd = open(file, O_WRONLY | O_CREAT, 0600);
	if (fd >= 0) {
		write(fd, cbrd->zapbuf, sizeof(int) * numboards);
		close(fd);
		return 0;
	}
	return -1;
}

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
static void goodbrd_save(choose_board_t *cbrd)
{
	load_default_board(cbrd);

	char file[HOMELEN];
	setuserfile(file, ".goodbrd");
	FILE *fp = fopen(file, "w");
	if (fp) {
		fwrite(cbrd->gbrds, sizeof(*cbrd->gbrds), cbrd->gnum, fp);
		fclose(fp);
	}
}

/**
 *
 */
static void goodbrd_add(choose_board_t *cbrd, char *board, int pid)
{
	if (cbrd->gnum >= GOOD_BRC_NUM)
		return;

	int i = getbnum(board, &currentuser);
	if (i > 0) {
		gbrdh_t *ptr = cbrd->gbrds + cbrd->gnum;
		ptr->pid = pid;
		ptr->pos = --i;
		strlcpy(ptr->filename, bcache[i].filename, sizeof(ptr->filename));
		strlcpy(ptr->title, bcache[i].title, sizeof(ptr->title));
		ptr->flag = bcache[i].flag;
		if (cbrd->gnum)
			ptr->id = (ptr - 1)->id + 1;
		else
			ptr->id = 1;

		cbrd->gnum++;
		goodbrd_save(cbrd);
	}
}

//pid暂时是个不使用的参数，因为不打算建二级目录（删除目录的代码目录仍不完善）
static void goodbrd_mkdir(choose_board_t *cbrd, const char *name,
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
		goodbrd_save(cbrd);
	}
}

//目录没有对二级目录嵌套删除的功能，也因为这个限制，收藏夹目录不允许建立二级目录
void goodbrd_rmdir(choose_board_t *cbrd, int id)
{
	// TODO: to many copying?
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

static bool check_newpost(board_data_t *ptr)
{
	ptr->unread = false;
	ptr->total = (brdshm->bstatus[ptr->pos]).total;
	if (!brc_initial(currentuser.userid, ptr->name)) {
		ptr->unread = true;
	} else {
		if (brc_unread1((brdshm->bstatus[ptr->pos]).lastpost)) {
			ptr->unread = true;
		}
	}
	return ptr->unread;
}

/**
 *
 */
static int choose_board_load(choose_t *cp)
{
	choose_board_t *cbrd = cp->data;
	struct boardheader *bptr;
	board_data_t *ptr;
	gbrdh_t *gptr;
	int addto = 0;

	cbrd->num = 0;

	if (cbrd->goodbrd)
		goodbrd_load(cbrd);

	int n;
	for (n = 0; n < numboards; n++) {
		bptr = bcache + n;
		if (bptr->filename[0] == '\0')
			continue;

		if (cbrd->goodbrd) {
			addto = inGoodBrds(cbrd, n);
		} else {
			if (!(bptr->flag & BOARD_POST_FLAG) && !HAS_PERM(bptr->level)
					&& !(bptr->flag & BOARD_NOZAP_FLAG))
				continue;
			if ((bptr->flag & BOARD_CLUB_FLAG) 
					&& (bptr->flag & BOARD_READ_FLAG )
					&& !chkBM(bptr, &currentuser)
					&& !isclubmember(currentuser.userid, bptr->filename))
				continue;

			if (cbrd->prefix != NULL) {
				if (!strchr(cbrd->prefix, bptr->title[0])
						&& cbrd->prefix[0] != '*')
					continue;
				if (cbrd->prefix[0] == '*') {
					if (!strstr(bptr->title, "●") && !strstr(bptr->title, "⊙")
							&& bptr->title[0] != '*')
						continue;
				}
			} else {
				if (cbrd->parent > 0 && bptr->group != cbrd->parent + 1)
					continue;
				if (cbrd->parent == 0 && bptr->group != 0)
					continue;
				if (cbrd->parent > 0 && bptr->title[0] == '*')
					continue;
			}

			addto = cbrd->yank || cbrd->zapbuf[n] != 0
					|| (bptr->flag & BOARD_NOZAP_FLAG);
		}

		if (addto) {
			ptr = cbrd->brds + cbrd->num++;
			ptr->name = bptr->filename;
			ptr->title = bptr->title;
			ptr->BM = bptr->BM;
			ptr->flag = bptr->flag;
			if (cbrd->goodbrd)
				ptr->parent = cbrd->gbrds[addto - 1].pid;
			else
				ptr->parent = bptr->group;
			ptr->pos = n;
			ptr->total = -1;
			ptr->zap = (cbrd->zapbuf[n] == 0);
			if (bptr ->flag & BOARD_DIR_FLAG) {
				if (bptr->level != 0)
					ptr->property = 'r';
				else
					ptr->property = ' ';
			} else {
				if (bptr->flag & BOARD_NOZAP_FLAG)
					ptr->property = 'z';
				else if (bptr->flag & BOARD_POST_FLAG)
					ptr->property = 'p';
				else if (bptr->flag & BOARD_NOREPLY_FLAG)
					ptr->property = 'x';
				else if (bptr->level != 0)
					ptr->property = 'r';
				else
					ptr->property = ' ';
			}
		}
	}

	// Load custom dirs.
	if (cbrd->goodbrd) {
		for (n = 0; n < cbrd->gnum && n < GOOD_BRC_NUM; n++) {
			gptr = cbrd->gbrds + n;
			if ((gptr->flag & BOARD_CUSTOM_FLAG)
					&& (gptr->pid == cbrd->parent)) {
				ptr = cbrd->brds + cbrd->num++;
				ptr->name = gptr->filename;
				ptr->title = gptr->title;
				ptr->BM = NULL;
				ptr->flag = gptr->flag;
				ptr->parent = gptr->pid;
				ptr->pos = gptr->id;
				ptr->zap = 0;
				ptr->total = 0;
				ptr->property = ' ';
			}
		}
	}

	if (cbrd->num == 0 && !cbrd->yank && cbrd->parent == -1) {
		cbrd->num = -1;
		cbrd->yank = true;
		return -1;
	}

	qsort(cbrd->brds, cbrd->num, sizeof(cbrd->brds[0]), cbrd->cmp);

	if (cbrd->newflag) {
		// Jump to first unread board.
		int i;
		for (i = 0; i < cbrd->num; ++i) {
			ptr = cbrd->brds + i;
			if (!(ptr->flag & BOARD_DIR_FLAG)) {
				if (ptr->total == -1)
					check_newpost(ptr);
				if (ptr->unread)
					break;
			}
		}
		if (i < cbrd->num)
			cp->cur = i;
	}
		
	cp->all= cbrd->num;
	return 0;
}

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

/**
 *
 */
static int choose_board_display(choose_t *cp)
{
	choose_board_t *cbrd = cp->data;
	board_data_t *ptr;
	char tmpBM[BM_LEN - 1];

	char cate[7], title[STRLEN];

	int n;
	for (n = cp->start; n < cp->start + BBS_PAGESIZE; n++) {
		if (n >= cbrd->num) {
			prints("\n");
			continue;
		}
		ptr = cbrd->brds + n;
		if (ptr->total == -1) {
			refresh();
			check_newpost(ptr);
		}

		if (!cbrd->newflag)
			prints(" %5d", n + 1);
		else if (ptr->flag & BOARD_DIR_FLAG)
			prints("  目录");
		else
			prints(" %5d", ptr->total);

		if (ptr->flag & BOARD_DIR_FLAG)
			prints("  ＋");
		else
			prints("  %s", ptr->unread ? "◆" : "◇");

		if (!(ptr->flag & BOARD_CUSTOM_FLAG))
			strcpy(tmpBM, ptr->BM);

		strlcpy(cate, ptr->title + 1, sizeof(cate));
		strlcpy(title, ptr->title + 11, sizeof(title));
		ellipsis(title, 20);

		prints("%c%-17s %s%s%6s %-20s %c ",
				(ptr->zap && !(ptr->flag & BOARD_NOZAP_FLAG)) ? '*' : ' ',
				ptr->name,
				(ptr->flag & BOARD_VOTE_FLAG) ? "\033[1;31mV\033[m" : " ",
				(ptr->flag & BOARD_CLUB_FLAG) ? (ptr->flag & BOARD_READ_FLAG)
				? "\033[1;31mc\033[m" : "\033[1;33mc\033[m" : " ",
				cate, title, HAS_PERM (PERM_POST) ? ptr->property : ' ');

		if (ptr->flag & BOARD_DIR_FLAG)
			prints("[目录]\n");
		else
			prints("%-12s %4d\n",
					ptr->BM[0] <= ' ' ? "诚征版主中" : strtok(tmpBM, " "),
					brdshm->bstatus[ptr->pos].inboard);
	}
	return 0;
}

static int board_cmp_flag(const void *brd1, const void *brd2)
{
	const board_data_t *b1 = brd1, *b2 = brd2;
	return strcasecmp(b1->name, b2->name);
}

static int board_cmp_online(const void *brd1, const void *brd2)
{
	const board_data_t *b1 = brd1, *b2 = brd2;
	return brdshm->bstatus[b2->pos].inboard - brdshm->bstatus[b1->pos].inboard;
}

static int board_cmp_default(const void *brd1, const void *brd2)
{
	const board_data_t *b1 = brd1, *b2 = brd2;
	int type = b1->title[0] - b2->title[0];
	if (type == 0)
		type = strncasecmp(b1->title + 1, b2->title + 1, 6);
	if (type == 0)
		type = strcasecmp(b1->name, b2->name);
	return type;
}

static int show_board_info(const char *board)
{
	int i;
	struct boardheader *bp;
	struct bstat *bs;
	char secu[40];
	bp = getbcache(board);
	bs = getbstat(board);
	clear();
	prints("版面详细信息:\n\n");
	prints("number  :     %d\n", getbnum(bp->filename, &currentuser));
	prints("英文名称:     %s\n", bp->filename);
	prints("中文名称:     %s\n", (HAS_PERM(PERM_SPECIAL0)) ? bp->title
			: (bp->title + 11));
	prints("版    主:     %s\n", bp->BM);
	prints("所属讨论区:   %s\n", bp->group ? bcache[bp->group - 1].filename
			: "无");
	prints("是否目录:     %s\n", (bp->flag & BOARD_DIR_FLAG) ? "目录" : "版面");
	prints("可以 ZAP:     %s\n", (bp->flag & BOARD_NOZAP_FLAG) ? "不可以"
			: "可以");

	if (!(bp->flag & BOARD_DIR_FLAG)) {
		prints("在线人数:     %d 人\n", bs->inboard);
		prints("文 章 数:     %s\n", (bp->flag & BOARD_JUNK_FLAG) ? "不计算"
				: "计算");
		prints("可以回复:     %s\n", (bp->flag & BOARD_NOREPLY_FLAG) ? "不可以"
				: "可以");
		//prints ("可以 ZAP:     %s\n",
		//	    (bp->flag & BOARD_NOZAP_FLAG) ? "不可以" : "可以");
		prints("匿 名 版:     %s\n", (bp->flag & BOARD_ANONY_FLAG) ? "是"
				: "否");
#ifdef ENABLE_PREFIX
		prints ("强制前缀:     %s\n",
				(bp->flag & BOARD_PREFIX_FLAG) ? "必须" : "不必");
#endif
		prints("俱 乐 部:     %s\n", (bp->flag & BOARD_CLUB_FLAG) ? (bp-> flag
				& BOARD_READ_FLAG) ? "读限制俱乐部" : "普通俱乐部" : "非俱乐部");
		prints("now id  :     %d\n", bs->nowid);
		prints("读写限制:     %s\n", (bp->flag & BOARD_POST_FLAG) ? "限制发文"
				: (bp->level ==0) ? "没有限制" : "限制阅读");
	}
	if (HAS_PERM(PERM_SPECIAL0) && bp->level != 0) {
		prints("权    限:     ");
		strcpy(secu, "ltmprbBOCAMURS#@XLEast0123456789");
		for (i = 0; i < 32; i++) {
			if (!(bp->level & (1 << i)))
				secu[i] = '-';
			else {
				prints("%s\n              ", permstrings[i]);
			}

		}
		prints("\n权 限 位:     %s\n", secu);
	}

	prints("URL 地址:     http://"BBSHOST"/bbs/doc?bid=%d\n",
			bp - bcache + 1);
	pressanykey();
	return FULLUPDATE;

}

static int choose_board(choose_board_t *cbrd);

/**
 *
 */
static int choose_board_read(choose_t *cp)
{
	choose_board_t *cbrd = cp->data;
	board_data_t *ptr = cbrd->brds + cp->cur;
	if (ptr->flag & BOARD_DIR_FLAG) {
		int parent = cbrd->parent;
		char *prefix = cbrd->prefix;
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
		Read();
		brc_zapbuf(cbrd->zapbuf + ptr->pos);
		ptr->total = -1;
		currBM[0] = '\0';
	}
	return FULLUPDATE;
}

/**
 *
 */
static int choose_board_init(choose_board_t *cbrd)
{
	if (cbrd->recursive)
		return 0;

	cbrd->brds = malloc(sizeof(board_data_t) * MAXBOARD);
	if (cbrd->brds == NULL)
		return -1;

	cbrd->gbrds = malloc(sizeof(*cbrd->gbrds) * GOOD_BRC_NUM);
	if (cbrd->gbrds == NULL) {
		free(cbrd->brds);
		return -1;
	}

	if (load_zapbuf(cbrd) != 0) {
		free(cbrd->brds);
		free(cbrd->gbrds);
		return -1;
	}

	cbrd->num = 0;
	if (!strcmp(currentuser.userid, "guest"))
		cbrd->yank = true;

	char flag = currentuser.flags[0];
	if (flag & BRDSORT_FLAG)
		cbrd->cmp = board_cmp_flag;
	else if (flag & BRDSORT_ONLINE)
		cbrd->cmp = board_cmp_online;
	else if (flag & BRDSORT_UDEF)
		cbrd->cmp = board_cmp_default;
	else if (flag & BRDSORT_UPDATE)
		cbrd->cmp = board_cmp_default;
	else
		cbrd->cmp = board_cmp_default;

	return 0;
}

/**
 *
 */
static void choose_board_title(choose_t *cp)
{
	choose_board_t *cbrd = cp->data;

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
			cbrd->newflag ? "全 部  未" : "编 号  未", "中  文  叙  述");
}

static int choose_board_handler(choose_t *cp, int ch)
{
	choose_board_t *cbrd = cp->data;
	board_data_t *ptr;
	char ans[2];
	bool modify_mode = false;

	switch (ch) {
		case '*':
			if (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG)
				return DONOTHING;
			ptr = cbrd->brds + cp->cur;
			show_board_info(ptr->name);
			return FULLUPDATE;
		case 'C':
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			if ((cbrd->gnum == 0) && (cbrd->parent == -1))
				return DONOTHING;
			if (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG)
				return DONOTHING;
			strlcpy(cbrd->buf, cbrd->brds[cp->cur].name, sizeof(cbrd->buf));
			presskeyfor("版名已复制 请按P粘贴", t_lines - 1);
			return DONOTHING;
		case 'c':
			cbrd->newflag = !cbrd->newflag;
			return PARTUPDATE;
		case 'L':
			m_read();
			cp->valid = false;
			modify_mode = true;
			break;
		case 'M':
			m_new();
			cp->valid = false;
			modify_mode = true;
			break;
		case 'u':
			modify_user_mode(QUERY);
			t_query();
			modify_mode = true;
			break;
		case 'H':
			getdata(t_lines - 1, 0, "您选择? (1) 本日十大  (2) 系统热点 [1]",
					ans, 2, DOECHO, YEA);
			if (ans[0] == '2')
				show_help("etc/hotspot");
			else
				show_help("0Announce/bbslist/day");
			break;
		case 'l':
			msg_more();
			modify_mode = true;
			break;
		case 'P':
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			if ((cbrd->gnum == 0) && (cbrd->parent == -1))
				return DONOTHING;
			goodbrd_add(cbrd, cbrd->buf, cbrd->parent);
			*cbrd->buf='\0';
			cp->valid = false;
			break;
		case '!':
			save_zapbuf(cbrd);
			free(cbrd->brds);
			free(cbrd->gbrds);
			Goodbye();
			return -1;
		case 'h':
			show_help("help/boardreadhelp");
			break;
		case '/':
			// TODO: search.
			break;
		case 's':
			if (currentuser.flags[0] & BRDSORT_FLAG) {
				currentuser.flags[0] ^= BRDSORT_FLAG;
				currentuser.flags[0] |= BRDSORT_ONLINE;
				cbrd->cmp = board_cmp_online;
			} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
				currentuser.flags[0] ^= BRDSORT_ONLINE;
				cbrd->cmp = board_cmp_default;
			} else {
				currentuser.flags[0] |= BRDSORT_FLAG;
				cbrd->cmp = board_cmp_flag;
			}
			qsort(cbrd->brds, cbrd->num, sizeof(*cbrd->brds), cbrd->cmp);
			substitut_record(PASSFILE, &currentuser,
					sizeof(currentuser), usernum);
			return PARTUPDATE;
		case 'y':
			if (cbrd->gnum)
				return DONOTHING;
			cbrd->yank = !cbrd->yank;
			cp->valid = false;
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
			cp->valid = false;
			return PARTUPDATE;
		case 'a':
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			if ((cbrd->gnum) && (cbrd->parent == -1))
				return DONOTHING;
			if (cbrd->gnum >= GOOD_BRC_NUM) {
				presskeyfor("个人热门版数已经达上限", t_lines - 1);
				return MINIUPDATE;
			} else if (cbrd->gnum) {
				int pos;
				char bname[STRLEN];
				struct boardheader fh;
				if (gettheboardname(1, "输入讨论区名 (按空白键自动搜寻): ",
						&pos, &fh, bname, 1)) {
					if (!inGoodBrds(cbrd, getbnum(bname, &currentuser)-1)) {
						goodbrd_add(cbrd, bname, cbrd->parent);
						cp->valid = false;
					}
				}
			} else {
				goodbrd_load(cbrd);
				if (cbrd->gnum >= GOOD_BRC_NUM) {
					presskeyfor("个人热门版数已经达上限", t_lines - 1);
					return MINIUPDATE;
				} else if (!inGoodBrds(cbrd, getbnum(cbrd->brds[cp->cur].name, &currentuser)-1)) {
					sprintf(genbuf, "您确定要添加%s到收藏夹吗?", cbrd->brds[cp->cur].name);
					if (askyn(genbuf, NA, YEA) == YEA) {
						goodbrd_add(cbrd, cbrd->brds[cp->cur].name, 0);
					}
					cp->valid = false;
				}
			}
			return FULLUPDATE;
		case 'A':
			//added by cometcaptor 2007-04-22 这里写入的是创建自定义目录的代码
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			if (cbrd->parent == 0) {
				if (cbrd->gnum >= GOOD_BRC_NUM) {
					presskeyfor("个人热门版数已经达上限", t_lines - 1);
					return MINIUPDATE;
				} else {
					//要求输入目录名
					char dirname[STRLEN];
					char dirtitle[STRLEN];
					dirname[0] = '\0';
					getdata(t_lines - 1, 0, "创建自定义目录: ", dirname, 17,
							DOECHO, NA); //设成17是因为只显示16个字符，为了防止创建的目录名和显示的不同而限定
					if (dirname[0] != '\0') {
						strcpy(dirtitle, "自定义目录");
						getdata(t_lines - 1, 0, "自定义目录描述: ", dirtitle,
								21, DOECHO, NA);
						goodbrd_mkdir(cbrd, dirname, dirtitle, 0);
					}
					cp->valid = false;
				}
				return PARTUPDATE;
			}
			return DONOTHING;
		case 'T':
			//added by cometcaptor 2007-04-25 修改自定义目录名
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			if ((cbrd->parent == 0)&& cbrd->num
					&& (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG)) {
				char dirname[STRLEN];
				char dirtitle[STRLEN];
				int gbid = 0;
				for (gbid = 0; gbid < cbrd->gnum; gbid++)
					if (cbrd->gbrds[gbid].id == cbrd->brds[cp->cur].pos)
						return DONOTHING;
				if (gbid == cbrd->gnum)
					return DONOTHING;
				strcpy(dirname, cbrd->gbrds[gbid].filename);
				getdata(t_lines - 1, 0, "修改自定义目录名: ", dirname, 17,
						DOECHO, NA);
				if (dirname[0] != '\0') {
					strcpy(dirtitle, cbrd->gbrds[gbid].title+11);
					getdata(t_lines - 1, 0, "自定义目录描述: ", dirtitle, 21,
							DOECHO, NA);
					if (dirtitle[0] == '\0')
						strcpy(dirtitle, cbrd->gbrds[gbid].title+11);
					strcpy(cbrd->gbrds[gbid].filename, dirname);
					strcpy(cbrd->gbrds[gbid].title+11, dirtitle);
					goodbrd_save(cbrd);
				}
				cp->valid = false;
				return PARTUPDATE;
			}
			return DONOTHING;
		case 'd':
			if ((cbrd->gnum) && (cbrd->num > 0)) {
				int i, pos;
				char ans[5];
				sprintf(genbuf, "要把 %s 从收藏夹中去掉？[y/N]",
						cbrd->brds[cp->cur].name);
				getdata(t_lines - 1, 0, genbuf, ans, 2, DOECHO, YEA);
				if (ans[0] == 'y' || ans[0] == 'Y') {
					if (cbrd->brds[cp->cur].flag & BOARD_CUSTOM_FLAG)
						goodbrd_rmdir(cbrd, cbrd->brds[cp->cur].pos);
					else {
						pos = inGoodBrds(cbrd, cbrd->brds[cp->cur].pos);
						if (pos) {
							for (i = pos-1; i < cbrd->gnum-1; i++)
								memcpy(cbrd->gbrds + i, cbrd->gbrds + i + 1,
										sizeof(struct goodbrdheader));
							cbrd->gnum--;
						}
						goodbrd_save(cbrd);
					}
					cp->valid = false;
					return PARTUPDATE;
				} else {
					return MINIUPDATE;
				}
			}
			return DONOTHING;
		case '\r':
		case '\n':
		case KEY_RIGHT:
			if (cbrd->num > 0)
				choose_board_read(cp);
			cp->valid = false;
			modify_mode = true;
			break;
		case 'S':
			if (!HAS_PERM(PERM_TALK))
				return DONOTHING;
			s_msg();
			modify_mode = true;
			break;
		case 'o':
			if (!HAS_PERM(PERM_LOGIN))
				return DONOTHING;
			online_users_show_override();
			modify_mode = true;
			break;
		default:
			return DONOTHING;
	}
	if (modify_mode)
		modify_user_mode(cbrd->newflag ? READNEW : READBRD);
	return FULLUPDATE;
}

/**
 *
 */
static int choose_board(choose_board_t *cbrd)
{
	choose_board_init(cbrd);

	choose_t cs;
	cs.data = cbrd;
	cs.loader = choose_board_load;
	cs.title = choose_board_title;
	cs.display = choose_board_display;
	cs.handler = choose_board_handler;

	modify_user_mode(cbrd->newflag ? READNEW : READBRD);

	choose2(&cs);

	if (!cbrd->recursive) {
		clear();
		save_zapbuf(cbrd);
		free(cbrd->brds);
		free(cbrd->zapbuf);
		free(cbrd->gbrds);
	}
	return 0;
}

void board_read_group(const char *cmd)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "EGROUP%c", *cmd);

	choose_board_t cbrd;
	memset(&cbrd, 0, sizeof(cbrd));
	cbrd.prefix = sysconf_str(buf);
	cbrd.newflag = DEFINE(DEF_NEWPOST);
	cbrd.parent = -1;

	choose_board(&cbrd);
}

void board_read_all(void)
{
	choose_board_t cbrd;
	memset(&cbrd, 0, sizeof(cbrd));
	cbrd.parent = -1;
	choose_board(&cbrd);
}

void board_read_new(void)
{
	choose_board_t cbrd;
	memset(&cbrd, 0, sizeof(cbrd));
	cbrd.parent = -1;
	cbrd.newflag = true;
	choose_board(&cbrd);
}

void goodbrd_show(void)
{
	if (!strcmp(currentuser.userid, "guest"))
		return;

	choose_board_t cbrd;
	memset(&cbrd, 0, sizeof(cbrd));
	cbrd.newflag = true;
	cbrd.goodbrd = true;

	choose_board(&cbrd);
}
