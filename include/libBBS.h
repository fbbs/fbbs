#ifndef LIBBBS_H
#define LIBBBS_H

//mmap.c
enum {
	MMAP_RDONLY = 0,
	MMAP_WRONLY = 1,
	MMAP_RDWR = 2,
	MMAP_NOLOCK = 3
};

//record.c
typedef int (*RECORD_FUNC_ARG)(void *, void *);
typedef int (*APPLY_FUNC_ARG)(void *, int, void *);

//fileio.c
int file_append(const char *fpath, const char *msg);
int dashf(const char *fname);
int dashd(const char *fname);
int part_cp(char *src, char *dst, char *mode);
int f_cp(const char *src, const char *dst, int mode);
int f_ln(const char *src, const char *dst);
int valid_fname(char *str);
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
int ellipsis(char *str, int len);
char *rtrim(char *str);
char *trim(char *str);
size_t strlcpy(char *dst, const char *src, size_t siz);
void strtourl(char *url, const char *str);

//boardrc.c
void brc_update(const char *userid, const char *board);
int brc_initial(const char* userid, const char *board);
void brc_addlist(const char *filename);
int brc_unread(const char *filename);
int brc_unread1(int ftime);
int brc_clear(int ent, const char *direct, int clearall);
void brc_zapbuf(int *zbuf);

//record.c
long get_num_records(const char *filename, const int size);
int append_record(const char *filename, const void *record, int size);
int get_record(char *filename, void *rptr, int size, int id);
int get_records(const char *filename, void *rptr, int size, int id,
		int number);
int apply_record(char *filename, APPLY_FUNC_ARG fptr, int size, void *arg,
		int applycopy, int reverse);
int search_record(char *filename, void *rptr, int size,
		RECORD_FUNC_ARG fptr, void *farg);
int substitute_record(char *filename, void *rptr, int size, int id);
int delete_record(char *filename, int size, int id,
		RECORD_FUNC_ARG filecheck, void *arg);
int delete_range(char *filename, int id1, int id2);
int insert_record(char *filename, int size, RECORD_FUNC_ARG filecheck,
		void *arg);

//pass.c
char *genpasswd(const char *pw);
int checkpasswd(char *pw_crypted, char *pw_try);

//shm.c
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);

//mmap.c
int mmap_open(const char *file, int flags, void **ptr, size_t *size);
void mmap_close(void *ptr, size_t size, int fd);
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size);

#endif // LIBBBS_H
