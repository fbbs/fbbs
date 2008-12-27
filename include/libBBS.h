#ifndef _LIBBBS_H_

#define _LIBBBS_H_

//fileio.c
void file_append(char *fpath, char *msg);
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
char *strtolower(char *dst, char *src);
char *strtoupper(char *dst, char *src);
char *strcasestr_gbk(char *haystack, char *needle);
char *ansifilter(char *dst, char *src);
int getdatestring(time_t time, int mode);

#endif
