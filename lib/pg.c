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

struct query_builder_t {
	pool_t *p;
	pstring_t *query;
	struct query_param_list_t params;
	query_param_t *tail;
	int count;
};

query_builder_t *query_builder_new(size_t size)
{
	pool_t *pool = pool_create(0);

	query_builder_t *b = pool_alloc(pool, sizeof(*b));
	b->p = pool;
	SLIST_INIT_HEAD(&b->params);
	b->tail = NULL;
	b->count = 0;

	if (!size)
		size = 511;
	b->query = pstring_sized_new(pool, size);

	return b;
}

static void query_builder_append_param(query_builder_t *b, query_param_t *p)
{
	query_param_t *n = pool_alloc(b->p, sizeof(*n));
	*n = *p;

	if (b->tail)
		SLIST_INSERT_AFTER(b->tail, n, next);
	else
		SLIST_INSERT_HEAD(&b->params, n, next);
	b->tail = n;

	pstring_append_printf(b->p, b->query, "$%d", ++b->count);
}

static void query_builder_vappend(query_builder_t *b, const char *cmd,
		va_list ap)
{
	if (!cmd)
		return;
	if (*cmd != ' ')
		pstring_append_space(b->p, b->query);

	while (*cmd != '\0') {
		if (*cmd == '%') {
			query_param_t param;
			++cmd;
			switch (*cmd) {
				case 'b':
					param.val.d = va_arg(ap, int);
					param.type = QUERY_PARAM_BOOL;
					query_builder_append_param(b, &param);
					break;
				case 'd':
					param.val.d = va_arg(ap, int);
					param.type = QUERY_PARAM_INT;
					query_builder_append_param(b, &param);
					break;
				case 'l':
					param.val.l = va_arg(ap, int64_t);
					param.type = QUERY_PARAM_BIGINT;
					query_builder_append_param(b, &param);
					break;
				case 's':
					param.val.s = va_arg(ap, const char *);
					param.type = QUERY_PARAM_STRING;
					query_builder_append_param(b, &param);
					break;
				case 't':
					param.val.t = va_arg(ap, fb_time_t);
					param.type = QUERY_PARAM_TIME;
					query_builder_append_param(b, &param);
					break;
				default:
					pstring_append_c(b->p, b->query, *cmd);
					break;
			}
		} else {
			pstring_append_c(b->p, b->query, *cmd);
		}
		++cmd;
	}
}

query_builder_t *query_builder_append(query_builder_t *b, const char *cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	query_builder_append(b, cmd, ap);
	va_end(ap);
	return b;
}

query_builder_t *query_builder_append_symbol(query_builder_t *b,
		const char *symbol, const char *cmd, ...)
{
	if (symbol && *symbol)
		pstring_append_string(b->p, b->query, symbol);

	va_list ap;
	va_start(ap, cmd);
	query_builder_append(b, cmd, ap);
	va_end(ap);
	return b;
}

static void convert_param_array(const query_builder_t *b, const char **vals,
		int *lens, int *fmts)
{
	int n = 0;
	SLIST_FOREACH(query_param_t, param, &b->params, next) {
		switch (param->type) {
			case QUERY_PARAM_BOOL: {
					char *p = pool_alloc(b->p, sizeof(*p));
					*p = param->val.d ? 1 : 0;
					vals[n] = p;
					lens[n] = 1;
					fmts[n] = 1;
				}
				break;
			case QUERY_PARAM_INT: {
					int *p = pool_alloc(b->p, sizeof(*p));
					*p = htonl(param->val.d);
					vals[n] = (const char *)p;
					lens[n] = sizeof(*p);
					fmts[n] = 1;
				}
				break;
			case QUERY_PARAM_BIGINT: {
					int64_t *p = pool_alloc(b->p, sizeof(*p));
					*p = htobe64(param->val.l);
					vals[n] = (const char *)p;
					lens[n] = sizeof(*p);
					fmts[n] = 1;
				}
				break;
			case QUERY_PARAM_TIME: {
					db_timestamp ts = time_to_ts(param->val.t);
					db_timestamp *p = pool_alloc(b->p, sizeof(*p));
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

static db_res_t *query_builder_exec(const query_builder_t *b, int expected)
{
	db_res_t *res;
	if (b->count) {
		const char **vals = pool_alloc(b->p, b->count * sizeof(*vals));
		int *lens = pool_alloc(b->p, b->count * sizeof(*lens));
		int *fmts = pool_alloc(b->p, b->count * sizeof(*fmts));

		convert_param_array(b, vals, lens, fmts);
		res = PQexecParams(global_db_conn, pstring(b->query), b->count, NULL,
				vals, lens, fmts, 1);
	} else {
		res = PQexecParams(global_db_conn, pstring(b->query), 0, NULL, NULL,
				NULL, NULL, 1);
	}

	if (db_res_status(res) != expected) {
		db_clear(res);
		return NULL;
	}
	return res;
}

db_res_t *query_builder_query(const query_builder_t *b)
{
	return query_builder_exec(b, DBRES_TUPLES_OK);
}

db_res_t *query_builder_cmd(const query_builder_t *b)
{
	return query_builder_exec(b, DBRES_COMMAND_OK);
}

void query_builder_free(query_builder_t *b)
{
	pool_destroy(b->p);
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

static db_res_t *db_exec(const char *cmd, va_list ap, int expected)
{
	db_res_t *res;
	int argc = get_num_args(cmd);
	if (argc > 0) {
		query_builder_t *builder = query_builder_new(strlen(cmd));
		query_builder_vappend(builder, cmd, ap);
		res = query_builder_exec(builder, expected);
		query_builder_free(builder);
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
