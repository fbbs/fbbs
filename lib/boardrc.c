// Handle unread mark

#include <stdbool.h>
#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"

#define BRC_MAXSIZE     50000
#define BRC_MAXNUM      60
#define BRC_STRLEN      20
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))

static char brc_buf[BRC_MAXSIZE];
static int brc_size, brc_changed = 0;
static char brc_name[BRC_STRLEN];
static int brc_list[BRC_MAXNUM], brc_num;

static char *brc_getrecord(char *ptr, char *name, int *pnum, int *list)
{
	int num;
	char *tmp;
	strlcpy(name, ptr, BRC_STRLEN);
	ptr += BRC_STRLEN;
	num = (*ptr++) & 0xff;
	tmp = ptr + num * sizeof (int);
	if (num > BRC_MAXNUM)
		num = BRC_MAXNUM;
	*pnum = num;
	memcpy (list, ptr, num * sizeof (int));
	return tmp;
}

static char *brc_putrecord(char *ptr, char *name, int num, int *list)
{
	if (num> 0) {
		if (num> BRC_MAXNUM)
			num = BRC_MAXNUM;
		strlcpy (ptr, name, BRC_STRLEN);
		ptr += BRC_STRLEN;
		*ptr++ = num;
		memcpy (ptr, list, num * sizeof (int));
		ptr += num * sizeof (int);
	}
	return ptr;
}

void brc_update(const char *userid, const char *board)
{
	char dirfile[STRLEN], *ptr;
	char tmp_buf[BRC_MAXSIZE], *tmp;
	char tmp_name[BRC_STRLEN];
	int tmp_list[BRC_MAXNUM], tmp_num;
	int fd, tmp_size;
	if (brc_changed == 0) {
		return;
	}
	ptr = brc_buf;
	if (brc_num > 0) {
		ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
	}
	if (1) {
		sethomefile(dirfile, userid, ".boardrc");
		if ((fd = open(dirfile, O_RDONLY)) != -1) {
			tmp_size = read(fd, tmp_buf, sizeof (tmp_buf));
			if (tmp_size > sizeof(tmp_buf) - BRC_ITEMSIZE)
				tmp_size = sizeof(tmp_buf) - BRC_ITEMSIZE*2 + 1;
			close(fd);
		} else {
			tmp_size = 0;
		}
	}
	tmp = tmp_buf;
	while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= '~')) {
		tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
		if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
			ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
		}
	}
	brc_size = (int) (ptr - brc_buf);

	if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
		ftruncate(fd, 0);
		write(fd, brc_buf, brc_size);
		close(fd);
	}
	brc_changed = 0;
}

int brc_initial(const char *userid, const char *board)
{
	char dirfile[STRLEN], *ptr;
	int fd;
	brc_update (userid, board);
	brc_changed = 0;
	if (brc_buf[0] == '\0') {
		sethomefile(dirfile, userid, ".boardrc");
		if ((fd = open (dirfile, O_RDONLY)) != -1) {
			brc_size = read (fd, brc_buf, sizeof (brc_buf));
			close (fd);
		} else {
			brc_size = 0;
		}
	}
	ptr = brc_buf;
	while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= '~')) {
		ptr = brc_getrecord (ptr, brc_name, &brc_num, brc_list);
		if (strncmp (brc_name, board, BRC_STRLEN) == 0) {
			return brc_num;
		}
	}
	strlcpy (brc_name, board, BRC_STRLEN);
	brc_list[0] = 1;
	brc_num = 1;
	return 0;
}

void brc_mark_as_read(int64_t id)
{
	int r = (int)id;

	if (brc_num <= 0) {
		brc_list[brc_num++] = r;
		brc_changed = 1;
		return;
	}

	for (int i = 0; i < brc_num; ++i) {
		if (r == brc_list[i]) {
			return;
		} else if (r > brc_list[i]) {
			if (brc_num < BRC_MAXNUM)
				brc_num++;
			for (int j = brc_num - 1; j > i; --j) {
				brc_list[j] = brc_list[j - 1];
			}
			brc_list[i] = r;
			brc_changed = 1;
			return;
		}
	}
	if (brc_num < BRC_MAXNUM) {
		brc_list[brc_num++] = r;
		brc_changed = 1;
	}
}

void brc_addlist_legacy(const char *filename)
{
	if (streq(currentuser.userid, "guest"))
		return;
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.')
		return;
	brc_mark_as_read(strtol(filename + 2, NULL, 10));
}

bool brc_unread(int64_t id)
{
	int r = (int)id;

	if (brc_num <= 0)
		return true;

	for (int i = 0; i < brc_num; ++i) {
		if (r > brc_list[i])
			return true;
		else if (r == brc_list[i])
			return false;
	}
	return false;
}

bool brc_unread_legacy(const char *filename)
{
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.')
		return false;
	return brc_unread(strtol(filename + 2, NULL, 10));
}

int brc_first_unread(void)
{
	if (brc_num > 0 && brc_num <= BRC_MAXNUM)
		return brc_list[brc_num - 1] + 1;
	return 1;
}

int brc_last_read(void)
{
	if (brc_num > 0)
		return brc_list[0];
	return 0;
}

void brc_clear(int64_t id)
{
	for (int i = id - BRC_MAXNUM + 1; i <= id; ++i)
		brc_mark_as_read(i);
}

void brc_clear_all(void)
{
	brc_clear(time(NULL));
}

void brc_zapbuf(int *zbuf)
{
	if (*zbuf > 0 && brc_num > 0)
		*zbuf = brc_list[0];
}

int brc_fcgi_init(const char *user, const char *board)
{
	brc_buf[0] = '\0';
	return brc_initial(user, board);
}

bool brc_board_unread(const char *user, const char *bname, int bid)
{
	brc_buf[0] = '\0';
	if (!brc_initial(currentuser.userid, bname)) {
		return true;
	} else {
		if (brc_unread((brdshm->bstatus[bid]).lastpost))
			return true;
		return false;
	}
}
