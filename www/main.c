#include "libweb.h"
#include <unistd.h>
#include <sys/types.h>
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

void check_bbserr(int err);
extern int bbssec_main(void);
extern int web_all_boards(void);
extern int web_sector(void);
extern int web_login(void);
extern int web_logout(void);
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

extern int api_board_all(void);
extern int api_board_fav(void);
extern int api_board_toc(void);

typedef struct {
	const char *name;    ///< name of the cgi.
	int (*func)(void);   ///< handler function.
	int status;          ///< user status. @see status_descr
} web_handler_t;

char fromhost[IP_LEN];

const static web_handler_t handlers[] = {
	{ "0an", bbs0an_main, ST_DIGEST },
	{ "activate", fcgi_activate, ST_NEW },
	{ "all", web_all_boards, ST_READBRD },
	{ "anc", bbsanc_main, ST_DIGEST },
	{ "bfind", bbsbfind_main, ST_READING },
	{ "boa", web_sector, ST_READNEW },
	{ "board-all", api_board_all, ST_READBRD },
	{ "board-fav", api_board_fav, ST_READNEW },
	{ "board-toc", api_board_toc, ST_READING },
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
	{ "logout", web_logout, ST_MMENU },
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

	char buf[16];
	strlcpy(buf, name, sizeof(buf));
	char *tmp = strrchr(buf, '.');
	if (tmp)
		*tmp = '\0';

	web_handler_t h = { .name = buf, .func = NULL, .status = 0 };
	return bsearch(&h, handlers, ARRAY_SIZE(handlers), sizeof(h),
			compare_handler);
}

/**
 * Initialization before entering FastCGI loop.
 * @return 0 on success, -1 on error.
 */
static int initialize(void)
{
	srand(time(NULL) * 2 + getpid());

	if (chdir(BBSHOME) != 0)
		return -1;

	if (setuid(BBSUID) != 0)
		return -1;
	if (setgid(BBSGID) != 0)
		return -1;

	if (resolve_ucache() == -1)
		return -1;

	if (resolve_boards() < 0)
		return -1;
	if (!brdshm)
		return -1;

	return 0;
}

/**
 * Get client IP address.
 */
static void get_client_ip(void)
{
#ifdef SQUID
	char *from;
	from = strrchr(getsenv("HTTP_X_FORWARDED_FOR"), ',');
	if (from == NULL) {
		strlcpy(fromhost, getsenv("HTTP_X_FORWARDED_FOR"), sizeof(fromhost));
	} else {
		while ((*from < '0') && (*from != '\0'))
			from++;
		strlcpy(fromhost, from, sizeof(fromhost));
	}
#else
	strlcpy(fromhost, getsenv("REMOTE_ADDR"), sizeof(fromhost));
#endif
}

static bool require_login(const web_handler_t *h)
{
#ifdef FDQUAN
	int (*f)(void) = h->func;
	return !(f == web_login || f == fcgi_reg || f == fcgi_activate);
#else
	return false;
#endif
}

static int execute(const web_handler_t *h)
{
	if (!session.id && require_login(h)) {
		if (request_type(REQUEST_API))
			return error_msg(ERROR_LOGIN_REQUIRED);
		return BBS_ELGNREQ;
	}
	return h->func();
}

/**
 * The main entrance of bbswebd.
 * @return 0 on success, 1 on initialization error.
 */
int main(void)
{
	if (initialize() < 0)
		return EXIT_FAILURE;
	initialize_environment(INIT_CONV | INIT_DB | INIT_MDB);

	while (FCGI_Accept() >= 0) {
		if (!web_ctx_init())
			return EXIT_FAILURE;

		const web_handler_t *h = _get_handler();
		int code = BBS_ENOURL;
		if (h) {
			get_client_ip();
			get_session();

			if (session.id) {
				set_user_status(h->status);
				set_idle_time(session.id, time(NULL));
			}

			code = execute(h);
		}

		if (code > 0)
			respond(code);
		else
			check_bbserr(code);

		web_ctx_destroy();
	}
	return 0;
}
