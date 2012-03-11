#include "bbs.h"

int main(void)
{
	resolve_utmp();
	int online = count_online();
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	printf("  %d:%d%s ,  %d user, \n",
		(now->tm_hour % 12 == 0) ? 12 : (now->tm_hour % 12),
		now->tm_min, (now->tm_hour > 11) ? "pm" : "am", online);
	return 0;
}
