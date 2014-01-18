#ifndef FB_RECORD_H
#define FB_RECORD_H

typedef enum {
	RECORD_RDLCK = 0, ///< 读锁 @see FILE_RDLCK
	RECORD_WRLCK = 1, ///< 写锁 @see FILE_WRLCK
	RECORD_UNLCK = 2, ///< 解锁 @see FILE_UNLCK
} record_lock_e;

typedef enum {
	RECORD_SET = 0, ///< 文件头 @see FILE_SET
	RECORD_CUR = 1, ///< 当前位置 @see FILE_CUR
	RECORD_END = 2, ///< 文件尾 @see FILE_END
} record_whence_e;

typedef enum {
	RECORD_READ = 1, ///< 只读模式
	RECORD_WRITE = 0, ///< 读写模式
} record_perm_e;

typedef enum {
	RECORD_CALLBACK_MATCH = 0, ///< 记录匹配
	RECORD_CALLBACK_CONTINUE = -1, ///< 继续处理下条记录
	RECORD_CALLBACK_BREAK = 1, ///< 不再处理之后记录
} record_callback_e;

/**
 * 记录排序函数
 * 如果记录a“小于”记录b, 函数返回负值；
 * 如果记录a“大于”记录b, 函数返回正值；
 * 如果两者相等，返回0.
 */
typedef int (*record_cmp_t)(const void *a, const void *b);

/**
 * 记录回调函数.
 * @param[in,out] ptr 一条记录, 可以直接修改
 * @param[in,out] args 给回调函数的参数
 * @param[in] offset 本条记录的偏移量，以记录为单位，文件头为基准
 */
typedef record_callback_e (*record_callback_t)(void *ptr, void *args, int offset);

/** 记录文件数据结构 */
typedef struct record_t {
	int fd; ///< 文件描述符
	int rlen; ///< 单条记录长度
	record_cmp_t cmp; ///< 记录排序函数
} record_t;

extern int record_open(const char *file, record_cmp_t cmp, int rlen, record_perm_e rdonly, record_t *rec);
extern int record_close(record_t *rec);
extern int record_count(record_t *rec);
extern int record_lock(record_t *rec, record_lock_e type, int offset, record_whence_e whence, int count);
extern int record_lock_all(record_t *rec, record_lock_e type);
extern int record_try_lock_all(record_t *rec, record_lock_e type);
extern int record_seek(record_t *rec, int offset, record_whence_e whence);
extern int record_read(record_t *rec, void *ptr, int count);
extern int record_read_after(record_t *rec, void *ptr, int count, int offset);
extern int record_write(record_t *rec, const void *ptr, int count, int offset);
extern int record_append(record_t *rec, const void *ptr, int count);
extern int record_append_locked(record_t *rec, const void *ptr, int count);
extern int record_apply(record_t *rec, void *ptr, int offset, record_callback_t callback, void *args, bool delete_);
/**
 * 删除记录文件中的匹配记录
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 当前记录, 可为NULL
 * @param[in] offset 当前偏移量, 以记录为单位, 文件头为基准
 * @param[in] callback 回调函数
 * @param[in] args 给回调函数的参数
 * @return 删除的记录数
 */
#define record_delete(rec, ptr, offset, callback, args)  record_apply(rec, ptr, offset, callback, args, true)
/**
 * 修改记录文件中的匹配记录
 * @param[in] rec 记录文件数据结构
 * @param[in] ptr 当前记录, 可为NULL
 * @param[in] offset 当前偏移量, 以记录为单位, 文件头为基准
 * @param[in] callback 回调函数
 * @param[in] args 给回调函数的参数
 * @return 匹配的记录数
 */
#define record_update(rec, ptr, offset, callback, args)  record_apply(rec, ptr, offset, callback, args, false)
extern int record_foreach(record_t *rec, void *ptr, int offset, record_callback_t callback, void *args);
extern int record_reverse_foreach(record_t *rec, record_callback_t callback, void *args);
extern int record_merge(record_t *rec, void *ptr, int count);
extern int record_search_copy(record_t *rec, record_callback_t filter, void *args, int offset, bool reverse, void *out);
#define record_search(r, f, a, o, e)  record_search_copy(r, f, a, o, e, NULL)
extern int record_truncate(record_t *rec, int count);

#define COMPARE_RETURN(a, b)  \
	do { \
		if (a > b) \
			return 1; \
		if (b > a) \
			return -1; \
		return 0; \
	} while (0)

#endif // FB_RECORD_H
