#ifndef FB_LIBBBS_H
#define FB_LIBBBS_H

#include <string.h>
#include <stdbool.h>

#include "hash.h"

//fileio.c
int file_append(const char *file, const char *msg);
int safer_write(int fd, const void *buf, int size);
int restart_close(int fd);
int restart_ftruncate(int fd, off_t size);
int dashf(const char *file);
int dashd(const char *file);
int part_cp(char *src, char *dst, char *mode);
int f_cp(const char *src, const char *dst, int mode);
int f_ln(const char *src, const char *dst);
int valid_fname(char *str);
int f_rm(char *fpath);

//mmdecode.c
void _mmdecode(unsigned char *str);

//modetype.c
int get_raw_mode(int mode);
const char *mode_type(int mode);
bool is_web_user(int mode);
int get_web_mode(int mode);

//string.c
char *strtolower(char *dst, const char *src);
char *strtoupper(char *dst, const char *src);
char *strcasestr_gbk(const char *haystack, const char *needle);
char *ansi_filter(char *dst, const char *src);
int ellipsis(char *str, int len);
char *rtrim(char *str);
char *trim(char *str);
size_t strlcpy(char *dst, const char *src, size_t siz);
void strtourl(char *url, const char *str);
void strappend(char **dst, size_t *size, const char *src);

//boardrc.c
void brc_update(const char *userid, const char *board);
int brc_initial(const char* userid, const char *board);
void brc_addlist(const char *filename);
int brc_unread(const char *filename);
int brc_unread1(int ftime);
int brc_clear(int ent, const char *direct, int clearall);
void brc_zapbuf(int *zbuf);
int brc_fcgi_init(const char *user, const char *board);

//pass.c
char *genpasswd(const char *pw);
int checkpasswd(const char *pw_crypted, const char *pw_try);

//shm.c
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);

//mail.c
int getmailboxsize(unsigned int userlevel);
int getmailboxhold(unsigned int userlevel);
int getmailsize(const char *userid);
int getmailnum(const char *userid);
int do_mail_file(const char *recv, const char *title, const char *header,
		const char *text, int len, const char *source);
int mail_file(const char *file, const char *recv, const char *title);

#endif // FB_LIBBBS_H
