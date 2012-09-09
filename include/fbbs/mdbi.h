#ifndef FB_MDBI_H
#define FB_MDBI_H

#include <hiredis/hiredis.h>

enum {
	MDB_RES_STATUS = REDIS_REPLY_STATUS,
	MDB_RES_ERROR = REDIS_REPLY_ERROR,
	MDB_RES_INTEGER = REDIS_REPLY_INTEGER,
	MDB_RES_NIL = REDIS_REPLY_NIL,
	MDB_RES_STRING = REDIS_REPLY_STRING,
	MDB_RES_ARRAY = REDIS_REPLY_ARRAY,
};

typedef struct mdb_conn_t mdb_conn_t;

typedef redisReply mdb_res_t;

extern int mdb_connect_unix(const char *path);
extern void mdb_disconnect(void);
extern mdb_res_t *mdb_cmd(const char *cmd, const char *fmt, ...);
extern long long mdb_integer(long long invalid, const char *cmd, const char *fmt, ...);

#define mdb_clear(res)  freeReplyObject(res)
#define mdb_finish(conn)  redisFree(conn)

#endif // FB_MDBI_H
