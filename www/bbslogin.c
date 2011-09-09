#include "libweb.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/string.h"
#include "fbbs/ucache.h"
#include "fbbs/uinfo.h"
#include "fbbs/web.h"

static int check_multi(const struct userec *user)
{
	int i, total = 0;
	int uid = searchuser(user->userid);

	int idle_time = time(NULL);
	struct user_info *idle_session = NULL;

	for (i = 0; i < MAXACTIVE; i++) {
		struct user_info *up = utmpshm->uinfo + i;
		if (up->active == 0)
			continue;
		if (up->uid == uid) {
			total++;
			if (is_web_user(up->mode) && up->idle_time < idle_time) {
				idle_time = up->idle_time;
				idle_session = up;
			}
		}
	}

	int max = get_login_quota(user);
	if (total == max && idle_session) {
		if (bbskill(idle_session, SIGHUP) == 0)
			--total;
	}

	return total;
}

static int wwwlogin(struct userec *user, const char *ref)
{
	if (!HAS_PERM(PERM_REGISTER) && HAS_PERM(PERM_BINDMAIL)) {
		char file[HOMELEN]; 
		sethomefile(file, currentuser.userid, "register");
		if (dashf(file)) {
			currentuser.userlevel |= PERM_DEFAULT;
			save_user_data(&currentuser);
		}
	}

	struct user_info info;
	memset(&info, 0, sizeof(info));
	info.active = 1;
	info.uid = searchuser(user->userid);
	info.pid = getpid();
	info.mode = WWW | LOGIN;
	if (HAS_PERM(PERM_LOGINCLOAK)
			&& (currentuser.flags[0] & CLOAK_FLAG))
		info.invisible = YEA;
	info.pager = 0;
	if (DEFINE(DEF_FRIENDCALL))
		info.pager |= FRIEND_PAGER;
	if (DEFINE(PAGER_FLAG)) {
		info.pager |= ALL_PAGER;
		info.pager |= FRIEND_PAGER;
	}
	if (DEFINE(DEF_FRIENDMSG))
		info.pager |= FRIENDMSG_PAGER;
	if (DEFINE(DEF_ALLMSG)) {
		info.pager |= ALLMSG_PAGER;
		info.pager |= FRIENDMSG_PAGER;
	}

	strlcpy(info.from, fromhost, sizeof(info.from));

	info.idle_time = time(NULL);
	strlcpy(info.username, user->username, sizeof(info.username));
	strlcpy(info.userid, user->userid, sizeof(info.userid));

	int utmpkey = rand() % 100000000;
	info.utmpkey = utmpkey;

	int fd = open("tmp/.UTMP.lock", O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		return BBS_EINTNL;
	if (fb_flock(fd, LOCK_EX) == -1) {
		close(fd);
		return BBS_EINTNL;
	}

	struct user_info *up = utmpshm->uinfo;
	int n;
	for (n = 0; n < MAXACTIVE; n++, up++) {
		if (!up->active) {
			*up = info;
			uidshm->status[up->uid - 1]++;
			break;
		}
	}
	fb_flock(fd, LOCK_UN);
	close(fd);
	if (n >= MAXACTIVE)
		return BBS_E2MANY;
	
	const char *referer = ref;
	if (*referer == '\0')
		referer = "sec";
	// TODO: these cookies should be merged into one.
	printf("Content-type: text/html; charset=%s\n"
			"Set-cookie: utmpnum=%d\nSet-cookie: utmpkey=%d\n"
			"Set-cookie: utmpuserid=%s\nLocation: %s\n\n",
			CHARSET, n + 1, utmpkey, currentuser.userid, referer);
	return 0;
}

static const char *get_login_referer(web_ctx_t *ctx)
{
	const char *next = get_param(ctx->r, "next");
	if (*next != '\0' && !strchr(next, '.'))
		return next;
	const char *referer = get_referer();
	const char *ref;
	if (!strcmp(referer, "/") || !strcmp(referer, "/index.htm")
			|| strstr(referer, "login"))
		ref = "sec";
	else
		ref = referer;
	return ref;
}

static int login_screen(web_ctx_t *ctx)
{
	http_header();
	const char *ref = get_login_referer(ctx);
	printf("<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
			"<link rel='stylesheet' type='text/css' href='../css/%s.css' />"
			"<title>”√ªßµ«¬º - "BBSNAME"</title></head>"
			"<body><form action='login' method='post'>"
			"<label for='id'>’ ∫≈</label><input type='text' name='id' /><br />"
			"<label for='pw'>√‹¬Î</label><input type='password' name='pw' /><br />"
			"<input type='hidden' name='ref' value='%s'/>"
			"<input type='submit' value='µ«¬º' />"
			"</form></body></html>",
			(ctx->r->flag & REQUEST_MOBILE) ? "mobile" : "bbs", ref);
	return 0;
}

int web_login(web_ctx_t *ctx)
{
	char fname[STRLEN];
	char buf[256], id[IDLEN + 1], pw[PASSLEN];
	struct userec user;

	if (parse_post_data(ctx->r) < 0)
		return BBS_EINVAL;
	strlcpy(id, get_param(ctx->r, "id"), sizeof(id));
	if (*id == '\0')
		return login_screen(ctx);
	strlcpy(pw, get_param(ctx->r, "pw"), sizeof(pw));
	if (loginok && !strcasecmp(id, currentuser.userid)) {
		const char *ref = get_login_referer(ctx);
		printf("Location: %s\n\n", ref);
		return 0;
	}
	if (getuserec(id, &user) == 0)
		return BBS_ENOUSR;

	user.numlogins++;
	if (strcasecmp(id, "guest")) {
		int total;
		time_t now = time(NULL);
		if (!passwd_check(user.userid, pw)) {
			sprintf(buf, "%-12.12s %s @%s\n", user.userid,
					getdatestring(now, DATE_ZH), fromhost);
			sethomefile(fname, user.userid, "logins.bad"); 
			file_append(fname, buf);
			file_append("logins.bad", buf);
			return BBS_EWPSWD;
		}

		total = check_multi(&user);
		if (!HAS_PERM2(PERM_SYSOPS, &user) && total >= 2)
			return BBS_ELGNQE;

		if (!HAS_PERM2(PERM_LOGIN, &user))
			return BBS_EACCES;

		// Do not count frequent logins.
		if (now - user.lastlogin < 20 * 60
				&& user.numlogins >= 100)
			user.numlogins--;

#ifdef CHECK_FREQUENTLOGIN
		time_t last = user.lastlogin;
		if (!HAS_PERM(PERM_SYSOPS)
				&& abs(last - now) < 10) {
			report("Too Frequent", user.userid);
			return BBS_ELFREQ;
		}
#endif

		update_user_stay(&user, true, total >= 1);

		strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));
		save_user_data(&user);
		currentuser = user;
	}

	log_usies("ENTER", fromhost, &user);
	if (!loginok && strcasecmp(id, "guest"))
		wwwlogin(&user, get_param(ctx->r, "ref"));
	return 0;
}
