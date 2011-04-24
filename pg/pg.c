#include <endian.h>

#ifndef be64toh
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  include <byteswap.h>
#  define be64toh(x) bswap_64(x)
# else
#  define be64toh(x) (x)
# endif
#endif

#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include "fbbs/dbi.h"
#include "fbbs/util.h"

/** Postgresql epoch (Jan 1, 2000) in unix time. */
#define POSTGRES_EPOCH_TIME INT64_C(946684800)

/**
 * Default postgresql installation used to use double to represent timestamp.
 * It was changed to 64-bit integer since 8.4 (Jul 2009).
 */
#define HAVE_INT64_TIMESTAMP

fb_time_t ts_to_time(timestamp ts)
{
	return ts / INT64_C(1000000) + POSTGRES_EPOCH_TIME;
}

timestamp time_to_ts(fb_time_t t)
{
	return (t - POSTGRES_EPOCH_TIME) * INT64_C(1000000);
}

db_conn_t *db_connect(const char *host, const char *port,
        const char *db, const char *user, const char *pwd)
{
	return PQsetdbLogin(host, port, NULL, NULL, db, user, pwd);
}

void db_finish(db_conn_t *conn)
{
	PQfinish(conn);
}

db_conn_status_t db_status(db_conn_t *conn)
{
	return PQstatus(conn);
}

const char *db_errmsg(db_conn_t *conn)
{
	return PQerrorMessage(conn);
}

db_res_t *db_exec(db_conn_t *conn, const char *cmd)
{
	return PQexec(conn, cmd);
}

db_res_t *db_exec_params(db_conn_t *conn, const char *cmd, int count,
		db_param_t *params, bool binary)
{
	if (count > 0) {
		const char *values[count];
		int lengths[count];
		int formats[count];
		
		for (int i = 0; i < count; ++i) {
			values[i] = params[i].value;
			lengths[i] = params[i].length;
			formats[i] = params[i].format;
		}
		
		return PQexecParams(conn, cmd, count, NULL, values, lengths,
			formats, binary);
	} else {
		return PQexecParams(conn, cmd, 0, NULL, NULL, NULL, NULL, binary);
	}
}

db_exec_status_t db_res_status(const db_res_t *res)
{
	return PQresultStatus(res);
}

void db_clear(db_res_t *res)
{
	PQclear(res);
}

int db_num_rows(const db_res_t *res)
{
	return PQntuples(res);
}

int db_num_fields(const db_res_t *res)
{
	return PQnfields(res);
}

bool _is_binary_field(const db_res_t *res, int col)
{
	return PQfformat(res, col);
}

bool db_get_is_null(const db_res_t *res, int row, int col)
{
	return PQgetisnull(res, row, col);
}

const char *db_get_value(const db_res_t *res, int row, int col)
{
	return PQgetvalue(res, row, col);
}

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
		timestamp ts = (int64_t)be64toh(*(uint64_t *)r);
#else
		uint64_t t = be64toh(*(uint64_t *)r);
		timestamp ts = *(double *)&t * 1000000;
#endif
		return ts_to_time(ts);
	}
	return 0;
}
