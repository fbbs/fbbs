#ifndef FB_FILEIO_H
#define FB_FILEIO_H

typedef enum {
	FILE_RDLCK = 0, // F_RDLCK
	FILE_WRLCK = 1, // F_WRLCK
	FILE_UNLCK = 2, // F_UNLCK
} file_lock_e;

typedef enum {
	FILE_SET = 0, // SEEK_SET
	FILE_CUR = 1, // SEEK_CUR
	FILE_END = 2, // SEEK_END
} file_whence_e;

extern int file_append(const char *file, const char *msg);
extern int file_read(int fd, void *buf, size_t size);
extern int file_write(int fd, const void *buf, size_t size);
extern int file_close(int fd);
extern int file_truncate(int fd, off_t size);
extern int dashf(const char *file);
extern int dashd(const char *file);
extern int part_cp(char *src, char *dst, char *mode);
extern int f_cp(const char *src, const char *dst, int mode);
extern int f_ln(const char *src, const char *dst);
extern int valid_fname(char *str);
extern int f_rm(char *fpath);
extern int file_lock(int fd, file_lock_e type, off_t offset, file_whence_e whence, off_t len);
extern int file_lock_all(int fd, file_lock_e type);

#endif // FB_FILEIO_H
