#ifndef _LIBBBS_H_

#define _LIBBBS_H_

//fileio.c
void file_append(const char *fpath, const char *msg);
int dashf(char *fname);
int dashd(char *fname);
int part_cp(char *src, char *dst, char *mode);
int f_cp(char *src, char *dst, int mode);
int f_ln(char *src, char *dst);
int valid_fname(char *str);
int touchfile(char *filename);
int f_rm(char *fpath);

//mmdecode.c
void _mmdecode(unsigned char *str);

//modetype.c
char *ModeType(int mode);

//string.c
char *strtolower(char *dst, const char *src);
char *strtoupper(char *dst, const char *src);
char *strcasestr_gbk(const char *haystack, const char *needle);
char *ansi_filter(char *dst, const char *src);
int getdatestring(time_t time, int mode);
int ellipsis(char *str, int len);
char *rtrim(unsigned char *str);
char *trim(unsigned char *str);
size_t strlcpy(char *dst, const char *src, size_t siz);

//boardrc.c
void brc_update(const char *userid, const char *board);
int brc_initial(const char* userid, const char *board);
void brc_addlist(const char *filename);
int brc_unread(const char *filename);
int brc_unread1(int ftime);
int brc_clear(int ent, const char *direct, int clearall);
void brc_zapbuf(int *zbuf);

#endif
