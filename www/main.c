#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

extern int bbs_board(web_ctx_t *ctx);
extern int bbs_post(web_ctx_t *ctx);

typedef struct web_handler_t {
	const char *name;          ///< name of the handler.
	int (*func)(web_ctx_t *);  ///< handler function.
} web_handler_t;

int fcgi_foo(web_ctx_t *ctx)
{
	html_header();
	printf("Hello, world!\n</head></html>");
	return 0;
}

static const web_handler_t _handlers[] = {
	{ "board", bbs_board },
	{ "foo", fcgi_foo },
	{ "post", bbs_post },
	{ NULL, NULL }
};

static const web_handler_t *_get_handler(void)
{
	char *url = getenv("SCRIPT_NAME");
	if (!url)
		return NULL;
	
	char *name = strrchr(url, '/');
	if (!name)
		name = url;
	else
		++name;
	
	const web_handler_t *h = _handlers;
	while (h->name) {
		if (streq(name, h->name))
			return h;
		++h;
	}
	return NULL;
}

/**
 * The main entrance of bbswebd.
 * @return 0 on success, 1 on initialization error.
 */
int main(void)
{
	config_t cfg;
	config_init(&cfg);
	if (config_load(&cfg, DEFAULT_CFG_FILE) != 0)
		return EXIT_FAILURE;

	db_conn_t *conn = db_connect(config_get(&cfg, "host"),
			config_get(&cfg, "port"), config_get(&cfg, "dbname"),
			config_get(&cfg, "user"), config_get(&cfg, "password"));
	if (db_status(conn) != DB_CONNECTION_OK)
        return EXIT_FAILURE;

	if (chdir(config_get(&cfg, "root")) < 0)
		return EXIT_FAILURE;

	while (FCGI_Accept() >= 0) {
		pool_t *p = pool_create(DEFAULT_POOL_SIZE);

		http_req_t *r = get_request(p);
		if (!r)
			return EXIT_FAILURE;

		web_ctx_t ctx = { .c = &cfg, .d = conn, .p = p, .r = r };

		int ret;
		const web_handler_t *h = _get_handler();
		if (!h) {
			;	
		} else {
			ret = (*(h->func))(&ctx);
		}

		pool_destroy(p);
	}
	return 0;
}
