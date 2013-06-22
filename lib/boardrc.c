// Handle unread mark

#include <stdbool.h>
#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

enum {
	BRC_MAXSIZE = 50000,
	BRC_MAXNUM = 60,
	BRC_STRLEN = 20,
	BRC_ITEMSIZE = BRC_STRLEN + 1 + BRC_MAXNUM * sizeof(int),
};

typedef uint32_t brc_item_t;

static struct {
	int size;
	int num;
	bool changed;
	char name[BRC_STRLEN];
	brc_item_t list[BRC_MAXNUM];
	char buf[BRC_MAXSIZE];
} brc;

static char *brc_get_record(char *ptr, char *name, int *pnum, brc_item_t *list)
{
	strlcpy(name, ptr, BRC_STRLEN);
	ptr += BRC_STRLEN;
	int num = (*ptr++) & 0xff;
	char *tmp = ptr + num * sizeof (int);
	if (num > BRC_MAXNUM)
		num = BRC_MAXNUM;
	*pnum = num;
	memcpy (list, ptr, num * sizeof (int));
	return tmp;
}

static char *brc_put_record(char *ptr, const char *name, int num,
		brc_item_t *list)
{
	if (num> 0) {
		if (num > BRC_MAXNUM)
			num = BRC_MAXNUM;
		strlcpy(ptr, name, BRC_STRLEN);
		ptr += BRC_STRLEN;
		*ptr++ = num;
		memcpy(ptr, list, num * sizeof(*list));
		ptr += num * sizeof(*list);
	}
	return ptr;
}

void brc_update(const char *userid, const char *board)
{
	char dirfile[STRLEN], *ptr;
	char tmp_buf[BRC_MAXSIZE], *tmp;
	char tmp_name[BRC_STRLEN];
	brc_item_t tmp_list[BRC_MAXNUM];
	int tmp_num;
	int fd, tmp_size;
	if (!brc.changed) {
		return;
	}
	ptr = brc.buf;
	if (brc.num > 0) {
		ptr = brc_put_record(ptr, brc.name, brc.num, brc.list);
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
		tmp = brc_get_record(tmp, tmp_name, &tmp_num, tmp_list);
		if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
			ptr = brc_put_record(ptr, tmp_name, tmp_num, tmp_list);
		}
	}
	brc.size = (int) (ptr - brc.buf);

	if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
		ftruncate(fd, 0);
		write(fd, brc.buf, brc.size);
		close(fd);
	}
	brc.changed = false;
}

int brc_initial(const char *userid, const char *board)
{
	char dirfile[STRLEN], *ptr;
	int fd;
	brc_update(userid, board);
	brc.changed = false;
	if (brc.buf[0] == '\0') {
		sethomefile(dirfile, userid, ".boardrc");
		if ((fd = open(dirfile, O_RDONLY)) != -1) {
			brc.size = read(fd, brc.buf, sizeof(brc.buf));
			close (fd);
		} else {
			brc.size = 0;
		}
	}
	ptr = brc.buf;
	while (ptr < &brc.buf[brc.size] && (*ptr >= ' ' && *ptr <= '~')) {
		ptr = brc_get_record(ptr, brc.name, &brc.num, brc.list);
		if (strncmp (brc.name, board, BRC_STRLEN) == 0) {
			return brc.num;
		}
	}
	strlcpy(brc.name, board, BRC_STRLEN);
	brc.list[0] = 1;
	brc.num = 1;
	return 0;
}

void brc_mark_as_read(int64_t id)
{
	int r = (int)id;

	if (brc.num <= 0) {
		brc.list[brc.num++] = r;
		brc.changed = true;
		return;
	}

	for (int i = 0; i < brc.num; ++i) {
		if (r == brc.list[i]) {
			return;
		} else if (r > brc.list[i]) {
			if (brc.num < BRC_MAXNUM)
				brc.num++;
			for (int j = brc.num - 1; j > i; --j) {
				brc.list[j] = brc.list[j - 1];
			}
			brc.list[i] = r;
			brc.changed = true;
			return;
		}
	}
	if (brc.num < BRC_MAXNUM) {
		brc.list[brc.num++] = r;
		brc.changed = true;
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

	if (brc.num <= 0)
		return true;

	for (int i = 0; i < brc.num; ++i) {
		if (r > brc.list[i])
			return true;
		else if (r == brc.list[i])
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
	if (brc.num > 0 && brc.num <= BRC_MAXNUM)
		return brc.list[brc.num - 1] + 1;
	return 1;
}

int brc_last_read(void)
{
	if (brc.num > 0)
		return brc.list[0];
	return 0;
}

void brc_clear(int64_t id)
{
	for (int i = id - BRC_MAXNUM + 1; i <= id; ++i)
		brc_mark_as_read(i);
}

void brc_clear_all(int bid)
{
	brc_clear(get_last_post_id(bid));
}

void brc_zapbuf(int *zbuf)
{
	if (*zbuf > 0 && brc.num > 0)
		*zbuf = brc.list[0];
}

int brc_fcgi_init(const char *user, const char *board)
{
	brc.buf[0] = '\0';
	return brc_initial(user, board);
}

bool brc_board_unread(const char *user, const char *bname, int bid)
{
	brc.buf[0] = '\0';
	if (!brc_initial(currentuser.userid, bname)) {
		return true;
	} else {
		if (brc_unread(get_last_post_id(bid)))
			return true;
		return false;
	}
}
