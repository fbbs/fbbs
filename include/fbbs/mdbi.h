#ifndef FB_MDBI_H
#define FB_MDBI_H

#include <stdbool.h>
#include <stddef.h>

typedef long long mdb_int_t;
typedef void mdb_res_t;

extern int mdb_connect_unix(const char *path);
extern void mdb_disconnect(void);
extern int mdb_fd(void);

extern bool mdb_cmd(const char *cmd, const char *fmt, ...);
extern bool mdb_cmd_safe(const char *cmd, const char *fmt, ...);
extern mdb_res_t *mdb_res(const char *cmd, const char *fmt, ...);
extern mdb_res_t *mdb_res_safe(const char *cmd, const char *fmt, ...);
extern mdb_res_t *mdb_res_at(const mdb_res_t *res, int index);
extern void mdb_clear(mdb_res_t *res);

extern mdb_int_t mdb_integer(mdb_int_t invalid, const char *cmd, const char *fmt, ...);
extern char *mdb_string_and_size(mdb_res_t *res, size_t *size);
extern char *mdb_string(mdb_res_t *res);

#endif // FB_MDBI_H
