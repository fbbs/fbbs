#ifndef FB_DBI_H
#define FB_DBI_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "fbbs/time.h"
#include "fbbs/util.h"

typedef PGresult db_res_t;

typedef int64_t db_timestamp;

typedef enum db_exec_status_t {
	DBRES_COMMAND_OK = PGRES_COMMAND_OK,
	DBRES_TUPLES_OK = PGRES_TUPLES_OK,
} db_exec_status_t;

extern db_timestamp time_to_ts(fb_time_t t);

extern bool db_connect(const char *host, const char *port, const char *db, const char *user, const char *pwd);
extern void db_finish(void);
extern const char *db_errmsg(void);

#define db_res_status(res)  ((db_exec_status_t)PQresultStatus(res))
extern void db_clear(db_res_t *res);

#define db_res_rows(res)  (PQntuples(res))
#define db_res_cols(res)  (PQnfields(res))
#define db_cmd_rows(res)  (strtol(PQcmdTuples(res), NULL, 10))

#define db_get_is_null(res, row, col)  ((bool)PQgetisnull(res, row, col))
#define db_get_value(res, row, col) ((const char *)PQgetvalue(res, row, col))
extern int16_t db_get_smallint(const db_res_t *res, int row, int col);
extern int32_t db_get_integer(const db_res_t *res, int row, int col);
extern int64_t db_get_bigint(const db_res_t *res, int row, int col);
extern bool db_get_bool(const db_res_t *res, int row, int col);
extern fb_time_t db_get_time(const db_res_t *res, int row, int col);
extern float db_get_float(const db_res_t *res, int row, int col);
#define db_get_length(res, row, col)  PQgetlength(res, row, col)
#define db_field_name(res, col)  PQfname(res, col)

extern db_res_t *db_cmd(const char *cmd, ...);
extern db_res_t *db_query(const char *cmd, ...);

extern int db_begin_trans(void);
extern int db_end_trans(void);

typedef struct query_t query_t;

extern query_t *query_new(size_t size);
extern void query_append(query_t *q, const char *cmd, ...);
extern void query_sappend(query_t *q, const char *symbol, const char *cmd, ...);
/** @ingroup query_builder */
/** @{ */
#define query_select(q, fields)  query_sappend(q, "SELECT", fields)
#define query_insert(q, table, fields)  query_sappend(q, "INSERT INTO", table " " fields)
#define query_returning(q, fields)  query_sappend(q, "RETURNING", fields)
#define query_from(q, table)  query_sappend(q, "FROM", table)
#define query_update(q, table)  query_sappend(q, "UPDATE", table)
#define query_groupby(q, field)  query_sappend(q, "GROUP BY", field)
#define query_set(q, ...)  query_sappend(q, "SET", __VA_ARGS__)
#define query_where(q, ...)  query_sappend(q, "WHERE", __VA_ARGS__)
#define query_and(q, ...)  query_sappend(q, "AND", __VA_ARGS__)
/** @} */
extern void query_orderby(query_t *q, const char *field, bool asc);
extern void query_limit(query_t *q, int limit);
extern db_res_t *query_exec(query_t *q);
extern db_res_t *query_cmd(query_t *q);
#endif // FB_DBI_H
