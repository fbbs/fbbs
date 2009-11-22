#include "libweb.h"

int bbsqry_main(void)
{
	char userid[IDLEN + 1];
	strlcpy(userid, getparm("u"), sizeof(userid));
	if (!loginok)
		return BBS_ELGNREQ;
	struct userec user;
	xml_header("bbsqry");
	if (getuserec(userid, &user) != 0) {
		int level, repeat;
		level = iconexp(countexp(&user), &repeat);		
		printf("<bbsqry %s id='%s' login='%d' lastlogin='%s' "
				"perf='%s' post='%d' hp='%d' level='%d' repeat='%d'",
				get_session_str(), user.userid, user.numlogins,
				getdatestring(user.lastlogin, DATE_XML),
				cperf(countperf(&user)), user.numposts,
				compute_user_value(&user), level, repeat);
		if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)) {
			printf(" horo='%s'", 
					horoscope(user.birthmonth, user.birthday));
			if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
				printf(" gender='%c'", user.gender);
		}
		printf("><ip>");
		xml_fputs(mask_host(user.lasthost), stdout);
		printf("</ip><nick>");
		xml_fputs(user.username, stdout);
		printf("</nick><ident>");
		char ident[160];
		show_position(&user, ident, sizeof(ident));
		xml_fputs(ident, stdout);
		printf("</ident><smd>");
		char file[HOMELEN];
		sethomefile(file, user.userid, "plans");
		xml_printfile(file, stdout);
		printf("</smd>");
		// TODO: mail, logout
	} else {
		printf("<bbsqry %s>", get_session_str());
	}
	printf("</bbsqry>");
	return 0;
}

