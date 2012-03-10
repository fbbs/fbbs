#ifndef FB_MDBI_H
#define FB_MDBI_H

#include <hiredis/hiredis.h>

enum {
	MDB_CMD_BUF_LEN = 128,
};

typedef struct mdb_conn_t {
	redisContext *c;
	char buf[MDB_CMD_BUF_LEN];
} mdb_conn_t;

typedef redisReply mdb_res_t;

extern int mdb_connect_unix(const char *path);
extern mdb_res_t *mdb_cmd(const char *cmd, ...);
#define mdb_clear(res)  freeReplyObject(res)
#define mdb_finish(conn)  redisFree(conn)

#endif // FB_MDBI_H
