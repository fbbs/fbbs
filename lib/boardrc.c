// Handle unread mark

#include <stdbool.h>
#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

/** @defgroup readmark 已读标记 */
/** @{ */

enum {
	BRC_MAXSIZE = 50000,
	BRC_MAXNUM = 60,
	BRC_STRLEN = 20,
	BRC_ITEMSIZE = BRC_STRLEN + 1 + BRC_MAXNUM * sizeof(int),
};

typedef uint32_t brc_item_t;
typedef uint_t brc_size_t;

/** 所有已读记录的缓存 */
typedef struct {
	brc_size_t size; ///< 缓存已用长度
	char ptr[BRC_MAXSIZE];  ///< 缓存区
} brc_buf_t;

/** 一个版面的已读记录 */
typedef struct {
	brc_size_t size;  ///< 已读记录条数
	bool changed;  ///< 是否修改过
	char name[BRC_STRLEN];  ///< 索引名(版面名)
	brc_item_t list[BRC_MAXNUM];  ///< 存储已读记录的数组
} brc_t;

static brc_buf_t brc_buf; ///< 所有已读记录的缓存
static brc_t brc; ///< 当前的版面已读记录变量

/**
 * 读取已读记录缓存中的一段记录.
 * @param[in] ptr 指向已读记录缓存其中一段的指针
 * @param[out] brcp 版面已读记录指针
 * @return 指向下一段记录的指针
 */
static char *brc_get_record(char *ptr, brc_t *brcp)
{
	strlcpy(brcp->name, ptr, sizeof(brcp->name));
	ptr += BRC_STRLEN;

	brc_size_t size = *ptr++;
	if (size > BRC_MAXNUM)
		size = BRC_MAXNUM;
	brcp->size = size;

	memcpy(brcp->list, ptr, size * sizeof(brc_item_t));
	return ptr + size * sizeof(brc_item_t);
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
	int fd, tmp_size;
	if (!brc.changed) {
		return;
	}
	ptr = brc_buf.ptr;
	if (brc.size) {
		ptr = brc_put_record(ptr, brc.name, brc.size, brc.list);
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
		brc_size_t tmp_size;
		tmp = brc_get_record(tmp, tmp_name, &tmp_size, tmp_list);
		if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
			ptr = brc_put_record(ptr, tmp_name, tmp_size, tmp_list);
		}
	}
	brc_buf.size = (brc_size_t) (ptr - brc_buf.ptr);

	if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
		ftruncate(fd, 0);
		write(fd, brc_buf.ptr, brc_buf.size);
		close(fd);
	}
	brc.changed = false;
}

/**
 * 读入指定用户在指定版面的已读记录.
 * @param[in] uname 用户名
 * @param[in] bname 版面名
 * @return 如果该版之前没有记录, 返回0; 否则返回当前该版已有的记录条数.
 */
int brc_init(const char *uname, const char *bname)
{
	brc_update(uname, bname);
	brc.changed = false;
	if (brc_buf.ptr[0] == '\0') {
		char file[HOMELEN];
		sethomefile(file, uname, ".boardrc");
		int fd = open(file, O_RDONLY);
		if (fd != -1) {
			brc_buf.size = read(fd, brc_buf.ptr, sizeof(brc_buf.ptr));
			close(fd);
		} else {
			brc_buf.size = 0;
		}
	}

	char *ptr = brc_buf.ptr, *end = brc_buf.ptr + brc_buf.size;
	while (ptr < end && (*ptr >= ' ' && *ptr <= '~')) {
		ptr = brc_get_record(ptr, &brc);
		if (strneq(brc.name, bname, sizeof(brc.name))) {
			return brc.size;
		}
	}

	strlcpy(brc.name, bname, sizeof(brc.name));
	brc.list[0] = 1;
	brc.size = 1;
	return 0;
}

/**
 * @copydoc brc_init
 * 先清空已读记录缓存, 适用于web.
 * @see brc_init
 */
int brc_initialize(const char *uname, const char *bname)
{
	brc_buf.ptr[0] = '\0';
	return brc_init(uname, bname);
}

void brc_mark_as_read(int64_t id)
{
	int r = (int)id;

	if (!brc.size) {
		brc.list[brc.size++] = r;
		brc.changed = true;
		return;
	}

	for (int i = 0; i < brc.size; ++i) {
		if (r == brc.list[i]) {
			return;
		} else if (r > brc.list[i]) {
			if (brc.size < BRC_MAXNUM)
				brc.size++;
			for (int j = brc.size - 1; j > i; --j) {
				brc.list[j] = brc.list[j - 1];
			}
			brc.list[i] = r;
			brc.changed = true;
			return;
		}
	}
	if (brc.size < BRC_MAXNUM) {
		brc.list[brc.size++] = r;
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

	if (!brc.size)
		return true;

	for (int i = 0; i < brc.size; ++i) {
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
	if (brc.size && brc.size <= BRC_MAXNUM)
		return brc.list[brc.size - 1] + 1;
	return 1;
}

int brc_last_read(void)
{
	if (brc.size)
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
	if (*zbuf > 0 && brc.size)
		*zbuf = brc.list[0];
}

bool brc_board_unread(const char *user, const char *bname, int bid)
{
	brc_buf.ptr[0] = '\0';
	if (!brc_init(currentuser.userid, bname)) {
		return true;
	} else {
		if (brc_unread(get_last_post_id(bid)))
			return true;
		return false;
	}
}

/** @} */
