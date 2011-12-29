#include "libweb.h"
#include <sys/types.h>
#include <unistd.h>
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/status.h"
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
extern int web_props(void);
extern int web_my_props(void);

typedef struct {
	char *name;          ///< name of the cgi.
	int (*func)(void);   ///< handler function.
	int mode;            ///< user mode. @see mode_type
} web_handler_t;

web_ctx_t ctx;

const static web_handler_t handlers[] = {
	{ "sec", bbssec_main, ST_READBRD },
	{ "all", bbsall_main, ST_READBRD },
	{ "boa", bbsboa_main, ST_READNEW },
	{ "login", web_login, ST_LOGIN},
	{ "logout", bbslogout_main, ST_MMENU },
	{ "doc", bbsdoc_main, ST_READING },
	{ "con", bbscon_main, ST_READING },
	{ "pst", bbspst_main, ST_POSTING },
	{ "snd", bbssnd_main, ST_POSTING },
	{ "qry", bbsqry_main, ST_QUERY },
	{ "clear", bbsclear_main, ST_READING },
	{ "upload", bbsupload_main, ST_UPLOAD },
	{ "preupload", bbspreupload_main, ST_UPLOAD },
	{ "0an", bbs0an_main, ST_DIGEST },
	{ "anc", bbsanc_main, ST_DIGEST },
	{ "not", bbsnot_main, ST_READING },
	{ "mail", bbsmail_main, ST_RMAIL },
	{ "mailcon", bbsmailcon_main, ST_RMAIL },
	{ "delmail", bbsdelmail_main, ST_RMAIL },
	{ "gdoc", bbsgdoc_main, ST_READING },
	{ "tdoc" ,bbstdoc_main, ST_READING },
	{ "gcon", bbsgcon_main, ST_READING },
	{ "tcon", bbstcon_main, ST_READING },
	{ "mybrd", web_mybrd, ST_READING },
	{ "brdadd", web_brdadd, ST_READING },
	{ "ccc", bbsccc_main, ST_POSTING },
	{ "fav", web_fav, ST_READING },
	{ "pstmail", bbspstmail_main, ST_SMAIL },
	{ "sndmail", bbssndmail_main, ST_SMAIL },
	{ "fall", bbsfall_main, ST_GMENU },
	{ "fadd", bbsfadd_main, ST_GMENU },
	{ "fdel", bbsfdel_main, ST_GMENU },
	{ "plan", bbsplan_main, ST_EDITUFILE },
	{ "sig", bbssig_main, ST_EDITUFILE },
	{ "del", bbsdel_main, ST_READING },
	{ "fwd", bbsfwd_main, ST_SMAIL },
	{ "info", bbsinfo_main, ST_GMENU },
	{ "pwd", bbspwd_main, ST_GMENU },
	{ "edit", bbsedit_main, ST_EDIT },
	{ "sel", bbssel_main, ST_SELECT },
	{ "rss", bbsrss_main, ST_READING },
	{ "ovr", bbsovr_main, ST_FRIEND },
	{ "top10", bbstop10_main, ST_READING },
	{ "newmail", bbsnewmail_main, ST_RMAIL },
	{ "bfind", bbsbfind_main, ST_READING },
	{ "idle", bbsidle_main, ST_IDLE },
	{ "reg", fcgi_reg, ST_NEW },
	{ "activate", fcgi_activate, ST_NEW },
	{ "exist", fcgi_exist, ST_QUERY },
	{ "sigopt", web_sigopt, ST_GMENU },
	{ "fdoc", web_forum, ST_READING },
	{ "mailman", web_mailman, ST_RMAIL },
	{ "prop", web_props, ST_GMENU },
	{ "myprop", web_my_props, ST_GMENU },
	{ NULL, NULL, 0 }
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

	initialize_convert_env();

	while (FCGI_Accept() >= 0) {
		pool_t *p = pool_create(DEFAULT_POOL_SIZE);

		ctx.p = p;
		ctx.r = get_request(p);
		if (!ctx.r)
			return EXIT_FAILURE;

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
