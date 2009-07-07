/* account.c  -- count for no. of logins */

#include "bbs.h"
#include "chart.h"

int main(int argc, char **argv)
{
	struct {
		int num[24];	// number of logins
		int sum[24];	// sum of online time
	} st;
	memset(&st, 0, sizeof(st));
	FILE *fp = fopen("/home/bbs/usies", "r");
	if (fp == NULL) {
		printf("can't open usies\n");
		return 1;
	}
	time_t now = time(NULL);
	char *date = ctime(&now) + 4;
	int hour;
	char buf[256], *p;
	while (fgets(buf, sizeof(buf), fp)) {
		hour = atoi(buf + 7);
		if (hour < 0 || hour > 23) {
			continue;
		}
		if (strncmp(buf, date, 6))
			continue;
		if (strstr(buf + 16, "bbsd: ENTER") || strstr(buf + 16, "bbslogin: ")) {
			st.num[hour]++;
			continue;
		}
		if ((p = (char *) strstr(buf + 41, "Stay:")) != NULL) {
			st.sum[hour] += atoi(p + 6);
			continue;
		}
	}
	fclose(fp);

	struct bbsstat bst;
	int i, total = 0, totaltime = 0;
	for (i = 0; i < MAX_BARS; i++) {
		bst.color[i] = ANSI_COLOR_YELLOW;
		bst.value[i] = st.num[i];
		total += st.num[i];
		totaltime += st.sum[i];
	}

	int item = draw_chart(&bst);
	printf("\033[0;1m     0└── %-8.8s 每小时到访人次统计 ── %s ──   \n"
			"\033[1;36m         00 01 02 03 04 05 06 07 08 09 10 11 "
			"\033[31m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
			"                 \033[1;32m1 \033[33m▇ \033[32m= \033[37m%-5d"
			"   \033[32m总上站人次: \033[37m%-9d    \033[32m平均使用时间: \033[37m%d\n",
			BBSNAME, getdatestring(now, DATE_ZH), item, total, 
			(totaltime == 0) ? 0 : totaltime / total + 1);
	return 0;
}
