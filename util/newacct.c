#include "bbs.h"
#include "chart.h"

int main(int argc, char **argv)
{
	struct bbsstat bst;
	memset(&bst, 0, sizeof(bst));
	FILE *fp = fopen(BBSHOME"/trace", "r");
	if (fp == NULL) {
		printf("can't open trace\n");
		return 1;
	}
	time_t now = time(NULL);
	char *date = ctime(&now) + 4;
	int hour;
	char buf[256];
	while (fgets(buf, sizeof(buf), fp)) {
		hour = atoi(buf + 7);
		if (hour < 0 || hour > 23) {
			continue;
		}
		if (strncmp(buf, date, 6))
			continue;
		if (strstr(buf + 16, "new account from")) {
			bst.value[hour]++;
			continue;
		}
	}
	fclose(fp);
	
	int i, total = 0;
	for (i = 0; i < MAX_BARS; i++) {
		bst.color[i] = ANSI_COLOR_RED;
		total += bst.value[i];
	}

	int item = draw_chart(&bst);
	char *mg = left_margin(item, NULL);
	//% printf("\033[0;1m%s0└── %-8.8s 本日新增人口统计 ─── %s ──   \n"
	printf("\033[0;1m%s0\xa9\xb8\xa9\xa4\xa9\xa4 %-8.8s \xb1\xbe\xc8\xd5\xd0\xc2\xd4\xf6\xc8\xcb\xbf\xda\xcd\xb3\xbc\xc6 \xa9\xa4\xa9\xa4\xa9\xa4 %s \xa9\xa4\xa9\xa4   \n"
			"\033[1;36m%s   00 01 02 03 04 05 06 07 08 09 10 11 "
			"\033[32m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
			//% "%s                  \033[1;33m1 \033[31m▇ \033[33m= \033[31m%-5d"
			"%s                  \033[1;33m1 \033[31m\xa8\x7e \033[33m= \033[31m%-5d"
			//% "         \033[33m新申请帐号数: \033[31m%d\n",
			"         \033[33m\xd0\xc2\xc9\xea\xc7\xeb\xd5\xca\xba\xc5\xca\xfd: \033[31m%d\n",
			mg, BBSNAME, format_time(now, TIME_FORMAT_ZH), mg, mg, item, total);
	return 0;
}

