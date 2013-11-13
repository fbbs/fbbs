// Handle unread mark

#include <stdbool.h>
#include "bbs.h"
#include "fbbs/brc.h"
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
	brc_item_t items[BRC_MAXNUM];  ///< 存储已读记录的数组
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

	memcpy(brcp->items, ptr, size * sizeof(*brcp->items));
	return ptr + size * sizeof(*brcp->items);
}

/**
 * 将一段记录写入已读记录缓存
 * @param[out] ptr 指向已读记录缓存的指针
 * @param[in] brcp 版面已读记录指针
 * @return 写入后的末端指针
 */
static char *brc_put_record(char *ptr, const brc_t *brcp)
{
	if (brcp->size) {
		brc_size_t size = brcp->size;
		if (size > BRC_MAXNUM)
			size = BRC_MAXNUM;

		strlcpy(ptr, brcp->name, BRC_STRLEN);
		ptr += BRC_STRLEN;
		*ptr++ = size;
		memcpy(ptr, brcp->items, size * sizeof(*brcp->items));
		ptr += size * sizeof(*brcp->items);
	}
	return ptr;
}

static void brc_load(const char *uname, brc_buf_t *buf)
{
	char file[HOMELEN];
	sethomefile(file, uname, ".boardrc");
	int fd = open(file, O_RDONLY);
	if (fd != -1) {
		buf->size = file_read(fd, buf->ptr, sizeof(buf->ptr));
		close(fd);
	} else {
		buf->size = 0;
	}
}

static void brc_save(const char *uname, const brc_buf_t *buf)
{
	char file[HOMELEN];
	sethomefile(file, uname, ".boardrc");
	int fd = open(file, O_WRONLY | O_CREAT, 0644);
	if (fd != -1) {
		ftruncate(fd, 0);
		file_write(fd, buf->ptr, buf->size);
		close(fd);
	}
}

/**
 * 将已读记录写入磁盘.
 * @param uname 用户名
 * @param bname 版面名
 */
void brc_update(const char *uname, const char *bname)
{
	if (!brc.changed) {
		return;
	}

	char *ptr = brc_buf.ptr;
	if (brc.size) {
		ptr = brc_put_record(ptr, &brc);
	}

	brc_buf_t buf;
	brc_load(uname, &buf);
	if (buf.size > sizeof(buf.ptr) - BRC_ITEMSIZE)
		buf.size = sizeof(buf.ptr) - BRC_ITEMSIZE * 2 + 1;

	char *tmp = buf.ptr, *end = buf.ptr + buf.size;
	while (tmp < end && (*tmp >= ' ' && *tmp <= '~')) {
		brc_t tmp_brc;
		tmp = brc_get_record(tmp, &tmp_brc);
		if (!strneq(tmp_brc.name, bname, sizeof(tmp_brc.name))) {
			ptr = brc_put_record(ptr, &tmp_brc);
		}
	}
	brc_buf.size = (brc_size_t) (ptr - brc_buf.ptr);
	brc_save(uname, &brc_buf);
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
		brc_load(uname, &brc_buf);
	}

	char *ptr = brc_buf.ptr, *end = brc_buf.ptr + brc_buf.size;
	while (ptr < end && (*ptr >= ' ' && *ptr <= '~')) {
		ptr = brc_get_record(ptr, &brc);
		if (strneq(brc.name, bname, sizeof(brc.name))) {
			return brc.size;
		}
	}

	strlcpy(brc.name, bname, sizeof(brc.name));
	brc.items[0] = 1;
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

/**
 * 在当前版面已读标记中加入一项.
 * @param item 要加入的项目
 */
void brc_mark_as_read(brc_item_t item)
{
	if (!brc.size) {
		brc.items[brc.size++] = item;
		brc.changed = true;
		return;
	}

	for (int i = 0; i < brc.size; ++i) {
		if (item == brc.items[i]) {
			return;
		} else if (item > brc.items[i]) {
			if (brc.size < BRC_MAXNUM)
				brc.size++;
			memmove(brc.items + i + 1, brc.items + i,
					(brc.size - i - 1) * sizeof(*brc.items));
			brc.items[i] = item;
			brc.changed = true;
			return;
		}
	}
	if (brc.size < BRC_MAXNUM) {
		brc.items[brc.size++] = item;
		brc.changed = true;
	}
}

/**
 * 测试当前版面某一项目是否已读.
 * @param item 要测试的项目
 * @return 已读返回false, 未读返回true.
 */
bool brc_unread(brc_item_t item)
{
	if (!brc.size)
		return true;

	for (int i = 0; i < brc.size; ++i) {
		if (item > brc.items[i])
			return true;
		else if (item == brc.items[i])
			return false;
	}
	return false;
}

bool brc_unread_legacy(const char *filename)
{
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.')
		return false;
	return brc_unread(strtoul(filename + 2, NULL, 10));
}

/**
 * 获得当前版面已读的最新项目编号.
 * @return 当前版面已读的最新项目编号, 如果没有记录则返回0.
 */
brc_item_t brc_last_read(void)
{
	if (brc.size)
		return brc.items[0];
	return 0;
}

/**
 * 将指定项目以前的项目都标记为已读.
 * @param item 要标记的项目
 */
void brc_clear(brc_item_t item)
{
	for (int i = item - BRC_MAXNUM + 1; i <= item; ++i)
		brc_mark_as_read(i);
}

/**
 * 将当前版面的所有项目标记为已读.
 */
void brc_clear_all(void)
{
	brc_clear(fb_time());
}

void brc_zapbuf(int *zbuf)
{
	if (*zbuf > 0 && brc.size)
		*zbuf = brc.items[0];
}

/**
 * 判断一个版面是否有未读项目
 * @param uname 用户名
 * @param bname 版面名
 * @param bid 版面编号
 * @return 如果该版面有未读项目返回true, 否则返回false.
 */
bool brc_board_unread(const char *uname, const char *bname, int bid)
{
	brc_buf.ptr[0] = '\0';
	if (!brc_init(uname, bname)) {
		return true;
	} else {
		if (brc_unread(get_last_post_time(bid)))
			return true;
		return false;
	}
}

/** @} */
