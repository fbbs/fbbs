#include "libweb.h"
#include <unistd.h>
#include <sys/types.h>
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/status.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

void check_bbserr(int err);
extern int bbssec_main(void);
extern int web_all_boards(void);
extern int web_sector(void);
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
extern int web_sel(void);
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
extern int web_buy_prop(void);

typedef struct {
	const char *name;    ///< name of the cgi.
	int (*func)(void);   ///< handler function.
	int mode;            ///< user mode. @see mode_type
} web_handler_t;

web_ctx_t ctx;

const static web_handler_t handlers[] = {
	{ "0an", bbs0an_main, ST_DIGEST },
	{ "activate", fcgi_activate, ST_NEW },
	{ "all", web_all_boards, ST_READBRD },
	{ "anc", bbsanc_main, ST_DIGEST },
	{ "bfind", bbsbfind_main, ST_READING },
	{ "boa", web_sector, ST_READNEW },
	{ "brdadd", web_brdadd, ST_READING },
	{ "buyprop", web_buy_prop, ST_PROP },
	{ "ccc", bbsccc_main, ST_POSTING },
	{ "clear", bbsclear_main, ST_READING },
	{ "con", bbscon_main, ST_READING },
	{ "del", bbsdel_main, ST_READING },
	{ "delmail", bbsdelmail_main, ST_RMAIL },
	{ "doc", bbsdoc_main, ST_READING },
	{ "edit", bbsedit_main, ST_EDIT },
	{ "exist", fcgi_exist, ST_QUERY },
	{ "fadd", bbsfadd_main, ST_GMENU },
	{ "fall", bbsfall_main, ST_GMENU },
	{ "fav", web_fav, ST_READING },
	{ "fdel", bbsfdel_main, ST_GMENU },
	{ "fdoc", web_forum, ST_READING },
	{ "fwd", bbsfwd_main, ST_SMAIL },
	{ "gcon", bbsgcon_main, ST_READING },
	{ "gdoc", bbsgdoc_main, ST_READING },
	{ "idle", bbsidle_main, ST_IDLE },
	{ "info", bbsinfo_main, ST_GMENU },
	{ "login", web_login, ST_LOGIN},
	{ "logout", bbslogout_main, ST_MMENU },
	{ "mail", bbsmail_main, ST_RMAIL },
	{ "mailcon", bbsmailcon_main, ST_RMAIL },
	{ "mailman", web_mailman, ST_RMAIL },
	{ "myprop", web_my_props, ST_MY_PROP },
	{ "newmail", bbsnewmail_main, ST_RMAIL },
	{ "not", bbsnot_main, ST_READING },
	{ "ovr", bbsovr_main, ST_FRIEND },
	{ "plan", bbsplan_main, ST_EDITUFILE },
	{ "preupload", bbspreupload_main, ST_UPLOAD },
	{ "prop", web_props, ST_PROP },
	{ "pst", bbspst_main, ST_POSTING },
	{ "pstmail", bbspstmail_main, ST_SMAIL },
	{ "pwd", bbspwd_main, ST_GMENU },
	{ "qry", bbsqry_main, ST_QUERY },
	{ "reg", fcgi_reg, ST_NEW },
	{ "rss", bbsrss_main, ST_READING },
	{ "sec", bbssec_main, ST_READBRD },
	{ "sel", web_sel, ST_SELECT },
	{ "sig", bbssig_main, ST_EDITUFILE },
	{ "sigopt", web_sigopt, ST_GMENU },
	{ "snd", bbssnd_main, ST_POSTING },
	{ "sndmail", bbssndmail_main, ST_SMAIL },
	{ "tcon", bbstcon_main, ST_READING },
	{ "tdoc" ,bbstdoc_main, ST_READING },
	{ "top10", bbstop10_main, ST_READING },
	{ "upload", bbsupload_main, ST_UPLOAD },
};

bbs_env_t env;

static int compare_handler(const void *l, const void *r)
{
	const web_handler_t *h1 = l, *h2 = r;
	return strcmp(h1->name, h2->name);
}

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

	web_handler_t h = { .name = name, .func = NULL, .mode = 0 };
	return bsearch(&h, handlers, NELEMS(handlers), sizeof(h), compare_handler);
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
