#include "bbs.h"
#include "list.h"

typedef struct {
	bool mode;
} online_users_t;

static int online_users_load(choose_t *cp)
{
	return 0;
}

static void online_users_title(choose_t *cp)
{
	online_users_t *up = cp->data;
	docmdtitle(up->mode ? "[好朋友列表]" : "[使用者列表]",
			" 聊天[\033[1;32mt\033[m] 寄信[\033[1;32mm\033[m] 送讯息["
			"\033[1;32ms\033[m] 加,减朋友[\033[1;32mo\033[m,\033[1;32md\033[m]"
			" 看说明档[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 切换模式 "
			"[\033[1;32mf\033[m] 求救[\033[1;32mh\033[m]");

}

static int online_users_display(choose_t *cp)
{
	return 0;
}

static int online_users_handler(choose_t *cp, int ch)
{
	return 0;
}


int online_users(void)
{
	choose_t cs;
	cs.data = NULL;
	cs.loader = online_users_load;
	cs.title = online_users_title;
	cs.display = online_users_display;
	cs.handler = online_users_handler;
	
	choose2(&cs);
}
