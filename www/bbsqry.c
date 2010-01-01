#include "libweb.h"

int bbsqry_main(void)
{
	char userid[IDLEN + 1];
	strlcpy(userid, getparm("u"), sizeof(userid));
	if (!loginok)
		return BBS_ELGNREQ;
	struct userec user;
	int uid;
	xml_header("bbsqry");
	uid = getuserec(userid, &user);
	if (uid != 0) {
		int level, repeat;
		level = iconexp(countexp(&user), &repeat);		
		printf("<bbsqry id='%s' login='%d' lastlogin='%s' "
				"perf='%s' post='%d' hp='%d' level='%d' repeat='%d' ",
				user.userid, user.numlogins,
				getdatestring(user.lastlogin, DATE_XML),
				cperf(countperf(&user)), user.numposts,
				compute_user_value(&user), level, repeat);
		print_session();
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

		// TODO: blacklist?
		int num = 0;
		struct user_info *uinfo = utmpshm->uinfo;
		for (int i = 0; i < USHM_SIZE; ++i, ++uinfo) {
			if (uinfo->active && uinfo->uid == uid) {
				if (uinfo->invisible && !HAS_PERM(PERM_SEECLOAK))
					continue;
				num++;
				int idle = (time(NULL) - uinfo->idle_time) / 60;
				if (idle < 1 || get_raw_mode(uinfo->mode) == BBSNET)
					idle = 0;
				printf("<st vis='%d' web='%d' idle='%d' desc='%s'/>",
						!uinfo->invisible, is_web_user(uinfo->mode),
						idle, mode_type(uinfo->mode));
			}
		}
		if (!num) {
			time_t logout = user.lastlogout;
			if (logout < user.lastlogin) {
				logout = ((time(NULL) - user.lastlogin) / 120) % 47 + 1
						+ user.lastlogin;
			}
			printf("<logout>%s</logout>",
					getdatestring(logout, DATE_XML));
		}
		// TODO: mail
	} else {
		printf("<bbsqry ");
		print_session();
		printf(" id='%s'>", userid);
	}
	printf("</bbsqry>");
	return 0;
}

