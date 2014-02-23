#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include "bbs.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/record.h"

enum {
	RECORD_BUFFER_SIZE = 8192,
};

/**
 * 打开记录文件
 * @param[in] file 文件名
 * @param[in] cmp 记录比较函数
 * @param[in] rlen 单条记录长度
 * @param[in] rdonly 是否以只读模式打开
 * @param[out] rec 记录文件数据结构
 * @return 成功返回文件描述符, 否则-1
 */
int record_open(const char *file, record_cmp_t cmp, int rlen,
		record_perm_e rdonly, record_t *rec)
{
	if (!file || !rec)
		return -1;

	rec->rlen = rlen;
	rec->cmp = cmp;

	if (rdonly)
		return rec->fd = open(file, O_RDONLY);

	int fd = open(file, O_RDWR);
	if (fd >= 0) {
		rec->fd = fd;
		return fd;
	} else if (errno == ENOENT) {
		fd = open(file, O_RDWR | O_CREAT | O_EXCL, 0644);
		rec->fd = fd;
		return fd;
	} else {
		return -1;
	}
}

/**
 * 关闭记录文件
 * @param[in] rec 记录文件数据结构
 * @return 成功返回0, 否则-1
 */
int record_close(record_t *rec)
{
	return rec ? file_close(rec->fd) : -1;
}

/**
 * 获取记录条数
 * @param[in] rec 记录文件数据结构
 * @return 记录条数
 */
int record_count(record_t *rec)
{
	struct stat st;
	if (!rec || fstat(rec->fd, &st) < 0)
		return 0;
	return st.st_size / rec->rlen;
}

/**
 * 锁定/解锁记录文件
 * @param[in] rec 记录文件数据结构
 * @param[in] type 锁的类型
 * @param[in] offset 锁的起始偏移量, 以记录为单位
 * @param[in] whence 偏移量的基准
 * @param[in] count 锁的长度, 以记录为单位
 * @return 成功返回0, 否则-1
 */
int record_lock(record_t *rec, record_lock_e type, int offset,
		record_whence_e whence, int count)
{
	if (!rec)
		return -1;
	return file_lock(rec->fd, (file_lock_e) type, offset * rec->rlen,
			(file_whence_e) whence, count * rec->rlen);
}

/**
 * 锁定/解锁整个记录文件
 * @param[in] rec 记录文件数据结构
 * @param[in] type 锁的类型
 * @return 成功返回0, 否则-1
 */
int record_lock_all(record_t *rec, record_lock_e type)
{
	return file_lock(rec->fd, (file_lock_e) type, 0, FILE_SET, 0);
}

int record_try_lock_all(record_t *rec, record_lock_e type)
{
	return file_try_lock_all(rec->fd, (file_lock_e) type);
}

/**
 * 设置记录文件读写位置
 * @param[in] rec 记录文件数据结构
 * @param[in] offset 偏移量, 以记录为单位
 * @param[in] whence 偏移量的基准
 * @return 成功时返回设置后的偏移量, 以记录为单位, 文件头为基准. 否则-1
 */
int record_seek(record_t *rec, int offset, record_whence_e whence)
{
	if (!rec)
		return -1;
	off_t off = lseek(rec->fd, offset * rec->rlen, whence);
	return off < 0 ? -1 : off / rec->rlen;
}

/**
 * 从当前位置读取记录文件
 * @param[in] rec 记录文件数据结构
 * @param[out] ptr 缓冲区
 * @param[in] count 要读取的记录条数
 * @return 成功时返回读取记录条数, 否则-1
 */
int record_read(record_t *rec, void *ptr, int count)
{
	if (!rec || !ptr || count < 0)
		return -1;
	int bytes = file_read(rec->fd, ptr, count * rec->rlen);
	return bytes < 0 ? -1 : bytes / rec->rlen;
}
/**
 * 从指定位置读取记录文件
 * @param[in] rec 记录文件数据结构
 * @param[out] ptr 缓冲区
 * @param[in] count 要读取的记录条数
 * @param[in] offset 偏移量, 以记录为单位, 文件头为基准
 * @return 返回成功读取记录条数
 */
int record_read_after(record_t *rec, void *ptr, int count, int offset)
{
	if (!ptr || count < 0 || record_seek(rec, offset, RECORD_SET) < 0)
		return 0;
	count = record_read(rec, ptr, count);
	return count < 0 ? 0 : count;
}

/**
 * 在指定位置写入记录文件
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 缓冲区
 * @param[in] count 要写入的记录条数
 * @param[in] offset 偏移量, 以记录为单位, 文件头为基准
 * @return 返回成功写入的记录条数
 */
int record_write(record_t *rec, const void *ptr, int count, int offset)
{
	if (!ptr || count < 0 || record_seek(rec, offset, RECORD_SET) < 0)
		return 0;
	int bytes = file_write(rec->fd, ptr, count * rec->rlen);
	return bytes < 0 ? 0 : bytes / rec->rlen;
}

/**
 * 向记录文件追加记录
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 缓冲区
 * @param[in] count 要追加的记录条数
 * @return 成功返回写入的记录条数, 否则-1
 * @note 本方法未加锁, 调用者应自行处理
 */
int record_append(record_t *rec, const void *ptr, int count)
{
	if (!rec || !ptr || count < 0 || lseek(rec->fd, 0, SEEK_END) < 0)
		return -1;
	return file_write(rec->fd, ptr, count * rec->rlen);
}

/**
 * 向记录文件追加记录(有锁版本)
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 缓冲区
 * @param[in] count 要追加的记录条数
 * @return 成功返回写入的记录条数, 否则-1
 */
int record_append_locked(record_t *rec, const void *ptr, int count)
{
	if (record_lock_all(rec, RECORD_WRLCK) < 0)
		return -1;
	int ret = record_append(rec, ptr, count);
	record_lock_all(rec, RECORD_UNLCK);
	return ret;
}

static int check_offset(const record_t *rec, const mmap_t *m, void *ptr,
		int offset)
{
	const char *begin = m->ptr;
	if (!ptr)
		return offset; // bypass check
	if (offset && offset * rec->rlen < m->size
			&& rec->cmp(begin + offset * rec->rlen, ptr) <= 0)
		return offset;
	return 0;
}

/**
 * 对记录文件逐条用指定函数进行修改/删除
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 当前记录, 可为NULL
 * @param[in] offset 当前偏移量, 以记录为单位, 文件头为基准
 * @param[in] callback 回调函数
 * @param[in] args 给回调函数的参数
 * @param[in] _delete 是否删除匹配的记录
 * @return 匹配的记录数
 */
int record_apply(record_t *rec, void *ptr, int offset,
		record_callback_t callback, void *args, bool _delete)
{
	if (!rec || !callback)
		return 0;

	mmap_t m = { .oflag = O_RDWR, .fd = rec->fd };
	if (mmap_open_fd(&m) < 0)
		return -1;

	offset = check_offset(rec, &m, ptr, offset);

	int affected = 0, len = rec->rlen;
	char *p = m.ptr, *end = m.ptr;
	p += offset * rec->rlen;
	end += m.size;

	for (; p < end; p += len, ++offset) {
		int r = callback(p, args, offset);
		if (r == RECORD_CALLBACK_MATCH) {
			++affected;
		} else if (_delete)
			memcpy(p - affected * len, p, len);
		else if (r == RECORD_CALLBACK_BREAK)
			break;
	}

	if (affected && _delete)
		file_truncate(rec->fd, m.size - affected * len);
	mmap_unmap(&m);
	return affected;
}

/**
 * 对记录文件逐条执行回调函数
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 当前记录, 可为NULL
 * @param[in] offset 当前偏移量, 以记录为单位, 文件头为基准
 * @param[in] callback 回调函数
 * @param[in] args 给回调函数的参数
 * @return 匹配的记录数
 */
int record_foreach(record_t *rec, void *ptr, int offset,
		record_callback_t callback, void *args)
{
	if (!rec || !callback)
		return 0;
	char buf[RECORD_BUFFER_SIZE];
	int block = sizeof(buf) / rec->rlen, matched = 0;
	bool checked = !ptr;

	record_seek(rec, offset, RECORD_SET);
	int count = 0;
	do {
		count = record_read(rec, buf, block);
		if (count < 0)
			return matched;
		if (!checked) {
			checked = true;
			if (!count || rec->cmp(buf, ptr) > 0) {
				record_seek(rec, 0, RECORD_SET);
				offset = 0;
				continue;
			}
		}

		for (int i = 0; i < count; ++i) {
			char *p = buf + i * rec->rlen;
			int r = callback(p, args, offset++);
			if (r == RECORD_CALLBACK_MATCH) {
				++matched;
			} else if (r == RECORD_CALLBACK_BREAK) {
				return matched;
			}
		}
	} while (count == block);
	return matched;
}

/**
 * 对记录文件从末尾开始逐条执行回调函数
 * @param[in] rec 记录文件数据结构
 * @param[in] callback 回调函数
 * @param[in] args 给回调函数的参数
 * @return 匹配的记录数
 */
int record_reverse_foreach(record_t *rec, record_callback_t callback,
		void *args)
{
	if (!rec || !callback)
		return 0;
	char buf[RECORD_BUFFER_SIZE];
	int block = sizeof(buf) / rec->rlen, matched = 0;

	int offset = record_count(rec);
	do {
		offset -= block;
		if (offset < 0)
			offset = 0;

		int count = record_read_after(rec, buf, block, offset);
		if (count <= 0)
			return matched;

		for (int i = count - 1; i >= 0; --i) {
			char *p = buf + i * rec->rlen;
			int r = callback(p, args, offset + i);
			if (r == RECORD_CALLBACK_MATCH) {
				++matched;
			} else if (r == RECORD_CALLBACK_BREAK) {
				return matched;
			}
		}
	} while (offset > 0);
	return matched;
}

/**
 * 向记录文件中插入若干条记录, 并保持记录文件原有排序
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 缓冲区
 * @param[in] count 要写入的记录条数
 * @return 成功0, 否则-1
 */
int record_merge(record_t *rec, void *ptr, int count)
{
	if (!rec || !ptr || count <= 0)
		return 0;
	qsort(ptr, count, rec->rlen, rec->cmp);

	mmap_t m = { .oflag = O_RDWR, .fd = rec->fd };
	if (mmap_open_fd(&m) < 0)
		return -1;

	int rlen = rec->rlen;
	size_t size = m.size / rec->rlen;

	if (mmap_truncate(&m, m.size + count * rlen) < 0)
		return -1;

	char *src = (char *) m.ptr + (size - 1) * rec->rlen;
	char *dst = (char *) m.ptr + (size + count - 1) * rec->rlen;
	char *p2 = (char *) ptr + (count - 1) * rec->rlen;

	while (p2 >= (char *) ptr) {
		if (src < (char *) m.ptr || rec->cmp(src, p2) <= 0) {
			memcpy(dst, p2, rlen);
			p2 -= rlen;
		} else {
			memcpy(dst, src, rlen);
			src -= rlen;
		}
		dst -= rlen;
	}

	mmap_close(&m);
	return 0;
}

/**
 * 在记录文件中查找一条记录, 并返回该条记录
 * @param[in] rec 记录文件数据结构
 * @param[in] filter 回调函数
 * @param[in] fargs 给回调函数的参数
 * @param[in] offset 查找的起始偏移量(不包含本条)
 * @param[in] reverse 是否反向查找
 * @param[out] out 保存找到的一条记录
 * @return 所找到记录的偏移量, 以记录为单位, 文件头为基准. 未找到或出错返回-1
 */
int record_search_copy(record_t *rec, record_callback_t filter, void *fargs,
		int offset, bool reverse, void *out)
{
	if (!rec || !filter)
		return -1;
	char buf[RECORD_BUFFER_SIZE];
	int capacity = sizeof(buf) / rec->rlen, all;
	if (offset >= 0)
		all = reverse ? offset : record_count(rec) - offset - 1;
	else
		all = record_count(rec);
	if (all <= 0)
		return -1;

	int count, base;
	int rounds = (all + capacity - 1) / capacity;
	if (reverse) {
		for (int i = 0; i < rounds; ++i) {
			base = (rounds - 1 - i) * capacity;
			record_seek(rec, base, RECORD_SET);
			count = record_read(rec, buf, capacity);
			if (count <= 0)
				return -1;
			if (all - base < count)
				count = all - base;
			for (int j = count - 1; j >= 0; --j) {
				int r = filter(buf + j * rec->rlen, fargs, base + j);
				if (r == RECORD_CALLBACK_MATCH) {
					if (out)
						memcpy(out, buf + j * rec->rlen, rec->rlen);
					return base + j;
				} else if (r == RECORD_CALLBACK_BREAK)
					return -1;
			}
		}
	} else {
		if (++offset < 0)
			offset = 0;
		record_seek(rec, offset, RECORD_SET);
		for (int i = 0; i < rounds; ++i) {
			base = i * capacity + offset;
			count = record_read(rec, buf, capacity);
			if (count <= 0)
				return -1;
			for (int j = 0; j < count; ++j) {
				int r = filter(buf + j * rec->rlen, fargs, base + j);
				if (r == 0) {
					if (out)
						memcpy(out, buf + j * rec->rlen, rec->rlen);
					return base + j;
				} else if (r > 0)
					return -1;
			}
		}
	}
	return -1;
}

/**
 * 设置记录文件长度
 * @param[in] rec 记录文件数据结构
 * @param[in] count 文件长度, 以记录为单位
 * @return 成功0, 否则-1
 */
int record_truncate(record_t *rec, int count)
{
	return rec ? ftruncate(rec->fd, count * rec->rlen) : -1;
}

USE_TRY;

/**
 * Get number of records in file.
 * @param file file name.
 * @param size bytes of a record.
 * @return number of records in file on success, -1 on error.
 */
long get_num_records(const char *file, int size)
{
	struct stat st;
	if (stat(file, &st) == -1)
		return 0;
	return (st.st_size / size);
}

/**
 * Append a record to file.
 * @param file file name.
 * @param record starting address of record to append.
 * @param size bytes of a record.
 * @return 0 on success, -1 on error.
 */
int append_record(const char *file, const void *record, int size)
{
	int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd < 0)
		return -1;
	file_lock_all(fd, FILE_WRLCK);
	int ret = file_write(fd, record, size);
	file_lock_all(fd, FILE_UNLCK);
	close(fd);
	return ret;
}

//取得记录的句柄,并存放在rptr中
//	fd是文件的描述符,size表示记录的大小,id表示记录的位置
//	不成功时,返回-1 ; 成功时,返回0
static int get_record_handle(int fd, void *rptr, int size, int id)
{
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1)
		return -1;
	if (read(fd, rptr, size) != size)
		return -1;
	return 0;
}

//取得记录,filename表示文件名,其它参数见get_record_handle
int get_record(char *filename, void *rptr, int size, int id)
{
	int fd;
	int ret;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	ret = get_record_handle(fd, rptr, size, id);
	close(fd);
	return ret;
}

//在文件filename中的第id-1个记录处读取大小为size,数量为number的记录集
//	如果失败,返回-1,如果未能读取number个记录,则返回读取的记录数
int get_records(const char *filename, void *rptr, int size, int id,
		int number)
{
	int fd;
	int n;
	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	if (lseek(fd, (off_t) (size * (id - 1)), SEEK_SET) == -1) {
		close(fd);
		return 0;
	}
	if ((n = read(fd, rptr, size * number)) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return (n / size);
}

/**
 * Apply function to records.
 * @param file name of the file that holds all records.
 * @param func function to be applied.
 * @param size the length of a record, in bytes.
 * @param arg parameters to be passed to func.
 * @param copy whether to apply function to a copy of the original record.
 * @param reverse whether to apply functions in reverse order.
 * @param lock whether to lock the file. Choose 'false' when 'file' is a 
 *        fixed-length shared resource, 'true' otherwise.
 * @return 0 on success, -1 on error.
 */
int apply_record(const char *file, apply_func_t func, int size,
			void *arg, bool copy, bool reverse, bool lock)
{
	void *ptr, *buf = NULL;
	int count, i;
	mmap_t m;

	if (copy && (buf = malloc(size)) == NULL)
		return -1;

	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0) {
		free(buf);
		return -1;
	}
	if (!lock)
		mmap_lock(&m, FILE_UNLCK);
	count = m.size / size;
	if (reverse)
		ptr = (char *)m.ptr + (count - 1) * size;
	else
		ptr = m.ptr;
	for (i = 0; i < count; ++i) {
		if (copy)
			memcpy(buf, ptr, size);
		else
			buf = ptr;
		if ((*func)(buf, reverse ? count - i : i + 1, arg) == QUIT) {
			mmap_close(&m);
			if (copy)
				free(buf);
			return 0;
		}
		if (reverse)
			ptr = (char *)ptr - size;
		else
			ptr = (char *)ptr + size;
	}
	mmap_close(&m);
	if (copy)
		free(buf);
	return 0;
}

/**
 * Search records in file (linear).
 * @param file file name.
 * @param rptr ptr to store record hit.
 * @param size record length in bytes.
 * @param func search criterion.
 * @param arg parameters to func.
 * @return record number (1-based) on success, 0 on error.
 */
int search_record(const char *file, void *rptr, int size, record_func_t func,
		void *arg)
{
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return 0;
	int i, count = m.size / size;
	char *buf = m.ptr;
	for (i = 0; i < count; ++i) {
		if ((*func)(arg, buf)) {
			if (rptr != NULL)
				memcpy(rptr, buf, size);
			mmap_close(&m);
			return i + 1;
		}
		buf += size;
	}
	mmap_close(&m);
	return 0;
}

//	将filename文件第id个记录替换为rptr所指向的数据
int substitute_record(char *filename, void *rptr, int size, int id)
{
	struct flock ldata;
	int retval;
	int fd;

	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1)
		return -1;
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (id - 1);
	if ((retval = fcntl(fd, F_SETLKW, &ldata)) == -1) {//以互斥方式锁文件
		close(fd);
		return -1;
	}
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) { //无法到文件的指定位置
		ldata.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &ldata);
		close(fd);
		return -1;
	}

	file_write(fd, rptr, size);
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &ldata);
	close(fd);

	return 0;
}

int delete_record(const char *file, int size, int id,
		record_func_t check, void *arg)
{
	mmap_t m;
	int ret;

	if (id <= 0)
		return 0;
	BBS_TRY {
		m.oflag = O_RDWR;
		if (mmap_open(file, &m) < 0)
			BBS_RETURN(-1);
		ret = 0;
		int count = m.size / size;

		if (id * size> m.size) {
			ret = -2;
		} else {
			if (check) {
				if (!(*check)((char *)m.ptr + (id - 1) * size, arg)) {
					for (id = 1; id <= count; id++)
						if ((*check) ((char *)m.ptr + (id - 1) * size, arg))
							break;
					if (id > count)
						ret = -2;
				}
			}
		}
		if (ret == 0) {
			memmove((char *)m.ptr + (id - 1) * size,
					(char *)m.ptr + id * size, m.size - size * id);
			mmap_truncate(&m, m.size - size);
		}
	}
	BBS_CATCH {
		ret = -3;
	}
	BBS_END mmap_close(&m);

	return ret;
}

int insert_record(const char *file, int size, record_func_t check, void *arg)
{
	if (check == NULL || arg == NULL)
		return -1;
	mmap_t m;
	m.oflag = O_RDWR;
	if (mmap_open(file, &m) < 0)
		return -1;
	void *iter, *end;
	if (mmap_truncate(&m, m.size + size) < 0)
		return -1;
	end = (char *)m.ptr + m.size - size;
	for (iter = m.ptr; iter != end; iter = (char *)iter + size) {
		if (check(iter, arg))
			break;
	}
	if (iter != end)
		memmove((char *)iter + size, iter, 
				m.size - size - ((char *)iter - (char *)m.ptr));
	memcpy(iter, arg, size);
	mmap_close(&m);
	return 0;
}
