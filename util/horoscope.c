#include "bbs.h"
#include "chart.h"

static int horo(char month, char day)
{
	switch (month) {
		case 1:
			if (day < 21)
				return 0;
			else
				return 1;
		case 2:
			if (day < 19)
				return 1;
			else
				return 2;
		case 3:
			if (day < 21)
				return 2;
			else
				return 3;
		case 4:
			if (day < 21)
				return 3;
			else
				return 4;
		case 5:
			if (day < 21)
				return 4;
			else
				return 5;
		case 6:
			if (day < 22)
				return 5;
			else
				return 6;
		case 7:
			if (day < 23)
				return 6;
			else
				return 7;
		case 8:
			if (day < 23)
				return 7;
			else
				return 8;
		case 9:
			if (day < 23)
				return 8;
			else
				return 9;
		case 10:
			if (day < 24)
				return 9;
			else
				return 10;
		case 11:
			if (day < 23)
				return 10;
			else
				return 11;
		case 12:
			if (day < 22)
				return 11;
			else
				return 0;
	}
	return -1;
}


int main(int argc, char **argv)
{
	struct userec user;
	struct bbsstat bst;
	memset(&bst, 0, sizeof(bst));
	FILE *fp = fopen(BBSHOME"/.PASSWDS", "rb");
	if (fp == NULL)
		return 1;
	int i, hr, mtotal = 0, ftotal = 0;
	while (fread(&user, sizeof(user), 1, fp) > 0) {
		if (user.birthmonth == 0)
			continue;
		hr = horo(user.birthmonth, user.birthday);
		if (hr < 0)
			continue;
		if (user.gender == 'M') {
			bst.value[hr * 2]++;
			mtotal++;
		} else {
			bst.value[hr * 2 + 1]++;
			ftotal++;
		}
	}
	fclose(fp);

	for (i = 0; i < 12; i++) {
		bst.color[i * 2] = ANSI_COLOR_CYAN;
		bst.color[i * 2 + 1] = ANSI_COLOR_PURPLE;
	}

	int item = draw_chart(&bst);
	char *mg = left_margin(item, NULL);
	printf("\033[0;1m%s0└─── 目前本站注册使用者星座统计───%s──\n"
			"%s\033[1;33m   摩羯  水瓶  双鱼  牡羊  金牛  双子  巨蟹  狮子  处女  天秤  天蝎  射手\n\n"
			"%s                          \033[1;36m█ 男生 (%d)    \033[35m█ 女生 (%d)\033[m\n",
			mg, getdatestring(time(NULL), DATE_ZH), mg, mg, mtotal, ftotal);
	return 0;
}

