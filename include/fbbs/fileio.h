#ifndef FB_FILEIO_H
#define FB_FILEIO_H

extern int file_append(const char *file, const char *msg);
extern int safer_write(int fd, const void *buf, int size);
extern int restart_close(int fd);
extern int restart_ftruncate(int fd, off_t size);
extern int dashf(const char *file);
extern int dashd(const char *file);
extern int part_cp(char *src, char *dst, char *mode);
extern int f_cp(const char *src, const char *dst, int mode);
extern int f_ln(const char *src, const char *dst);
extern int valid_fname(char *str);
extern int f_rm(char *fpath);
extern int fb_flock(int fd, int operation);

#endif // FB_FILEIO_H
