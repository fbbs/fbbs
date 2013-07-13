#ifndef FB_MDBI_H
#define FB_MDBI_H

#include <stdbool.h>

typedef struct mdb_conn_t mdb_conn_t;
typedef long long mdb_int_t;

extern int mdb_connect_unix(const char *path);
extern void mdb_disconnect(void);
extern bool mdb_cmd(const char *cmd, const char *fmt, ...);
extern mdb_int_t mdb_integer(mdb_int_t invalid, const char *cmd, const char *fmt, ...);

#endif // FB_MDBI_H
