#include "fbbs/cfg.h"
#include "fbbs/fbbs.h"
#include "fbbs/pool.h"

extern void initialize_db(void);
extern void initialize_mdb(void);

int main(void)
{
	env.p = pool_create(DEFAULT_POOL_SIZE);
	env.c = config_load(env.p, DEFAULT_CFG_FILE);
	initialize_db();
	initialize_mdb();

	int online = online_count();
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	printf("  %d:%d%s ,  %d user, \n",
		(now->tm_hour % 12 == 0) ? 12 : (now->tm_hour % 12),
		now->tm_min, (now->tm_hour > 11) ? "pm" : "am", online);
	return 0;
}
