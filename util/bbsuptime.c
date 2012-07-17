#include "fbbs/cfg.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/pool.h"

int main(void)
{
	initialize_environment(INIT_DB | INIT_MDB);

	int online = online_count();
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	printf("  %d:%d%s ,  %d user, \n",
		(now->tm_hour % 12 == 0) ? 12 : (now->tm_hour % 12),
		now->tm_min, (now->tm_hour > 11) ? "pm" : "am", online);
	return 0;
}
