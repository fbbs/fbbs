#ifndef FB_RECORD_H
#define FB_RECORD_H

#include "fbbs/fileio.h"

typedef enum {
	RECORD_RDLCK = FILE_RDLCK,
	RECORD_WRLCK = FILE_WRLCK,
	RECORD_UNLCK = FILE_UNLCK,
} record_lock_e;

typedef enum {
	RECORD_SET = FILE_SET,
	RECORD_CUR = FILE_CUR,
	RECORD_END = FILE_END,
} record_whence_e;

typedef enum {
	RECORD_READ = 1,
	RECORD_WRITE = 0,
} record_perm_e;

typedef enum {
	RECORD_CALLBACK_MATCH = 0,
	RECORD_CALLBACK_CONTINUE = -1,
	RECORD_CALLBACK_BREAK = 1,
} record_callback_e;

typedef int (*record_cmp_t)(const void *, const void *);
typedef record_callback_e (*record_callback_t)(void *, void *, int);

typedef struct record_t {
	int fd;
	int rlen;
	record_cmp_t cmp;
} record_t;

extern int record_open(const char *file, record_cmp_t cmp, int rlen, record_perm_e rdonly, record_t *rec);
extern int record_close(record_t *rec);
extern int record_count(record_t *rec);
extern int record_lock(record_t *rec, record_lock_e type, int offset, record_whence_e whence, int count);
extern int record_lock_all(record_t *rec, record_lock_e type);
extern int record_seek(record_t *rec, int offset, record_whence_e whence);
extern int record_read(record_t *rec, void *ptr, int count);
extern int record_read_after(record_t *rec, void *ptr, int count, int offset);
extern int record_write(record_t *rec, const void *ptr, int count, int offset);
extern int record_append(record_t *rec, const void *ptr, int count);
extern int record_append_locked(record_t *rec, const void *ptr, int count);
extern int record_apply(record_t *rec, void *ptr, int offset, record_callback_t callback, void *args, bool delete_);
#define record_delete(r, p, o, c, a)  record_apply(r, p, o, c, a, true)
#define record_update(r, p, o, c, a)  record_apply(r, p, o, c, a, false)
extern int record_foreach(record_t *rec, void *ptr, int offset, record_callback_t callback, void *args);
extern int record_reverse_foreach(record_t *rec, record_callback_t callback, void *args);
extern int record_insert(record_t *rec, void *ptr, int count);
extern int record_merge(record_t *rec, void *ptr, int count);
extern int record_search_copy(record_t *rec, record_callback_t filter, void *args, int offset, bool reverse, void *out);
#define record_search(r, f, a, o, e)  record_search_copy(r, f, a, o, e, NULL)

#define COMPARE_RETURN(a, b)  \
	do { \
		if (a > b) \
			return 1; \
		if (b > a) \
			return -1; \
		return 0; \
	} while (0)

#endif // FB_RECORD_H
