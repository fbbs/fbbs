/*  averun.c  -- calculate the average logon users per hour  */

#include "bbs.h"
#include "chart.h"

#define AVEFILE BBSHOME"/reclog/ave.src"
#define AVEPIC  BBSHOME"/0Announce/bbslist/today"

int parse_avg(const char *name)
{
	// process 'name'.
	FILE *fp = fopen(name, "r");
	if (fp == NULL)
		return -1;
	char buf[80];
	int hour = -1, avg = 0, i = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (hour < 0) {
			hour = atoi(buf);
			if (strstr(buf, "pm") && hour != 12)
				hour += 12;
			if (strstr(buf, "am") && hour == 12)
                hour = 0;
		}
		avg += atoi(index(buf, ',') + 1);
		++i;		
	}
	if (i == 0)
		avg = 0;
	else
		avg = (avg + i - 1) / i;

	// save AVEFILE.
	if ((fp = fopen(AVEFILE, "a+")) == NULL)
		return -1;
	fprintf(fp, "%d:%d\n", hour, avg);
	fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s crontab_output_filename\n", argv[0]);
		return 1;
	}

	if (parse_avg(argv[1]) < 0)
		return 1;

	struct bbsstat bst;
	FILE *fp = fopen(AVEFILE, "r");
	if (fp == NULL)
		return 1;
	memset(&bst, 0, sizeof(bst));
	char buf[80];
	int hour, count, avg = 0, i = 0;
	// parse AVEFILE. format: "hour:online\n"
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		hour = atoi(buf);
		count = atoi(index(buf, ':') + 1);
		bst.value[hour] = count;
		avg += count;
		i++;
	}
	if (i == 0)
		avg = 0;
	else
		avg = (avg + i - 1) / i;
	for (i = 0; i < MAX_BARS; i++) {
		bst.color[i] = ANSI_COLOR_GREEN;
	}

	int item = draw_chart(&bst);
	printf("\033[0;1m     0└── %-8.8s 平均负载人数统计 ─── %s ──   \n"
			"\033[1;36m        00 01 02 03 04 05 06 07 08 09 10 11 "
			"\033[31m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
			"                       \033[1;37m1 \033[32m▇ \033[37m= \033[36m%-5d"
			"         \033[37m平均负载人数: \033[36m%d\n",
			BBSNAME, getdatestring(time(NULL), DATE_ZH), item, avg);
	return 0;
}
