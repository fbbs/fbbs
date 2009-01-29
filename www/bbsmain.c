#include "libweb.h"

struct cgi_applet {
	char *name;
	int (*func) (void);
};

struct cgi_applet applets[] = {
		{ "bbsleft", bbsleft_main },
		{ "bbssec", bbssec_main },
		{ "bbsfoot", bbsfoot_main },
		{ "bbsgetmsg", bbsgetmsg_main },
		{ "bbsall", bbsall_main },
		{ "bbsboa", bbsboa_main },
		{ "bbslogin", bbslogin_main},
		{ "bbslogout", bbslogout_main},
		{ NULL, NULL }
};

static struct cgi_applet *getapplet(char *buf, size_t len)
{
	char *surl = getenv("SCRIPT_NAME");
	if(surl == NULL)
		return NULL;
	strlcpy(buf, surl, len);
	char *result = strrchr(buf, '/');
	if (result == NULL)
		result = buf;
	else
		result++;	
	struct cgi_applet *app = applets;
	while (app->name != NULL) {
		if (!strcmp(result, app->name))
			return app;
		app++;
	}
	return NULL;
}
	
int main(void)
{
	char buf[STRLEN];

	fcgi_init_all();
	while (FCGI_Accept() >= 0) {
		fcgi_init_loop();
		struct cgi_applet *app = getapplet(buf, sizeof(buf));
		if (app == NULL) {
			http_fatal("请求的页面 %s 不存在", getenv("SCRIPT_NAME"));
		}
		(*(app->func)) ();
	}
	return 0;
}
