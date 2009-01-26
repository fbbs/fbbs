#include "libweb.h"

// Returns 1 if last mail is unread, 0 otherwise.
static int check_unread_mail(const struct userec *user)
{
	char buf[HOMELEN];
	setmfile(buf, user->userid, DOT_DIR);
	struct fileheader *fh = NULL;
	// Get count of all mails.
	size_t count = get_num_records(buf, sizeof(fh));
	// Get last mail header in .DIR.
	get_record(buf, fh, sizeof(fh), count);
	if (fh != NULL)
		return !(fh->accessed[0] & FILE_READ);
	return 0;
}

int bbsfoot_main(void)
{
	int dt = 0, mail_unread = 0;

	printf("<style type=text/css>\nA{color: #000000}\n</style>\n");
	printf("<body bgcolor=#eeeeee marginwidth=0 marginheight=0>\n");
	if (!loginok) {
		printf("<form action=bbslogin method=post target=_top>");
	} else {
		mail_unread = check_unread_mail(&currentuser);
		if(mail_unread != 0) {
			printf("<bgsound src=/mail.wav>\n");
		}
	}
	printf("<table border=0 cellspacing=0 padding=0 width=100%%><tr><td nowrap>");

	if (!loginok) {
		// Login box.
		printf("用户<input name=id type=text maxlength=12 size=10 class='THINBLUE'>&nbsp;");
		printf("密码<input name=pw type=password maxlength=12 size=10 class='THINBLUE'>&nbsp;");
		printf("<input border=0 src=/images/login.gif type=image align=absmiddle>");
	}
	else
	{
		printf("<img border=0 align=absmiddle src=/images/user_%c.gif>"
				"<a href=bbsqry?userid=%s target=view><b>%s</b></a>&nbsp;",
				currentuser.gender == 'F' ? 'F' : 'M',
				currentuser.userid, currentuser.userid);
		iconexp(countexp(&currentuser));
		if (!HAS_PERM(PERM_BINDMAIL))
		{
			printf("<font color='#ff0000'>您尚未绑定邮箱,请尽快用telnet登陆本站填写邮箱进行绑定认证</font>");
		}
		// ...
#ifdef SPARC
		dt=abs(time(0) - *(int*)(u_info->from+30))/60;
#else
		dt=abs(time(0) - *(int*)(u_info->from+32))/60;
#endif
		u_info->idle_time = time(NULL);
	}

	printf("</td><td nowrap width=100%% ALIGN=RIGHT>");
	printf("<img border=0 src=/images/clock.gif align=absmiddle>[%s </a>] ", cn_Ctime(time(0)));
	printf("<img border=0 src=/images/users.gif align=absmiddle>[<a href=bbsusr target=view>%d</a>] ", count_online());
	if (loginok && mail_unread)
		printf("<img border=0 src=/images/mailw.gif align=absmiddle>"
				"[<a href=bbsmail target=view>有新信<font color=red></font>)</a>]");
	printf("<img border=0 src=/images/water.gif align=absmiddle>[%d小时%d分]", dt/60, dt%60);
	printf("</td></tr></table>");
	if(!loginok)
		printf("</form>");
	printf("<script>setTimeout('self.location=self.location', 240000);</script>");
	printf("</body>");
	return 0;
}


