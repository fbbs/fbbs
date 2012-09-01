#include <stdio.h>
#include <string.h>
#include "chart.h"

enum {
	NUMBLKS = 8
};

static int max_value(const struct bbsstat *st)
{
	int i, max = st->value[0];
	for (i = 1; i < MAX_BARS; ++i)
		if (st->value[i] > max)
			max = st->value[i];
	return max;
}

char *left_margin(int item, int *left)
{
	static char buf[MAX_LEFT_MARGIN + 1];
	int count = snprintf(buf, sizeof(buf), "%d", item * (MAX_HEIGHT + 1));
	if (left != NULL)
		*left = count;
	memset(buf, ' ', --count);
	buf[count] = '\0';
	return buf;
}

int draw_chart(const struct bbsstat *st)
{
	char *blk[NUMBLKS] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};

	int max = max_value(st);
	int item = (max + MAX_HEIGHT - 1)/ MAX_HEIGHT;
	if (item == 0)
		item = 1;
	max = item * MAX_HEIGHT;

	int i, j, height, lastcolor, left;
	char str[20], fstr[20];
	char *mg = left_margin(item, &left);
	sprintf(fstr, "\033[1;%%dm%%%dd│", left);
	if (max < 1000)
		printf("\033[1;37m%s ┌────────────────────────────────────\n", mg);
	else
		printf("\033[1;37m%s ┌────超过1000只显示前三位数字────────────────────\n", mg);
	for (i = MAX_HEIGHT + 1; i > 0; --i) {
		lastcolor = ANSI_COLOR_WHITE;
		printf(fstr, lastcolor, i * item);
		for (j = 0; j < MAX_BARS; ++j) {
			height = st->value[j] * NUMBLKS * MAX_HEIGHT / max;
			if (height >= i * NUMBLKS) {
				if (lastcolor == st->color[j]) {
					printf("%s ", blk[NUMBLKS - 1]);
				} else {
					lastcolor = st->color[j];
					printf("\033[%dm%s ", lastcolor, blk[NUMBLKS - 1]);
				}
			} else if (height > (i - 1) * NUMBLKS && height < i * NUMBLKS) {
				if (lastcolor == st->color[j]) {
					printf("%s ", blk[height % NUMBLKS - 1]);
				} else {
					lastcolor = st->color[j];
					printf("\033[%dm%s ", lastcolor, blk[height % NUMBLKS - 1]);
				}
			} else if (height > (i - 2) * NUMBLKS && height <= (i - 1) * NUMBLKS) {
				if (st->value[j] == 0)
					sprintf(str, "   ");
				else
					sprintf(str, "%d", st->value[j]);
				if (lastcolor == ANSI_COLOR_WHITE) {
					printf("%-3.3s", str);
				} else {
					lastcolor = ANSI_COLOR_WHITE;
					printf("\033[%dm%-3.3s", lastcolor, str);
				}
			} else {
				printf("   ");
			}
		}
		printf("\n");
	}
	return item;	
}

