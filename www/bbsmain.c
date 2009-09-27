#include "libweb.h"
#include <sys/types.h>
#include <unistd.h>

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
int bbsfadd_main(void);
int bbsfdel_main(void);

typedef struct {
	char *name;          ///< name of the cgi.
	int (*func) (void);  ///< handler function.
	int mode;            ///< user mode. @see mode_type
} web_handler_t;

static web_handler_t applets[] = {
		{"sec", bbssec_main, READBRD},
		{"all", bbsall_main, READBRD},
		{"boa", bbsboa_main, READNEW},
		{"login", bbslogin_main, LOGIN},
		{"logout", bbslogout_main, MMENU},
		{"doc", bbsdoc_main, READING},
		{"con", bbscon_main, READING},
		{"pst", bbspst_main, POSTING},
		{"snd", bbssnd_main, POSTING},
		{"qry", bbsqry_main, QUERY},
		{"clear", bbsclear_main, READING},
		{"upload", bbsupload_main, BBSST_UPLOAD},
		{"preupload", bbspreupload_main, BBSST_UPLOAD},
		{"0an", bbs0an_main, DIGEST},
		{"anc", bbsanc_main, DIGEST},
		{"not", bbsnot_main, READING},
		{"mail", bbsmail_main, RMAIL},
		{"mailcon", bbsmailcon_main, RMAIL},
		{"delmail", bbsdelmail_main, RMAIL},
		{"gdoc", bbsgdoc_main, READING},
		{"tdoc" ,bbstdoc_main, READING},
		{"gcon", bbsgcon_main, READING},
		{"tcon", bbstcon_main, READING},
		{"mybrd", bbsmybrd_main, READING},
		{"brdadd", bbsbrdadd_main, READING},
		{"ccc", bbsccc_main, POSTING},
		{"fav", bbsfav_main, READING},
		{"pstmail", bbspstmail_main, SMAIL},
		{"sndmail", bbssndmail_main, SMAIL},
		{"fall", bbsfall_main, GMENU},
		{"fadd", bbsfadd_main, GMENU},
		{"fdel", bbsfdel_main, GMENU},
		{NULL, NULL, 0}
};

/**
 * Get an web request handler according to its name.
 * @return handler pointer if found, NULL otherwise.
 */
static web_handler_t *getapplet(void)
{
	char *surl = getenv("SCRIPT_NAME");
	if(surl == NULL)
		return NULL;
	char *result = strrchr(surl, '/');
	if (result == NULL)
		result = surl;
	else
		result++;	
	web_handler_t *app = applets;
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

/**
 * The main entrance of bbswebd.
 * @return 0 on success, 1 on initialization error.
 */
int main(void)
{
	if (fcgi_init_all() < 0) {
		check_bbserr(BBS_EINTNL);
		return 1;
	}
	while (FCGI_Accept() >= 0) {
		fcgi_init_loop();
		web_handler_t *app = getapplet();
		int ret;
		if (app == NULL) {
			ret = BBS_ENOURL;
		} else {
			if (loginok) {
				u_info->mode = get_web_mode(app->mode);
				u_info->idle_time = time(NULL);
			}
			ret = (*(app->func)) ();
		}
		check_bbserr(ret);
	}
	return 0;
}
