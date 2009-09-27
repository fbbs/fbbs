#include "libweb.h"

void check_bbserr(int err);
int bbssec_main(void);
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
int bbsmail_main(void);
int bbsmailcon_main(void);
int bbsdelmail_main(void);
int bbsgdoc_main(void);
int bbstdoc_main(void);
int bbsgcon_main(void);
int bbstcon_main(void);
int bbsmybrd_main(void);
int bbsbrdadd_main(void);
int bbsccc_main(void);
int bbsfav_main(void);
int bbspstmail_main(void);
int bbssndmail_main(void);
int bbsfall_main(void);

struct cgi_applet {
	char *name;
	int (*func) (void);
};

static struct cgi_applet applets[] = {
		{ "sec", bbssec_main },
		{ "all", bbsall_main },
		{ "boa", bbsboa_main },
		{ "login", bbslogin_main},
		{ "logout", bbslogout_main},
		{ "doc", bbsdoc_main},
		{ "con", bbscon_main},
		{ "pst", bbspst_main},
		{ "snd", bbssnd_main},
		{ "qry", bbsqry_main},
		{ "clear", bbsclear_main},
		{ "upload", bbsupload_main},
		{ "preupload", bbspreupload_main},
		{ "0an", bbs0an_main},
		{ "anc", bbsanc_main},
		{ "not", bbsnot_main},
		{ "mail", bbsmail_main},
		{ "mailcon", bbsmailcon_main},
		{ "delmail", bbsdelmail_main},
		{ "gdoc", bbsgdoc_main},
		{ "tdoc" ,bbstdoc_main},
		{ "gcon", bbsgcon_main},
		{ "tcon", bbstcon_main},
		{ "mybrd", bbsmybrd_main},
		{ "brdadd", bbsbrdadd_main},
		{ "ccc", bbsccc_main},
		{ "fav", bbsfav_main},
		{ "pstmail", bbspstmail_main},
		{ "sndmail", bbssndmail_main},
		{ "fall", bbsfall_main},
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

/**
 * Initialization before any FastCGI loop.
 * @return 0 on success, BBS_EINTNL on error.
 */
static int fcgi_init_all(void)
{
	srand(time(NULL) * 2 + getpid());
	chdir(BBSHOME);
	seteuid(BBSUID);
	if(geteuid() != BBSUID)
		return BBS_EINTNL;
	if (resolve_ucache() == -1)
		return BBS_EINTNL;
	resolve_utmp();
	if (resolve_boards() < 0)
		return BBS_EINTNL;
	if (utmpshm == NULL || brdshm == NULL)
		return BBS_EINTNL;
	return 0;
}

int main(void)
{
	char buf[STRLEN];

	if (fcgi_init_all() < 0) {
		check_bbserr(BBS_EINTNL);
		return 1;
	}
	while (FCGI_Accept() >= 0) {
		fcgi_init_loop();
		struct cgi_applet *app = getapplet(buf, sizeof(buf));
		int ret;
		if (app == NULL) {
			ret = BBS_ENOURL;
		} else {
			ret = (*(app->func)) ();
		}
		check_bbserr(ret);
	}
	return 0;
}
