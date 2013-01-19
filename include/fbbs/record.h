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

typedef int (*record_cmp_t)(const void *, const void *);
typedef int (*record_filter_t)(const void *, void *);
typedef int (*record_callback_t)(const void *, void *);

typedef struct record_t {
	int fd;
	int rlen;
	record_cmp_t cmp;
} record_t;

extern int record_open(const char *file, record_cmp_t cmp, int rlen, record_t *rec);
extern int record_close(record_t *rec);
extern int record_count(record_t *rec);
extern int record_lock(record_t *rec, record_lock_e type, int offset, record_whence_e whence, int count);
extern int record_lock_all(record_t *rec, record_lock_e type);
extern int record_seek(record_t *rec, int offset, record_whence_e whence);
extern int record_read(record_t *rec, void *ptr, int count);
extern int record_read_after(record_t *rec, void *ptr, int count, int offset);
extern int record_append(record_t *rec, const void *ptr, int count);
extern int record_delete(record_t *rec, void *ptr, int offset, record_filter_t filter, void *fargs, record_callback_t callback, void *cargs);
extern int record_insert(record_t *rec, void *ptr, int count);

#endif // FB_RECORD_H
