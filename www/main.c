#include "libweb.h"
#include <sys/types.h>
#include <unistd.h>
#include "fbbs/web.h"

void check_bbserr(int err);
extern int bbssec_main(web_ctx_t *ctx);
extern int bbsall_main(web_ctx_t *ctx);
extern int bbsboa_main(web_ctx_t *ctx);
extern int bbslogin_main(web_ctx_t *ctx);
extern int bbslogout_main(web_ctx_t *ctx);
extern int bbsdoc_main(web_ctx_t *ctx);
extern int bbscon_main(web_ctx_t *ctx);
extern int bbspst_main(web_ctx_t *ctx);
extern int bbssnd_main(web_ctx_t *ctx);
extern int bbsqry_main(web_ctx_t *ctx);
extern int bbsclear_main(web_ctx_t *ctx);
extern int bbsupload_main(web_ctx_t *ctx);
extern int bbspreupload_main(web_ctx_t *ctx);
extern int bbs0an_main(web_ctx_t *ctx);
extern int bbsanc_main(web_ctx_t *ctx);
extern int bbsnot_main(web_ctx_t *ctx);
extern int bbsmail_main(web_ctx_t *ctx);
extern int bbsmailcon_main(web_ctx_t *ctx);
extern int bbsdelmail_main(web_ctx_t *ctx);
extern int bbsgdoc_main(web_ctx_t *ctx);
extern int bbstdoc_main(web_ctx_t *ctx);
extern int bbsgcon_main(web_ctx_t *ctx);
extern int bbstcon_main(web_ctx_t *ctx);
extern int bbsmybrd_main(web_ctx_t *ctx);
extern int bbsbrdadd_main(web_ctx_t *ctx);
extern int bbsccc_main(web_ctx_t *ctx);
extern int bbsfav_main(web_ctx_t *ctx);
extern int bbspstmail_main(web_ctx_t *ctx);
extern int bbssndmail_main(web_ctx_t *ctx);
extern int bbsfall_main(web_ctx_t *ctx);
extern int bbsfadd_main(web_ctx_t *ctx);
extern int bbsfdel_main(web_ctx_t *ctx);
extern int bbsplan_main(web_ctx_t *ctx);
extern int bbssig_main(web_ctx_t *ctx);
extern int bbsdel_main(web_ctx_t *ctx);
extern int bbsfwd_main(web_ctx_t *ctx);
extern int bbsinfo_main(web_ctx_t *ctx);
extern int bbspwd_main(web_ctx_t *ctx);
extern int bbsedit_main(web_ctx_t *ctx);
extern int bbssel_main(web_ctx_t *ctx);
extern int bbsrss_main(web_ctx_t *ctx);
extern int bbsovr_main(web_ctx_t *ctx);
extern int bbstop10_main(web_ctx_t *ctx);
extern int bbsnewmail_main(web_ctx_t *ctx);
extern int bbsbfind_main(web_ctx_t *ctx);
extern int bbsidle_main(web_ctx_t *ctx);
extern int fcgi_reg(web_ctx_t *ctx);
extern int fcgi_activate(web_ctx_t *ctx);
extern int fcgi_exist(web_ctx_t *ctx);

typedef struct {
	char *name;          ///< name of the cgi.
	int (*func) (web_ctx_t *);  ///< handler function.
	int mode;            ///< user mode. @see mode_type
} web_handler_t;

const static web_handler_t applets[] = {
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
		{"plan", bbsplan_main, EDITUFILE},
		{"sig", bbssig_main, EDITUFILE},
		{"del", bbsdel_main, READING},
		{"fwd", bbsfwd_main, SMAIL},
		{"info", bbsinfo_main, GMENU},
		{"pwd", bbspwd_main, GMENU},
		{"edit", bbsedit_main, EDIT},
		{"sel", bbssel_main, SELECT},
		{"rss", bbsrss_main, READING},
		{"ovr", bbsovr_main, FRIEND},
		{"top10", bbstop10_main, READING},
		{"newmail", bbsnewmail_main, RMAIL},
		{"bfind", bbsbfind_main, READING},
		{"idle", bbsidle_main, IDLE},
		{"reg", fcgi_reg, NEW},
		{"activate", fcgi_activate, NEW},
		{"exist", fcgi_exist, QUERY},
		{NULL, NULL, 0}
};

/**
 * Get an web request handler according to its name.
 * @return handler pointer if found, NULL otherwise.
 */
static const web_handler_t *getapplet(void)
{
	char *surl = getenv("SCRIPT_NAME");
	if (surl == NULL)
		return NULL;
	char *result = strrchr(surl, '/');
	if (result == NULL)
		result = surl;
	else
		result++;	
	const web_handler_t *app = applets;
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
	if (chdir(BBSHOME) != 0)
		return BBS_EINTNL;
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
	if (fcgi_init_all() < 0)
		return EXIT_FAILURE;

	while (FCGI_Accept() >= 0) {
		pool_t *p = pool_create(DEFAULT_POOL_SIZE);

		http_req_t *r = get_request(p);
		if (!r)
			return EXIT_FAILURE;

		web_ctx_t ctx = { .r = r };
		const web_handler_t *app = getapplet();
		int ret;
		if (app == NULL) {
			ret = BBS_ENOURL;
		} else {
			fcgi_init_loop(&ctx, get_web_mode(app->mode));
#ifndef FDQUAN
			if (!loginok && app->func != bbslogin_main
					&& app->func != fcgi_reg && app->func != fcgi_activate)
				ret = BBS_ELGNREQ;
			else
				ret = (*(app->func))(&ctx);
#else
			ret = (*(app->func))(&ctx);
#endif // FDQUAN
		}
		check_bbserr(ret);
	}
	return 0;
}
