#include "libweb.h"

int bbsqry_main(void)
{
	char userid[IDLEN + 1];
	strlcpy(userid, getparm("u"), sizeof(userid));
	if (!loginok)
		http_fatal("请先登录方可查询");
	struct userec user;
	xml_header("bbsqry");
	fputs("<bbsqry>\n", stdout);
	if (getuserec(userid, &user) != 0) {
		printf("<id>%s</id><login>%d</login><ip>%s</ip>"
				"<lastlogin>%s</lastlogin><perf>%s</perf><post>%d</post>"
				"<hp>%d</hp>",
				user.userid, user.numlogins, user.lasthost,
				getdatestring(user.lastlogin, DATE_XML),
				cperf(countperf(&user)), user.numposts,
				compute_user_value(&user));
		int level, repeat;
		level = iconexp(countexp(&user), &repeat);
		printf("<level>%d</level>\n<repeat>%d</repeat>\n", level, repeat);
		fputs("<nick>", stdout);
		xml_fputs(user.username, stdout);
		fputs("</nick>", stdout);
		if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)) {
			printf("<horo>%s</horo>", 
					horoscope(user.birthmonth, user.birthday));
			if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
				printf("<gender>%c</gender>", user.gender);
		}
		// TODO: mail, logout, identity
	}
	fputs("</bbsqry>", stdout);
	return 0;
}

