#include <stdarg.h>

#include "fbbs/fbbs.h"
#include "fbbs/mdbi.h"

int mdb_connect_unix(const char *path)
{
	env.m->c = redisConnectUnix(path);
	if (env.m->c || env.m->c->err)
		return 0;
	return -1;
}

mdb_res_t *mdb_cmd(const char *cmd, ...)
{
	va_list ap, aq;
	va_start(ap, cmd);

	va_copy(aq, ap);
	size_t size = vsnprintf(env.m->buf, sizeof(env.m->buf), cmd, aq);
	va_end(aq);

	if (size >= sizeof(env.m->buf)) {
		char *buf = malloc(size + 1);
		vsnprintf(buf, size + 1, cmd, ap);
		va_end(ap);
		void *r = redisCommand(env.m->c, buf);
		free(buf);
		return r;
	} else {
		va_end(ap);
		return redisCommand(env.m->c, env.m->buf);
	}
}
