#ifndef FB_MDBI_H
#define FB_MDBI_H

#include <hiredis/hiredis.h>

enum {
	MDB_CMD_BUF_LEN = 128,

	MDB_RES_STATUS = REDIS_REPLY_STATUS,
	MDB_RES_ERROR = REDIS_REPLY_ERROR,
	MDB_RES_INTEGER = REDIS_REPLY_INTEGER,
	MDB_RES_NIL = REDIS_REPLY_NIL,
	MDB_RES_STRING = REDIS_REPLY_STRING,
	MDB_RES_ARRAY = REDIS_REPLY_ARRAY,
};

typedef struct mdb_conn_t {
	redisContext *c;
	char buf[MDB_CMD_BUF_LEN];
} mdb_conn_t;

typedef redisReply mdb_res_t;

extern int mdb_connect_unix(const char *path);
extern mdb_res_t *mdb_cmd(const char *cmd, ...);
extern long long mdb_get_integer(long long invalid, const char *cmd, ...);

#define mdb_clear(res)  freeReplyObject(res)
#define mdb_finish(conn)  redisFree(conn)

#endif // FB_MDBI_H
