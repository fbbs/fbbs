#include "libweb.h"
#include <sys/types.h>
#include <unistd.h>
#include "fbbs/fbbs.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

void check_bbserr(int err);
extern int bbssec_main(void);
extern int bbsall_main(void);
extern int bbsboa_main(void);
extern int web_login(void);
extern int bbslogout_main(void);
extern int bbsdoc_main(void);
extern int bbscon_main(void);
extern int bbspst_main(void);
extern int bbssnd_main(void);
extern int bbsqry_main(void);
extern int bbsclear_main(void);
extern int bbsupload_main(void);
extern int bbspreupload_main(void);
extern int bbs0an_main(void);
extern int bbsanc_main(void);
extern int bbsnot_main(void);
extern int bbsmail_main(void);
extern int bbsmailcon_main(void);
extern int bbsdelmail_main(void);
extern int bbsgdoc_main(void);
extern int bbstdoc_main(void);
extern int bbsgcon_main(void);
extern int bbstcon_main(void);
extern int web_mybrd(void);
extern int web_brdadd(void);
extern int bbsccc_main(void);
extern int web_fav(void);
extern int bbspstmail_main(void);
extern int bbssndmail_main(void);
extern int bbsfall_main(void);
extern int bbsfadd_main(void);
extern int bbsfdel_main(void);
extern int bbsplan_main(void);
extern int bbssig_main(void);
extern int bbsdel_main(void);
extern int bbsfwd_main(void);
extern int bbsinfo_main(void);
extern int bbspwd_main(void);
extern int bbsedit_main(void);
extern int bbssel_main(void);
extern int bbsrss_main(void);
extern int bbsovr_main(void);
extern int bbstop10_main(void);
extern int bbsnewmail_main(void);
extern int bbsbfind_main(void);
extern int bbsidle_main(void);
extern int fcgi_reg(void);
extern int fcgi_activate(void);
extern int fcgi_exist(void);
extern int web_sigopt(void);
extern int web_forum(void);
extern int web_mailman(void);

typedef struct {
	char *name;          ///< name of the cgi.
	int (*func)(void);   ///< handler function.
	int mode;            ///< user mode. @see mode_type
} web_handler_t;

web_ctx_t ctx;

const static web_handler_t handlers[] = {
		{"sec", bbssec_main, READBRD},
		{"all", bbsall_main, READBRD},
		{"boa", bbsboa_main, READNEW},
		{"login", web_login, LOGIN},
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
		{"mybrd", web_mybrd, READING},
		{"brdadd", web_brdadd, READING},
		{"ccc", bbsccc_main, POSTING},
		{"fav", web_fav, READING},
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
		{"sigopt", web_sigopt, GMENU},
		{"fdoc", web_forum, READING},
		{"mailman", web_mailman, RMAIL},
		{NULL, NULL, 0}
};

bbs_env_t env;

/**
 * Get an web request handler according to its name.
 * @return handler pointer if found, NULL otherwise.
 */
static const web_handler_t *_get_handler(void)
{
	char *surl = getenv("SCRIPT_NAME");
	if (!surl)
		return NULL;

	char *name = strrchr(surl, '/');
	if (!name)
		name = surl;
	else
		++name;

	const web_handler_t *h = handlers;
	while (h->name) {
		if (streq(name, h->name))
			return h;
		++h;
	}
	return NULL;
}

/**
 * Initialization before entering FastCGI loop.
 * @return 0 on success, -1 on error.
 */
static int _init_all(void)
{
	srand(time(NULL) * 2 + getpid());

	if (chdir(BBSHOME) != 0)
		return -1;

	seteuid(BBSUID);
	if(geteuid() != BBSUID)
		return -1;

	if (resolve_ucache() == -1)
		return -1;
	resolve_utmp();

	if (resolve_boards() < 0)
		return -1;
	if (!brdshm)
		return -1;

	env.p = pool_create(DEFAULT_POOL_SIZE);
	if (!env.p)
		return -1;

	env.c = config_load(env.p, DEFAULT_CFG_FILE);
	if (!env.c)
		return -1;

	env.d = db_connect(config_get(env.c, "host"), config_get(env.c, "port"),
			config_get(env.c, "dbname"), config_get(env.c, "user"),
			config_get(env.c, "password"));
	if (db_status(env.d) != DB_CONNECTION_OK) {
		db_finish(env.d);
		return -1;
	}

	return 0;
}

/**
 * The main entrance of bbswebd.
 * @return 0 on success, 1 on initialization error.
 */
int main(void)
{
	if (_init_all() < 0)
		return EXIT_FAILURE;

	convert_t u2g, g2u;
	if (convert_open(&u2g, "GBK", "UTF-8") < 0
			|| convert_open(&g2u, "UTF-8", "GBK") < 0)
		return EXIT_FAILURE;

	while (FCGI_Accept() >= 0) {
		pool_t *p = pool_create(DEFAULT_POOL_SIZE);

		ctx.r = get_request(p);
		if (!ctx.r)
			return EXIT_FAILURE;
		ctx.u2g = &u2g;
		ctx.g2u = &g2u;

		const web_handler_t *h = _get_handler();
		int ret;
		if (!h) {
			ret = BBS_ENOURL;
		} else {
			fcgi_init_loop(get_web_mode(h->mode));
#ifdef FDQUAN
			if (!loginok && h->func != web_login && h->func != fcgi_reg
					&& h->func != fcgi_activate)
				ret = BBS_ELGNREQ;
			else
				ret = (*(h->func))();
#else
			ret = (*(h->func))();
#endif // FDQUAN
		}
		check_bbserr(ret);
		pool_destroy(p);
	}
	return 0;
}
