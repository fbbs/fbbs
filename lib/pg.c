#include <arpa/inet.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "fbbs/dbi.h"
#include "fbbs/list.h"
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/util.h"

/** Postgresql epoch (Jan 1, 2000) in unix time. */
#define POSTGRES_EPOCH_TIME UINT32_C(946684800)

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
	return ts / UINT32_C(1000000) + POSTGRES_EPOCH_TIME;
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

/** @defgroup query_builder Database Query Builder */
/** @{ */

typedef enum {
	QUERY_PARAM_BOOL,
	QUERY_PARAM_INT,
	QUERY_PARAM_BIGINT,
	QUERY_PARAM_STRING,
	QUERY_PARAM_TIME,
} query_param_e;

typedef struct query_param_t {
	query_param_e type;
	union {
		int d;
		int64_t l;
		const char *s;
		fb_time_t t;
	} val;
	SLIST_FIELD(query_param_t) next;
} query_param_t;

SLIST_HEAD(query_param_list_t, query_param_t);

struct query_t {
	pool_t *p;
	pstring_t *query;
	struct query_param_list_t params;
	query_param_t *tail;
	int count;
};

static void query_append_param(query_t *q, query_param_t *p)
{
	query_param_t *n = pool_alloc(q->p, sizeof(*n));
	*n = *p;

	if (q->tail)
		SLIST_INSERT_AFTER(q->tail, n, next);
	else
		SLIST_INSERT_HEAD(&q->params, n, next);
	q->tail = n;

	pstring_append_printf(q->p, q->query, "$%d", ++q->count);
}

static void query_vappend(query_t *q, const char *cmd, va_list ap)
{
	if (!cmd)
		return;
	if (*cmd != ' ')
		pstring_append_space(q->p, q->query);

	while (*cmd != '\0') {
		if (*cmd == '%') {
			query_param_t param;
			++cmd;
			switch (*cmd) {
				case 'b':
					param.val.d = va_arg(ap, int);
					param.type = QUERY_PARAM_BOOL;
					query_append_param(q, &param);
					break;
				case 'd':
					param.val.d = va_arg(ap, int);
					param.type = QUERY_PARAM_INT;
					query_append_param(q, &param);
					break;
				case 'l':
					param.val.l = va_arg(ap, int64_t);
					param.type = QUERY_PARAM_BIGINT;
					query_append_param(q, &param);
					break;
				case 's':
					param.val.s = va_arg(ap, const char *);
					param.type = QUERY_PARAM_STRING;
					query_append_param(q, &param);
					break;
				case 't':
					param.val.t = va_arg(ap, fb_time_t);
					param.type = QUERY_PARAM_TIME;
					query_append_param(q, &param);
					break;
				default:
					pstring_append_c(q->p, q->query, *cmd);
					break;
			}
		} else {
			pstring_append_c(q->p, q->query, *cmd);
		}
		++cmd;
	}
}

static void query_vappend_symbol(query_t *q, const char *symbol,
		const char *cmd, va_list ap)
{
	pstring_append_space(q->p, q->query);

	if (symbol && *symbol)
		pstring_append_string(q->p, q->query, symbol);

	query_vappend(q, cmd, ap);
}

void query_append(query_t *q, const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	query_vappend_symbol(q, NULL, cmd, ap);
	va_end(ap);
}

void query_sappend(query_t *q, const char *symbol, const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	query_vappend_symbol(q, symbol, cmd, ap);
	va_end(ap);
}

void query_orderby(query_t *q, const char *field, bool asc)
{
	query_sappend(q, "ORDER BY", field);
	if (!asc)
		query_append(q, "DESC");
}

void query_limit(query_t *q, int limit)
{
	int64_t l = limit;
	query_sappend(q, "LIMIT", "%l", l);
}

static void convert_param_array(const query_t *q,
		const char **vals, int *lens, int *fmts)
{
	int n = 0;
	SLIST_FOREACH(query_param_t, param, &q->params, next) {
		switch (param->type) {
			case QUERY_PARAM_BOOL: {
					char *p = pool_alloc(q->p, sizeof(*p));
					*p = param->val.d ? 1 : 0;
					vals[n] = p;
					lens[n] = 1;
					fmts[n] = 1;
				}
				break;
			case QUERY_PARAM_INT: {
					int *p = pool_alloc(q->p, sizeof(*p));
					*p = htonl(param->val.d);
					vals[n] = (const char *)p;
					lens[n] = sizeof(*p);
					fmts[n] = 1;
				}
				break;
			case QUERY_PARAM_BIGINT: {
					int64_t *p = pool_alloc(q->p, sizeof(*p));
					*p = htobe64(param->val.l);
					vals[n] = (const char *)p;
					lens[n] = sizeof(*p);
					fmts[n] = 1;
				}
				break;
			case QUERY_PARAM_TIME: {
					db_timestamp ts = time_to_ts(param->val.t);
					db_timestamp *p = pool_alloc(q->p, sizeof(*p));
					*p = htobe64(ts);
					vals[n] = (const char *)p;
					lens[n] = sizeof(*p);
					fmts[n] = 1;
				}
				break;
			default: { // QUERY_PARAM_STRING{
					vals[n] = param->val.s;
					lens[n] = 0;
					fmts[n] = 0;
				}
				break;
		}
		++n;
	}
}

static void query_free(query_t *q)
{
	pool_destroy(q->p);
}

static db_res_t *_query_exec(query_t *q, int expected)
{
	db_res_t *res;
	if (q->count) {
		const char **vals = pool_alloc(q->p, q->count * sizeof(*vals));
		int *lens = pool_alloc(q->p, q->count * sizeof(*lens));
		int *fmts = pool_alloc(q->p, q->count * sizeof(*fmts));

		convert_param_array(q, vals, lens, fmts);
		res = PQexecParams(global_db_conn, pstring(q->query), q->count, NULL,
				vals, lens, fmts, 1);
	} else {
		res = PQexecParams(global_db_conn, pstring(q->query), 0, NULL, NULL,
				NULL, NULL, 1);
	}
	query_free(q);

	if (db_res_status(res) != expected) {
		db_clear(res);
		return NULL;
	}
	return res;
}

db_res_t *query_exec(query_t *q)
{
	return _query_exec(q, DBRES_TUPLES_OK);
}

db_res_t *query_cmd(query_t *q)
{
	return _query_exec(q, DBRES_COMMAND_OK);
}

query_t *query_new(size_t size)
{
	pool_t *pool = pool_create(0);

	query_t *q = pool_alloc(pool, sizeof(*q));
	q->p = pool;

	SLIST_INIT_HEAD(&q->params);
	q->tail = NULL;
	q->count = 0;

	if (!size)
		size = 511;
	q->query = pstring_sized_new(pool, size);

	return q;
}

/** @} */

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

static db_res_t *db_exec(const char *cmd, va_list ap, int expected)
{
	db_res_t *res;
	int argc = get_num_args(cmd);
	if (argc > 0) {
		query_t *q = query_new(strlen(cmd));
		query_vappend(q, cmd, ap);
		res = _query_exec(q, expected);
	} else {
		res = PQexecParams(global_db_conn, cmd, 0, NULL, NULL, NULL, NULL, 1);
		if (db_res_status(res) != expected) {
			db_clear(res);
			return NULL;
		}
	}
	return res;
}

db_res_t *db_cmd(const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	db_res_t *res = db_exec(cmd, ap, DBRES_COMMAND_OK);
	va_end(ap);
	return res;
}

db_res_t *db_query(const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	db_res_t *res = db_exec(cmd, ap, DBRES_TUPLES_OK);
	va_end(ap);
	return res;
}
