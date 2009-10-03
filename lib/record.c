#include "bbs.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define BUFSIZE (MAXUSERS + 244)

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
	flock(fd, LOCK_EX);
	int ret = safer_write(fd, record, size);
	flock(fd, LOCK_UN);
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

#ifndef THREAD_C

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
	if (mmap_open(file, &m) < 0)
		return -1;
	if (!lock)
		mmap_lock(&m, LOCK_UN);
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
	/*     * add by KCN      */
	struct flock ldata;
	int retval;
	int fd;

	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1)
		return -1;
	/*
	 * change by KCN
	 * flock(fd,LOCK_EX) ;
	 */
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (id - 1);
	if ((retval = fcntl(fd, F_SETLKW, &ldata)) == -1) {//以互斥方式锁文件
		//bbslog("user", "%s", "reclock error");
		close(fd);
		/*---	period	2000-10-20	file should be closed	---*/
		return -1;
	}
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) { //无法到文件的指定位置
		// bbslog("user", "%s", "subrec seek err");
		/*---	period	2000-10-24	---*/
		ldata.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &ldata);
		close(fd);
		return -1;
	}

	safer_write(fd, rptr, size);
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
		if (id * size> m.size) {
			ret = -2;
		} else {
			if (check) {
				if (!(*check)((char *)m.ptr + (id - 1) * size, arg)) {
					for (id = 0; id * size < m.size; id++)
						if ((*check) ((char *)m.ptr + (id - 1) * size, arg))
							break;
					if (id * size >= m.size)
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
		memmove(iter + size, iter, 
				m.size - size - ((char *)iter - (char *)m.ptr));
	memcpy(iter, arg, size);
	mmap_close(&m);
	return 0;
}

#endif

/**
 * Linear search of an array.
 * @param[in] key object to match.
 * @param[in] base pointer to the initial member.
 * @param[in] nmemb number of objects in the array.
 * @param[in] size size of each object of the array.
 * @param[in] compar the comparator, see ::comparator_t.
 * @return a pointer to a matching member of the array, NULL if not found.
 */
void *lsearch(const void *key, const void *base, size_t nmemb, size_t size,
		comparator_t compar)
{
	const char *ptr = base;
	const char *end = ptr + nmemb * size;
	while (ptr != end) {
		if (!compar(key, ptr))
			return ptr;
		ptr += size;
	}
	return NULL;
}

/**
 * Open file as record stream.
 * @param[in] file file to open.
 * @param[in] mode open mode (O_RDONLY, O_WRONLY, O_RDWR).
 * @param[in,out] r pointer to a record stream.
 * @return 0 on success, -1 on error.
 */
int record_open(const char *file, int mode, record_t *r)
{
	r->oflag = mode;
	return mmap_open(file, r);
}

/**
 * Close a record stream.
 * @param[in] r pointer to a record stream.
 * @return 0 on success, -1 on error.
 */
int record_close(record_t *r)
{
	return mmap_close(r);
}

/**
 * Search for a specific record.
 * @param[in] r pointer to a record stream.
 * @param[in] key pointer to the object to match.
 * @param[in] size size of each record.
 * @param[in] method search method.
 * @param[in] compar compare function.
 * @return a pointer to a matching member of the array, NULL if not found.
 */
void *record_search(record_t *r, const void *key, size_t size,
		search_method_t method, comparator_t compar)
{
	if (r == NULL || key == NULL || method == NULL)
		return NULL;
	return (*method)(key, r->ptr, r->size / size, size, compar);
}

/**
 * Delete a specific record.
 * @param[in,out] r pointer to a record stream.
 * @param[in] ptr pointer to the record for deletion.
 * @param[in] size record size.
 * @return 0 on success, -1 on error.
 */
int record_delete(record_t *r, void *ptr, int size)
{
	if (r == NULL || ptr == NULL 
			|| (r->oflag != O_RDWR && r->oflag != O_WRONLY))
		return -1;
	if (ptr < r->ptr || ((char *)ptr + size > (char *)(r->ptr) + r->size))
		return -1;
	memmove(ptr, (char *)ptr + size,
			r->size - size - ((char *)ptr - (char *)r->ptr));
	if (mmap_shrink(r, r->size - size) < 0)
		return -1;
	return 0;
}
