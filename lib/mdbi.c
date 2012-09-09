#include <stdarg.h>

#include "fbbs/fbbs.h"
#include "fbbs/mdbi.h"

enum {
	MDB_CMD_BUF_LEN = 128,
};

struct mdb_conn_t {
	redisContext *c;
	char buf[MDB_CMD_BUF_LEN];
};

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
	mdb_finish(_mdb.c);
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

static mdb_res_t *mdb_vcmd(const char *cmd, const char *fmt, va_list ap)
{
	char real_fmt[32];
	size_t bytes = snprintf(real_fmt, sizeof(real_fmt), "%s %s", cmd, fmt);
	if (bytes >= sizeof(real_fmt))
		return NULL;

	char *buf = _mdb.buf;
	char *s = smart_vsnprintf(buf, sizeof(_mdb.buf), real_fmt, ap);

	mdb_res_t *res = redisCommand(_mdb.c, s);

	if (s != buf)
		free(s);

	if (!res || res->type == MDB_RES_ERROR) {
		mdb_clear(res);
		return NULL;
	}
	return res;
}

mdb_res_t *mdb_cmd(const char *cmd, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	mdb_res_t *res = mdb_vcmd(cmd, fmt, ap);
	va_end(ap);
	return res;
}

long long mdb_integer(long long invalid, const char *cmd, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	mdb_res_t *res = mdb_vcmd(cmd, fmt, ap);
	va_end(ap);

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
