#ifndef FB_DBI_H
#define FB_DBI_H

#include <inttypes.h>
#include <stdbool.h>
#include <libpq-fe.h>

typedef PGconn db_conn_t;
typedef PGresult db_res_t;

typedef enum db_conn_status_t {
	DB_CONNECTION_OK = CONNECTION_OK,
} db_conn_status_t;

typedef enum db_exec_status_t {
	DBRES_COMMAND_OK = PGRES_COMMAND_OK,
	DBRES_TUPLES_OK = PGRES_TUPLES_OK,
} db_exec_status_t;

typedef struct db_param_t {
	const char *value;
	int length;
	int format;
} db_param_t;

extern db_conn_t *db_connect(const char *host, const char *port,
		const char *db, const char *user, const char *pwd);
extern void db_finish(db_conn_t *conn);
extern db_conn_status_t db_status(db_conn_t *conn);
extern const char *db_errmsg(db_conn_t *conn);

extern db_res_t *db_exec_params(db_conn_t *conn, const char *cmd, int count,
		db_param_t *params, bool binary);
extern db_exec_status_t db_res_status(const db_res_t *res);
extern void db_clear(db_res_t *res);

extern int db_num_rows(const db_res_t *res);
extern int db_num_fields(const db_res_t *res);

extern bool db_get_is_null(const db_res_t *res, int row, int col);
extern const char *db_get_value(const db_res_t *res, int row, int col);
extern int16_t db_get_smallint(const db_res_t *res, int row, int col);
extern int32_t db_get_integer(const db_res_t *res, int row, int col);
extern int64_t db_get_bigint(const db_res_t *res, int row, int col);
extern bool db_get_bool(const db_res_t *res, int row, int col);

#endif // FB_DBI_H
