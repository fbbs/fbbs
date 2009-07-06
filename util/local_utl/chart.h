#ifndef CHART_H

enum {
	ANSI_COLOR_BLACK = 30,
	ANSI_COLOR_RED,
	ANSI_COLOR_GREEN,
	ANSI_COLOR_YELLOW,
	ANSI_COLOR_BLUE,
	ANSI_COLOR_PURPLE,
	ANSI_COLOR_CYAN,
	ANSI_COLOR_WHITE
};

enum {
	MAX_BARS = 24,
	MAX_HEIGHT = 15
};

struct bbsstat {
	int color[MAX_BARS];
	int value[MAX_BARS];
};

int draw_chart(const struct bbsstat *st);
#endif
