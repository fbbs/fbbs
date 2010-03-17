#include "bbs.h"

//定义页面大小
#define BBS_PAGESIZE (t_lines-4)

/**
 *
 */
static bool club_do_add_equal(const char *buf, size_t size, const char *str,
		size_t len)
{
	if (len >= size)
		return false;
	return (buf[len] == ' ' && !strncmp(buf, str, len));
}

/**
 *
 */
static int club_do_add(const char *user, const char *board, const char *ps)
{
	char buf[LINE_BUFSIZE], file[HOMELEN], title[STRLEN], msg[256];
	time_t now= time(NULL);
	struct tm* t = localtime(&now);
	snprintf(buf, sizeof(buf), "%-12s %-40s %04d.%02d.%02d %-12s\n", user, ps,
			1900 + t->tm_year, t->tm_mon + 1, t->tm_mday, currentuser.userid);

	setbfile(file, board, "club_users");
	int ret = add_to_file(file, buf, strlen(user), false, club_do_add_equal);
	if (ret < 0)
		return ret;

	bm_log(currentuser.userid, board, BMLOG_ADDCLUB, 1);

	snprintf(title, sizeof(title), "%s邀请%s加入俱乐部版%s",
			currentuser.userid, user, board);
	snprintf(msg, sizeof(msg), "%s:\n\n    您被邀请加入俱乐部版 %s\n\n补充说明"
			"：%s\n\n邀请人: %s\n", user, board, ps, currentuser.userid);
	autoreport(title, msg, YEA, user, 2);
	Poststring(msg, "club", title, 2);
	return 0;
}

/**
 *
 */
static int club_add(void)
{
	struct userec urec;
	char user[IDLEN + 1], ps[40], buf[STRLEN];
	move(1, 0);
	usercomplete("增加俱乐部成员: ", user);
	if (*user == '\0' || !getuserec(user, &urec))
		return -1;
	if (!strcasecmp(user, "guest")) {
		presskeyfor("不能邀请guest加入俱乐部", t_lines - 1);
		return -1;
	}
	getdata(1, 0, "输入补充说明:", ps, sizeof(ps), DOECHO, YEA);
	move(1, 0);
	snprintf(buf, sizeof(buf), "邀请 %s 加入俱乐部吗?", urec.userid);
	if (!askyn(buf, YEA, NA))
		return -1;
	return club_do_add(urec.userid, currboard, ps);
}

/**
 *
 */
static int club_del(const char *user, const char *board)
{
	char file[HOMELEN];
	setbfile(file, currboard, "club_users");
	if (del_from_file(file, user) < 0)
		return -1;

	bm_log(currentuser.userid, board, BMLOG_DELCLUB, 1);

	char title[STRLEN];
	char *msg = "";
	snprintf(title, sizeof(title), "%s取消%s在俱乐部版%s的权利",
			currentuser.userid, user, board);
	autoreport(title, msg, YEA, user, 2);
	Poststring(msg, "club", title, 2);
	return 0;
}

/**
 *
 */
static void club_title_show(void)
{
	move(0, 0);
	outs("\033[1;44;36m 设定俱乐部名单\033[K\033[m\n"
			"离开[\033[1;32m←\033[m] 选择[\033[1;32m↑\033[m,\033[1;32m↓"
			"\033[m] 添加[\033[1;32ma\033[m] 删除[\033[1;32md\033[m] 查找"
			"[\033[1;32m/\033[m]\n"
			"\033[1;44m 用户代号     附加说明                             "
			"    邀请日期   邀请人\033[K\033[m\n");
}

/**
 *
 */
static int club_key_deal(const char* fname, int ch, char* line)
{
	char user[IDLEN + 1], buf[STRLEN];
	if (line) {
		strlcpy(user, line, sizeof(user));
		strtok(user, " \n\r\t");
	}
	switch (ch) {
		case 'a':
			club_add();
			break;
		case 'd':
			if (!line)
				return 0;
			move(1, 0);
			snprintf(buf, sizeof(buf), "删除俱乐部成员%s吗?", user);
			if (!askyn(buf, NA, NA))
				return 1;
			club_del(user, currboard);
			break;
		case Ctrl('A'):
		case KEY_RIGHT: //用户信息
			if (!line)
				return 0;
			t_query(user);
			break;
		default:
			return 0;
	}
	return 1;
}

/**
 *
 */
int club_user(void)
{
	struct boardheader *bp = getbcache(currboard);
	if (!bp)
		return DONOTHING;
	char file[HOMELEN];
	if ((bp->flag & BOARD_CLUB_FLAG) && chkBM(currbp, &currentuser)) {
		setbfile(file, currboard, "club_users");
		list_text(file, club_title_show, club_key_deal, NULL);
		return FULLUPDATE;
	}
	return DONOTHING;
}

int bm_log(const char *user, const char *board, int type, int value)
{
	int fd, data[BMLOGLEN];
	struct flock ldata;
	struct stat buf;
	struct boardheader *btemp;
	char direct[STRLEN], BM[BM_LEN];
	char *ptr;

	btemp = getbcache(board);
	if (btemp == NULL)
		return 0;
	strlcpy(BM, btemp->BM, sizeof(BM) - 1);
	BM[sizeof(BM) - 1] = '\0';
	ptr = strtok(BM, ",: ;|&()\0\n");
	while (ptr) {
		if (!strcmp(ptr, currentuser.userid))
			break;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	if (!ptr)
		return 0;
	sprintf(direct, "boards/%s/.bm.%s", board, user);
	if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1)
		return 0;
	ldata.l_type = F_RDLCK;
	ldata.l_whence = 0;
	ldata.l_len = 0;
	ldata.l_start = 0;
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		close(fd);
		return 0;
	}
	fstat(fd, &buf);
	if (buf.st_size < BMLOGLEN * sizeof(int)) {
		memset(data, 0, sizeof(int) * BMLOGLEN);
	} else {
		read(fd, data, sizeof(int) * BMLOGLEN);
	}
	if (type >= 0 && type < BMLOGLEN)
		data[type] += value;
	lseek(fd, 0, SEEK_SET);
	write(fd, data, sizeof(int) * BMLOGLEN);
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &ldata);
	close(fd);
	return 0;
}
