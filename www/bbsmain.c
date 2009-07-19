#include "libweb.h"

int bbsleft_main(void);
int bbssec_main(void);
int bbsfoot_main(void);
int bbsgetmsg_main(void);
int bbsall_main(void);
int bbsboa_main(void);
int bbslogin_main(void);
int bbslogout_main(void);
int bbsdoc_main(void);
int bbscon_main(void);
int bbspst_main(void);
int bbssnd_main(void);
int bbsqry_main(void);
int bbsclear_main(void);
int bbsupload_main(void);
int bbspreupload_main(void);
int bbs0an_main(void);
int bbsanc_main(void);
int bbsnot_main(void);
int bbssendmsg_main(void);
int bbsmail_main(void);
int bbsmailcon_main(void);

struct cgi_applet {
	char *name;
	int (*func) (void);
};

static struct cgi_applet applets[] = {
		{ "bbsleft", bbsleft_main },
		{ "bbssec", bbssec_main },
		{ "bbsfoot", bbsfoot_main },
		{ "bbsgetmsg", bbsgetmsg_main },
		{ "bbsall", bbsall_main },
		{ "bbsboa", bbsboa_main },
		{ "bbslogin", bbslogin_main},
		{ "bbslogout", bbslogout_main},
		{ "bbsdoc", bbsdoc_main},
		{ "bbscon", bbscon_main},
		{ "bbspst", bbspst_main},
		{ "bbssnd", bbssnd_main},
		{ "bbsqry", bbsqry_main},
		{ "bbsclear", bbsclear_main},
		{ "bbsupload", bbsupload_main},
		{ "bbspreupload", bbspreupload_main},
		{ "bbs0an", bbs0an_main},
		{ "bbsanc", bbsanc_main},
		{ "bbsnot", bbsnot_main},
		{ "bbssendmsg", bbssendmsg_main},
		{ "bbsmail", bbsmail_main},
		{ "bbsmailcon", bbsmailcon_main},
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
			http_fatal2(HTTP_STATUS_NOTFOUND, "请求的页面不存在");
		} else {
			(*(app->func)) ();
		}
	}
	return 0;
}
