#include <arpa/inet.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "fbbs/dbi.h"
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/util.h"

/** Postgresql epoch (Jan 1, 2000) in unix time. */
#define POSTGRES_EPOCH_TIME INT64_C(946684800)

/**
 * Default postgresql installation used to use double to represent timestamp.
 * It was changed to 64-bit integer since 8.4 (Jul 2009).
 */
#define HAVE_INT64_TIMESTAMP

typedef PGconn db_conn_t;

static db_conn_t *global_db_conn;

bool db_connect(const char *host, const char *port, const char *db,
		const char *user, const char *pwd)
{
	global_db_conn = PQsetdbLogin(host, port, NULL, NULL, db, user, pwd);

	if (PQstatus(global_db_conn) != CONNECTION_OK)
		global_db_conn = NULL;

	return global_db_conn;
}

void db_finish(void)
{
	PQfinish(global_db_conn);
}

const char *db_errmsg(void)
{
	return PQerrorMessage(global_db_conn);
}

fb_time_t ts_to_time(db_timestamp ts)
{
	return ts / INT64_C(1000000) + POSTGRES_EPOCH_TIME;
}

db_timestamp time_to_ts(fb_time_t t)
{
	return (t - POSTGRES_EPOCH_TIME) * INT64_C(1000000);
}

void db_clear(db_res_t *res)
{
	if (res)
		PQclear(res);
}

#define _is_binary_field(res, col)  PQfformat(res, col)

int16_t db_get_smallint(const db_res_t *res, int row, int col)
{
	const char *r = PQgetvalue(res, row, col);
	if (_is_binary_field(res, col)) {
		return (int16_t)ntohs(*((uint16_t *)r));
	} else {
		return (int16_t)strtol(r, NULL, 10);
	}
}

int32_t db_get_integer(const db_res_t *res, int row, int col)
{
	const char *r = PQgetvalue(res, row, col);
	if (_is_binary_field(res, col)) {
		return (int32_t)ntohl(*((uint32_t *)r));
	} else {
		return (int32_t)strtol(r, NULL, 10);
	}
}

int64_t db_get_bigint(const db_res_t *res, int row, int col)
{
	const char *r = PQgetvalue(res, row, col);
	if (_is_binary_field(res, col)) {
		return (int64_t)be64toh(*((uint64_t *)r));
	} else {
		return (int64_t)strtoll(r, NULL, 10);
	}
}

bool db_get_bool(const db_res_t *res, int row, int col)
{
	const char *r = PQgetvalue(res, row, col);
	if (_is_binary_field(res, col)) {
		return *r;
	} else {
		switch (*r) {
			case 'T':
			case 't':
			case 'Y':
			case 'y':
			case '1':
				return true;
			default:
				return false;
		}
	}
}

fb_time_t db_get_time(const db_res_t *res, int row, int col)
{
	const char *r = PQgetvalue(res, row, col);
	if (_is_binary_field(res, col)) {
#ifdef HAVE_INT64_TIMESTAMP
		db_timestamp ts = (int64_t)be64toh(*(uint64_t *)r);
#else
		uint64_t t = be64toh(*(uint64_t *)r);
		db_timestamp ts = *(double *)&t * 1000000;
#endif
		return ts_to_time(ts);
	}
	return 0;
}

float db_get_float(const db_res_t *res, int row, int col)
{
	if (_is_binary_field(res, col)) {
		union {
			int32_t integer;
			float my_float;
		} i;
		i.integer = db_get_integer(res, row, col);
		return i.my_float;
	} else {
		return strtof(PQgetvalue(res, row, col), NULL);
	}
}

#define is_supported_format(c) \
	(c == 'd' || c == 'l' || c == 's' || c == 't' || c == 'b')

static int get_num_args(const char *cmd)
{
	int argc = 0;
	while (*cmd != '\0') {
		if (*cmd == '%') {
			++cmd;
			if (is_supported_format(*cmd)) {
				++argc;
			}
		}
		++cmd;
	}
	return argc;
}

typedef struct query_t {
	pstring_t *s;
	pool_t *p;
	const char **vals;
	int *lens;
	int *fmts;
} query_t;

static query_t *query_new(const char *cmd, int argc, va_list ap)
{
	pool_t *p = pool_create(0);
	query_t *q = pool_alloc(p, sizeof(*q));
	q->p = p;
	q->vals = pool_alloc(p, sizeof(*q->vals) * argc);
	q->lens = pool_alloc(p, sizeof(*q->lens) * argc);
	q->fmts = pool_alloc(p, sizeof(*q->fmts) * argc);
	q->s = pstring_sized_new(p, strlen(cmd));

	int n = 0;
	while (*cmd != '\0') {
		if (*cmd == '%') {
			int d;
			int64_t l;
			const char *s;
			fb_time_t t;

			++cmd;
			switch (*cmd) {
				case 'b':
					d = va_arg(ap, int);
					char *bp = pool_alloc(p, sizeof(*bp));
					*bp = d ? 1 : 0;
					q->vals[n] = (const char *) bp;
					q->lens[n] = sizeof(*bp);
					q->fmts[n] = 1;
					break;
				case 'd':
					d = va_arg(ap, int);
					int *dp = pool_alloc(p, sizeof(*dp));
					*dp = htonl(d);
					q->vals[n] = (const char *) dp;
					q->lens[n] = sizeof(int);
					q->fmts[n] = 1;
					break;
				case 'l':
					l = va_arg(ap, int64_t);
					int64_t *lp = pool_alloc(p, sizeof(*lp));
					*lp = htobe64(l);
					q->vals[n] = (const char *) lp;
					q->lens[n] = sizeof(int64_t);
					q->fmts[n] = 1;
					break;
				case 's':
					s = va_arg(ap, const char *);
					q->vals[n] = s;
					q->lens[n] = 0;
					q->fmts[n] = 0;
					break;
				case 't':
					t = va_arg(ap, fb_time_t);
					db_timestamp ts = time_to_ts(t);
					db_timestamp *tsp = pool_alloc(p, sizeof(*tsp));
					*tsp = htobe64(ts);
					q->vals[n] = (const char *) tsp;
					q->lens[n] = sizeof(int64_t);
					q->fmts[n] = 1;
					break;
				default:
					pstring_append_c(p, q->s, *cmd);
					break;
			}
			if (is_supported_format(*cmd)) {
				pstring_append_printf(p, q->s, "$%d", ++n);
			}
		} else {
			pstring_append_c(p, q->s, *cmd);
		}
		++cmd;
	}
	return q;
}

static void query_free(query_t *q)
{
	pool_destroy(q->p);
}

static db_res_t *_db_exec_cmd(db_conn_t *conn, const char *cmd, bool binary,
		int expected, va_list ap)
{
	db_res_t *res;
	int argc = get_num_args(cmd);
	if (argc > 0) {
		query_t *q = query_new(cmd, argc, ap);
		res = PQexecParams(conn, pstring(q->s), argc, NULL, q->vals, q->lens,
				q->fmts, binary);
		query_free(q);
	} else {
		res = PQexecParams(conn, cmd, 0, NULL, NULL, NULL, NULL, binary);
	}

	if (db_res_status(res) != expected) {
		db_clear(res);
		return NULL;
	}

	return res;
}

db_res_t *db_cmd(const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	db_res_t *res = _db_exec_cmd(global_db_conn, cmd, true, DBRES_COMMAND_OK, ap);
	va_end(ap);
	return res;
}

db_res_t *db_query(const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	db_res_t *res = _db_exec_cmd(global_db_conn, cmd, true, DBRES_TUPLES_OK, ap);
	va_end(ap);
	return res;
}

int db_begin_trans(void)
{
	db_res_t *res = PQexec(global_db_conn, "BEGIN");
	int r = (PQresultStatus(res) == PGRES_COMMAND_OK ? 0 : -1);
	PQclear(res);
	return r;
}

int db_end_trans(void)
{
	db_res_t *res = PQexec(global_db_conn, "END");
	int r = (PQresultStatus(res) == PGRES_COMMAND_OK ? 0 : -1);
	PQclear(res);
	return r;
}
