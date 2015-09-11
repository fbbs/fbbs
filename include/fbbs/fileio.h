#ifndef FB_FILEIO_H
#define FB_FILEIO_H

#include <stdbool.h>
#include <unistd.h>

typedef enum {
	FILE_RDLCK = 0, ///< 读锁(F_RDLCK)
	FILE_WRLCK = 1, ///< 写锁(F_WRLCK)
	FILE_UNLCK = 2, ///< 解锁(F_UNLCK)
} file_lock_e;

typedef enum {
	FILE_SET = 0, ///< 文件头(SEEK_SET)
	FILE_CUR = 1, ///< 当前位置(SEEK_CUR)
	FILE_END = 2, ///< 文件尾(SEEK_END)
} file_whence_e;

extern int file_append(const char *file, const char *msg);
extern int file_read(int fd, void *buf, size_t size);
extern char *file_read_all(const char *file);
extern int file_write(int fd, const void *buf, size_t size);
extern int file_close(int fd);
extern void file_close_all(void);
extern int file_truncate(int fd, off_t size);
extern bool dashf(const char *file);
extern bool dashd(const char *file);
extern int part_cp(char *src, char *dst, char *mode);
extern int f_cp(const char *src, const char *dst, int mode);
extern int f_ln(const char *src, const char *dst);
extern int valid_fname(char *str);
extern int file_rm(const char *fpath);
extern int file_lock(int fd, file_lock_e type, off_t offset, file_whence_e whence, off_t len);
extern int file_lock_all(int fd, file_lock_e type);
extern int file_try_lock_all(int fd, file_lock_e type);
extern int file_temporary_name(char *file, size_t size, const char *prefix, int num);

/**
 * 生成一个临时文件名
 * @param[out] file 文件名
 * @param[in] size 文件名的最大长度
 * @return 生成文件名的长度
 */
#define file_temp_name(file, size)  file_temporary_name(file, size, __func__, __LINE__)

#endif // FB_FILEIO_H
