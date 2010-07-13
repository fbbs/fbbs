#include <unistd.h>

#include "fbbs/cache.h"
#include "fbbs/socket.h"
#include "fbbs/string.h"

typedef struct cache_server_t {
	int fd;
} cache_server_t;

static cache_server_t server;

/**
 * Initialize a cache client.
 * @return 0 if OK, -1 on error.
 */
int cache_client_init(void)
{
	int fd = unix_dgram_connect(CACHE_CLIENT, CACHE_SERVER);
	if (fd < 0)
		return -1;
	server.fd = fd;
	return 0;
}

/**
 * Submit a cache query and fetch the result.
 * @param query The query.
 * @param len Length of the query.
 * @param result Buffer for the result.
 * @param size The size of the buffer.
 * @return Length of the result, -1 on error.
 */
static int submit_cache_query(const void *query, int len, void *result, int size)
{
	if (write(server.fd, query, len) < 0)
		return 0;
	return read(server.fd, result, size);
}

/**
 * Client call to verify user password.
 * @param user User name.
 * @param passwd The password.
 * @return True if no error occurs the two match, false otherwise.
 */
bool verify_user_passwd(const char *user, const char *passwd)
{
	password_query_t query = { .type = PASSWORD_QUERY };
	strlcpy(query.username, user, sizeof(query.username));
	strlcpy(query.passwd, passwd, sizeof(query.passwd));

	password_result_t res;
	int len = submit_cache_query(&query, sizeof(query), &res, sizeof(res));
	if (len == sizeof(res))
		return res.match;
	return false;
}
