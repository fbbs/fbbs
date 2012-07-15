#include "libweb.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/ucache.h"
#include "fbbs/uinfo.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

enum {
	WEB_ACTIVE_LOGIN_QUOTA = 2,
};

static int check_web_login_quota(const char *uname, bool force)
{
	db_res_t *res = db_query("SELECT s.id"
			" FROM sessions s JOIN users u ON s.user_id = u.id"
			" WHERE s.active AND s.web AND lower(u.name) = lower(%s)", uname);
	if (!res)
		return INT_MAX;

	int count = db_res_rows(res);
	if (force) {
		// TODO: terminate extra web sessions
		;
	}
	db_clear(res);
	return count;
}

static char *digest_to_hex(const uchar_t *digest, char *buf, size_t size)
{
	const char *str = "0123456789abcdef";
	for (int i = 0; i < size / 2; ++i) {
		buf[i * 2] = str[(digest[i] & 0xf0) >> 4];
		buf[i * 2 + 1] = str[digest[i] & 0x0f];
	}
	buf[size - 1] = '\0';
	return buf;
}

static char *generate_session_key(char *buf, size_t size, session_id_t sid)
{
	struct {
		time_t sec;
		int usec;
		int random;
		session_id_t sid;
	} s;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	s.sec = tv.tv_sec;
	s.usec = tv.tv_usec;

	s.random = rand();

	s.sid = sid;

	gcry_md_reset(ctx.sha1);
	gcry_md_write(ctx.sha1, &s, sizeof(s));
	gcry_md_final(ctx.sha1);

	const uchar_t *digest = gcry_md_read(ctx.sha1, 0);
	return digest_to_hex(digest, buf, size);
}

static void grant_permission(struct userec *user)
{
	if (!HAS_PERM2(PERM_REGISTER, user) && HAS_PERM2(PERM_BINDMAIL, user)) {
		char file[HOMELEN];
		sethomefile(file, user->userid, "register");
		if (dashf(file)) {
			user->userlevel |= PERM_DEFAULT;
		}
	}
}

static const char *get_login_referer(void)
{
	const char *next = get_param("next");
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

static int login_screen(void)
{
	http_header();
	const char *ref = get_login_referer();
	printf("<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
			"<link rel='stylesheet' type='text/css' href='../css/%s.css' />"
			"<title>”√ªßµ«¬º - "BBSNAME"</title></head>"
			"<body><form action='login' method='post'>"
			"<label for='id'>’ ∫≈</label><input type='text' name='id' /><br />"
			"<label for='pw'>√‹¬Î</label><input type='password' name='pw' /><br />"
			"<input type='hidden' name='ref' value='%s'/>"
			"<input type='submit' value='µ«¬º' />"
			"</form></body></html>",
			(ctx.r->flag & REQUEST_MOBILE) ? "mobile" : "bbs", ref);
	return 0;
}

void login_redirect(const char *key)
{
	const char *referer = get_param("ref");
	if (*referer == '\0')
		referer = "sec";

	// TODO: these cookies should be merged into one.
	printf("Content-type: text/html; charset=%s\n"
			"Set-cookie: utmpkey=%s\n"
			"Set-cookie: utmpuserid=%s\nLocation: %s\n\n",
			CHARSET, key, currentuser.userid, referer);
}

int web_login(void)
{
	if (parse_post_data() < 0)
		return BBS_EINVAL;

	const char *uname = get_param("id");
	if (*uname == '\0' || strcaseeq(uname, "guest"))
		return login_screen();

	if (session.uid && streq(uname, currentuser.userid)) {
		const char *ref = get_login_referer();
		printf("Location: %s\n\n", ref);
		return 0;
	}

	struct userec user;
	if (getuserec(uname, &user) == 0)
		return BBS_ENOUSR;
	session.uid = get_user_id(uname);

	char pw[PASSLEN];
	strlcpy(pw, get_param("pw"), sizeof(pw));
	if (!passwd_check(uname, pw)) {
		log_attempt(user.userid, fromhost, "web");
		return BBS_EWPSWD;
	}

	int sessions = check_web_login_quota(uname, false);
	if (!HAS_PERM2(PERM_SYSOPS, &user) && sessions >= WEB_ACTIVE_LOGIN_QUOTA)
		return BBS_ELGNQE;

	if (!HAS_PERM2(PERM_LOGIN, &user))
		return BBS_EACCES;

	time_t now = time(NULL);
	if (now - user.lastlogin >= 20 * 60
			|| user.numlogins < 100)
		user.numlogins++;

#ifdef CHECK_FREQUENTLOGIN
	time_t last = user.lastlogin;
	if (!HAS_PERM(PERM_SYSOPS)
			&& abs(last - now) < 10) {
		report("Too Frequent", user.userid);
		return BBS_ELFREQ;
	}
#endif

	update_user_stay(&user, true, sessions >= 1);

	grant_permission(&user);

	strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));
	save_user_data(&user);
	currentuser = user;

	char key[SESSION_KEY_LEN + 1];
	session.id = session_new_id();
	generate_session_key(key, sizeof(key), session.id);
	session.id = session_new(key, session.id, session.uid, fromhost,
			SESSION_WEB, SESSION_PLAIN);

	log_usies("ENTER", fromhost, &user);

	login_redirect(key);
	return 0;
}

static void logout(void)
{
	update_user_stay(&currentuser, false, false);
	save_user_data(&currentuser);
}

int web_logout(void)
{
	if (!session.id) {
		printf("Location: sec\n\n");
		return 0;
	}

	logout();

	printf("Set-cookie: utmpnum=;expires=Fri, 19-Apr-1996 11:11:11 GMT\n"
			"Set-cookie: utmpkey=;expires=Fri, 19-Apr-1996 11:11:11 GMT\n"
			"Set-cookie: utmpuserid=;expires=Fri, 19-Apr-1996 11:11:11 GMT\n"
			"Location: sec\n\n");
	return 0;
}
