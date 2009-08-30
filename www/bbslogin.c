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
	FILE *fp;
	char buf[STRLEN];
	int n, tmp;
	struct user_info *u;

	if(!(currentuser.userlevel & PERM_REGISTER)) { 
		char file[HOMELEN]; 
		sethomefile(file, currentuser.userid, "register");
		if(dashf(file)) { 
			currentuser.userlevel |=PERM_DEFAULT; 
			save_user_data(&currentuser);
		}
	}

	fp=fopen("tmp/.UTMP.lock", "a");
	FLOCK(fileno(fp), LOCK_EX);
	for(n = 0; n < MAXACTIVE; n++) {
		if(utmpshm->uinfo[n].active == 0) {
			u = &(utmpshm->uinfo[n]);
			u_info = u;
			bzero(u, sizeof(struct user_info));
			u->active = 1;
			u->uid = searchuser(user->userid);
			u->pid = getpid();
			u->mode = WWW;
			if (HAS_PERM(PERM_LOGINCLOAK)
					&& (currentuser.flags[0] & CLOAK_FLAG))
				u->invisible = YEA;
			u->pager = 0;
			if (DEFINE(DEF_FRIENDCALL))
				u->pager |= FRIEND_PAGER;
			if (DEFINE(PAGER_FLAG)) {
				u->pager |= ALL_PAGER;
				u->pager |= FRIEND_PAGER;
			}
			if (DEFINE(DEF_FRIENDMSG))
				u->pager |= FRIENDMSG_PAGER;
			if (DEFINE(DEF_ALLMSG)) {
				u->pager |= ALLMSG_PAGER;
				u->pager |= FRIENDMSG_PAGER;
			}

			SpecialID(u->userid, fromhost, IPLEN);
			strlcpy(u->from, fromhost, 24);//???
#ifdef SPARC
			*(int*)(u->from + 30)=time(NULL);
#else
			*(int*)(u->from + 32)=time(NULL);
#endif
			u->from[22] = DEFINE(DEF_NOTHIDEIP) ? 'S' : 'H';

			u->idle_time = time(NULL);
			strlcpy(u->username, user->username, sizeof(u->username));
			strlcpy(u->userid, user->userid, sizeof(u->userid));

			tmp = rand() % 100000000;
			u->utmpkey = tmp;
			FLOCK(fileno(fp), LOCK_UN);
			fclose(fp);
			http_header();
			sprintf(buf, "%d", n + 1);
			setcookie("utmpnum", buf);
			sprintf(buf, "%d", tmp);
			setcookie("utmpkey", buf);
			setcookie("utmpuserid", currentuser.userid);
			set_my_cookie();

			const char *referer = ref;
			if (*referer == '\0') {
				referer = "sec";
			}
			refreshto(1, referer);
			printf("</head>\n<body>登录成功，1秒钟后自动转到<a href='%s'>"
					"登录前页面</a>\n</body>\n</html>\n", referer);

			uidshm->status[u->uid - 1]++;
			return 0;
		}
	}
	FLOCK(fileno(fp), LOCK_UN);
	fclose(fp);
	return BBS_E2MANY;
	return 0;
}

static login_screen(void)
{
	http_header();
	const char *referer = get_referer();
	const char *ref;
	if (!strcmp(referer, "/") || !strcmp(referer, "/index.htm"))
		ref = "sec";
	else
		ref = referer;
	printf("<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
			"<link rel='stylesheet' type='text/css' href='/css/bbs.css' />"
			"<title>用户登录 - "BBSNAME"</title></head>"
			"<body><form action='login' method='post'>"
			"<label for='id'>帐号</label><input type='text' name='id' /><br />"
			"<label for='pw'>密码</label><input type='password' name='pw' /><br />"
			"<input type='hidden' name='ref' value='%s'/>"
			"<input type='submit' value='登录' />"
			"</form></body></html>", ref);
	return 0;
}

int bbslogin_main(void)
{
	char fname[STRLEN];
	char buf[256], id[IDLEN + 1], pw[PASSLEN];
	struct userec user;

	parse_post_data();
	strlcpy(id, getparm("id"), sizeof(id));
	if (*id == '\0')
		return login_screen();
	strlcpy(pw, getparm("pw"), sizeof(pw));
	if(loginok && strcasecmp(id, currentuser.userid))
		return BBS_EDUPLGN;
	if (getuserec(id, &user) == 0)
		return BBS_ENOUSR;

	user.numlogins++;
	if (strcasecmp(id, "guest")) {
		int total;
		time_t stay, recent, now, t;
		if (!checkpasswd(user.passwd, pw)) {
			sprintf(buf, "%-12.12s %s @%s\n",
					user.userid, cn_Ctime(time(NULL)), fromhost);
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
	if(!loginok && strcasecmp(id, "guest"))
		wwwlogin(&user, getparm("ref"));
	return 0;
}
