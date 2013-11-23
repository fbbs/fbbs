#include <stdarg.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "fbbs/mdbi.h"
#include "fbbs/string.h"

enum {
	MDB_RES_STATUS = REDIS_REPLY_STATUS,
	MDB_RES_ERROR = REDIS_REPLY_ERROR,
	MDB_RES_INTEGER = REDIS_REPLY_INTEGER,
	MDB_RES_NIL = REDIS_REPLY_NIL,
	MDB_RES_STRING = REDIS_REPLY_STRING,
	MDB_RES_ARRAY = REDIS_REPLY_ARRAY,
};

enum {
	MDB_CMD_BUF_LEN = 128,
};

typedef struct {
	redisContext *c;
	char buf[MDB_CMD_BUF_LEN];
} mdb_conn_t;

static mdb_conn_t _mdb;

int mdb_connect_unix(const char *path)
{
	_mdb.c = redisConnectUnix(path);
	if (!_mdb.c || _mdb.c->err)
		return -1;
	return 0;
}

void mdb_disconnect(void)
{
	redisFree(_mdb.c);
}

int mdb_fd(void)
{
	return _mdb.c ? _mdb.c->fd : -1;
}

static char *smart_vsnprintf(char *buf, size_t size,
		const char *fmt, va_list ap)
{
	va_list aq;
	va_copy(aq, ap);

	char *s = buf;
	size_t len = vsnprintf(buf, size, fmt, ap);
	if (len >= size) {
		s = malloc(len + 1);
		vsnprintf(s, len + 1, fmt, aq);
	}

	va_end(aq);
	return s;
}

void mdb_clear(mdb_res_t *res)
{
	if (res)
		freeReplyObject(res);
}

static mdb_res_t *mdb_vcmd(bool safe, const char *cmd, const char *fmt,
		va_list ap)
{
	char real_fmt[32];
	size_t bytes = snprintf(real_fmt, sizeof(real_fmt), "%s %s", cmd, fmt);
	if (bytes >= sizeof(real_fmt))
		return NULL;

	redisReply *res;
	if (safe) {
		res = redisvCommand(_mdb.c, real_fmt, ap);
	} else {
		char *buf = _mdb.buf;
		char *s = smart_vsnprintf(buf, sizeof(_mdb.buf), real_fmt, ap);
		res = redisCommand(_mdb.c, s);
		if (s != buf)
			free(s);
	}

	if (!res || res->type == MDB_RES_ERROR) {
		mdb_clear(res);
		return NULL;
	}
	return res;
}

#define MDB_CMD_HELPER(safe)  \
	va_list ap; \
	va_start(ap, fmt); \
	redisReply *res = mdb_vcmd(safe, cmd, fmt, ap); \
	va_end(ap);

bool mdb_cmd(const char *cmd, const char *fmt, ...)
{
	MDB_CMD_HELPER(false);
	mdb_clear(res);
	return res;
}

bool mdb_cmd_safe(const char *cmd, const char *fmt, ...)
{
	MDB_CMD_HELPER(true);
	mdb_clear(res);
	return res;
}

mdb_res_t *mdb_res(const char *cmd, const char *fmt, ...)
{
	MDB_CMD_HELPER(false);
	return res;
}

mdb_res_t *mdb_res_safe(const char *cmd, const char *fmt, ...)
{
	MDB_CMD_HELPER(true);
	return res;
}

mdb_res_t *mdb_res_at(const mdb_res_t *res, int index)
{
	const redisReply *r = res;
	if (r && r->type == MDB_RES_ARRAY && index >= 0 && index < r->elements) {
		return r->element[index];
	}
	return NULL;
}

mdb_int_t mdb_integer(mdb_int_t invalid, const char *cmd, const char *fmt, ...)
{
	MDB_CMD_HELPER(false);
	if (!res)
		return invalid;

	long long i;
	if (res->type == MDB_RES_INTEGER)
		i = res->integer;
	else if (res->type == MDB_RES_STRING)
		i = strtoll(res->str, NULL, 10);
	else
		i = invalid;

	mdb_clear(res);
	return i;
}

char *mdb_string_and_size(mdb_res_t *res, size_t *size)
{
	redisReply *r = res;
	if (!r || r->type != MDB_RES_STRING)
		return NULL;

	*size = r->len;
	return r->str;
}

char *mdb_string(mdb_res_t *res)
{
	size_t size;
	return mdb_string_and_size(res, &size);
}
