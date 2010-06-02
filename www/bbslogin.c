#include "libweb.h"

static int check_multi(const struct userec *user)
{
	int i, total = 0;
	int uid = searchuser(user->userid);
	for (i = 0; i < MAXACTIVE; i++) {
		if (utmpshm->uinfo[i].active == 0)
			continue;
		if (utmpshm->uinfo[i].uid == uid)
			total++;
	}
	return total;
}

static int wwwlogin(struct userec *user, const char *ref)
{
	if (!(currentuser.userlevel & PERM_REGISTER)) {
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

// TODO:...
	strlcpy(info.from, fromhost, 24);
// login start..
#ifdef SPARC 
	*(int*)(info.from + 30) = time(NULL);
#else
	*(int*)(info.from + 32) = time(NULL);
#endif
	info.from[22] = DEFINE(DEF_NOTHIDEIP) ? 'S' : 'H';

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

static const char *get_login_referer(void)
{
	const char *referer = get_referer();
	const char *ref;
	if (!strcmp(referer, "/") || !strcmp(referer, "/index.htm"))
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
			"<link rel='stylesheet' type='text/css' href='/css/bbs.css' />"
			"<title>”√ªßµ«¬º - "BBSNAME"</title></head>"
			"<body><form action='login' method='post'>"
			"<label for='id'>’ ∫≈</label><input type='text' name='id' /><br />"
			"<label for='pw'>√‹¬Î</label><input type='password' name='pw' /><br />"
			"<input type='hidden' name='ref' value='%s'/>"
			"<input type='submit' value='µ«¬º' />"
			"</form></body></html>", ref);
	return 0;
}

int bbslogin_main(void)
{
	char fname[STRLEN];
	char buf[256], id[IDLEN + 1], pw[PASSLEN];
	struct userec user;

	if (parse_post_data() < 0)
		return BBS_EINVAL;
	strlcpy(id, getparm("id"), sizeof(id));
	if (*id == '\0')
		return login_screen();
	strlcpy(pw, getparm("pw"), sizeof(pw));
	if (loginok && !strcasecmp(id, currentuser.userid)) {
		const char *ref = get_login_referer();
		printf("Location: %s\n\n", ref);
		return 0;
	}
	if (getuserec(id, &user) == 0)
		return BBS_ENOUSR;

	user.numlogins++;
	if (strcasecmp(id, "guest")) {
		int total;
		time_t stay, recent, now, t;
		if (!checkpasswd(user.passwd, pw)) {
			sprintf(buf, "%-12.12s %s @%s\n", user.userid,
					getdatestring(time(NULL), DATE_ZH), fromhost);
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

		now = time(NULL);
		// Do not count frequent logins.
		if (now - user.lastlogin < 20 * 60
				&& user.numlogins >= 100)
			user.numlogins--;
		if (total > 1) {
			recent = user.lastlogout;
			if (user.lastlogin > recent)
				recent = user.lastlogin;
			stay = now - recent;
			if (stay < 0)
				stay = 0;
		} else {
			stay = 0;
		}
		t = user.lastlogin;
		user.lastlogin = now;
		user.stay += stay;
#ifdef CHECK_FREQUENTLOGIN
		if (!HAS_PERM(PERM_SYSOPS)
				&& abs(t - time(NULL)) < 10) {
			report("Too Frequent", user.userid);
			return BBS_ELFREQ;
		}
#endif
		strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));
		save_user_data(&user);
		currentuser = user;
	}

	log_usies("ENTER", fromhost, &user);
	if (!loginok && strcasecmp(id, "guest"))
		wwwlogin(&user, getparm("ref"));
	return 0;
}
