#ifndef FB_MDBI_H
#define FB_MDBI_H

#include <hiredis/hiredis.h>

typedef redisContext mdb_conn_t;
typedef redisReply mdb_res_t;

#define mdb_connect_unix(path)  redisConnectUnix(path)
#define mdb_cmd(cmd, ...)  redisCommand(env.m, cmd, __VA_ARGS__)
#define mdb_clear(res)  freeReplyObject(res)
#define mdb_finish(conn)  redisFree(conn)

#endif // FB_MDBI_H
