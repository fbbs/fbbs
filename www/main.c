#include <stdlib.h>
#include <string.h>
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

typedef struct web_handler_t {
	const char *name;           ///< name of the handler.
	int (*func)(http_req_t *);  ///< handler function.
} web_handler_t;

int fcgi_foo(http_req_t *r)
{
	html_header();
	printf("Hello, world!\n</head></html>");
	return 0;
}

static const web_handler_t _handlers[] = {
	{ "foo", fcgi_foo },
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
	while (FCGI_Accept() >= 0) {
		pool_t *p = pool_create(DEFAULT_POOL_SIZE);

		http_req_t *r = get_request(p);
		if (!r)
			return EXIT_FAILURE;

		int ret;
		const web_handler_t *h = _get_handler();
		if (!h) {
			;	
		} else {
			ret = (*(h->func))(r);
		}

		pool_destroy(p);
	}
	return 0;
}
