#ifndef FB_LIBBBS_H
#define FB_LIBBBS_H

#include <string.h>
#include <stdbool.h>

//mmap.c
/** Memory mapped file information. */
typedef struct {
	int fd;       ///< file descriptor.
	int oflag;    ///< open flags.
	int lock;     ///< lock status.
	int prot;     ///< memory protection of the mapping.
	int mflag;    ///< MAP_SHARED or MAP_PRIVATE.
	void *ptr;    ///< starting address of the mapping.
	size_t msize; ///< mmaped size.
	size_t size;  ///< real file size.
} mmap_t;

//record.c
/** Record stream information. */
typedef mmap_t record_t;
typedef int (*record_func_t)(void *, void *);
typedef int (*apply_func_t)(void *, int, void *);
/** Comparator function pointer.
 * The comparator routine is expected to return an integer less than, equal
 * to, or greater than zero if the first object is less than, equal to, or 
 * greater than the second object.
 */
typedef int (*comparator_t)(const void *, const void *);
/** Pointer to functions performing search algorithms. */
typedef void *(*search_method_t)(const void *, const void *, size_t, size_t,
		comparator_t);

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
char *mask_host(const char *host);

//boardrc.c
void brc_update(const char *userid, const char *board);
int brc_initial(const char* userid, const char *board);
void brc_addlist(const char *filename);
int brc_unread(const char *filename);
int brc_unread1(int ftime);
int brc_clear(int ent, const char *direct, int clearall);
void brc_zapbuf(int *zbuf);

//record.c
long get_num_records(const char *file, int size);
int append_record(const char *file, const void *record, int size);
int get_record(char *filename, void *rptr, int size, int id);
int get_records(const char *filename, void *rptr, int size, int id,
		int number);
int apply_record(const char *file, apply_func_t func, int size,
			void *arg, bool copy, bool reverse, bool lock);
int search_record(const char *file, void *rptr, int size, record_func_t func,
		void *arg);
int substitute_record(char *filename, void *rptr, int size, int id);
int delete_record(const char *file, int size, int id,
		record_func_t check, void *arg);
int delete_range(char *filename, int id1, int id2);
int insert_record(const char *file, int size, record_func_t check, void *arg);
void *lsearch(const void *key, const void *base, size_t nmemb, size_t size,
		comparator_t compar);
int record_open(const char *file, int mode, record_t *r);
int record_close(record_t *r);
void *record_search(record_t *r, const void *key, size_t size,
		search_method_t method, comparator_t compar);
int record_delete(record_t *r, void *ptr, int size);

//pass.c
char *genpasswd(const char *pw);
int checkpasswd(char *pw_crypted, char *pw_try);

//shm.c
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);

//mmap.c
int mmap_open_fd(mmap_t *m);
int mmap_open(const char *file, mmap_t *m);
int mmap_close(mmap_t *m);
int mmap_truncate(mmap_t *m, size_t size);
int mmap_shrink(mmap_t *m, size_t size);
int mmap_lock(mmap_t *m, int lock);

//mail.c
int getmailboxsize(unsigned int userlevel);
int getmailboxhold(unsigned int userlevel);
int getmailsize(const char *userid);
int getmailnum(const char *userid);
int do_mail_file(const char *recv, const char *title, const char *header,
		const char *text, int len, const char *source);
int mail_file(const char *file, const char *recv, const char *title);

#endif // FB_LIBBBS_H
