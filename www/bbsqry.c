#include "libweb.h"
#include "fbbs/helper.h"
#include "fbbs/money.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/uinfo.h"
#include "fbbs/web.h"

static int show_sessions(const char *uname)
{
	int num = 0;
	db_res_t *res = db_query("SELECT s.id, s.visible, s.web"
			" FROM sessions s JOIN alive_users u ON s.user_id = u.id"
			" WHERE s.active AND lower(u.name) = lower(%s)", uname);
	if (!res)
		return 0;

	fb_time_t now = fb_time();
	for (int i = 0; i < db_res_rows(res); ++i) {
		bool visible = db_get_bool(res, i, 1);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		++num;

		session_id_t sid = db_get_session_id(res, i, 0);
		bool web = db_get_bool(res, i, 2);

		fb_time_t refresh = get_idle_time(sid);
		int status = get_user_status(sid);

		int idle;
		if (refresh < 1 || status == ST_BBSNET)
			idle = 0;
		else
			idle = (now - refresh) / 60;

		printf("<st vis='%d' web='%d' idle='%d' desc='%s'/>",
				visible, web, idle, status_descr(status));
	}

	db_clear(res);
	return num;
}

int bbsqry_main(void)
{
	char userid[IDLEN + 1];
	strlcpy(userid, get_param("u"), sizeof(userid));
	if (!session.id)
		return BBS_ELGNREQ;
	struct userec user;
	int uid;
	xml_header(NULL);
	uid = getuserec(userid, &user);

	bool self = streq(currentuser.userid, user.userid);

	if (uid != 0) {
		int level, repeat;
		level = iconexp(countexp(&user), &repeat);		
		printf("<bbsqry id='%s' login='%d' lastlogin='%s' "
				"perf='%s' post='%d' hp='%d' level='%d' repeat='%d' ",
				user.userid, user.numlogins,
				format_time(user.lastlogin, TIME_FORMAT_XML),
				cperf(countperf(&user)), user.numposts,
				compute_user_value(&user), level, repeat);

		uinfo_t u;
		uinfo_load(user.userid, &u);
#ifdef ENABLE_BANK
		printf("contrib='%d' rank='%.1f'",
				TO_YUAN_INT(u.contrib), PERCENT_RANK(u.rank));
		if (self || HAS_PERM2(PERM_OCHAT, &currentuser)) {
			printf("money='%d' ", TO_YUAN_INT(u.money));
		}
		printf("contrib='%d' rank='%.1f' ",
				TO_YUAN_INT(u.contrib), PERCENT_RANK(u.rank));
#endif
		if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)) {
			printf("horo='%s'",
					horoscope(user.birthmonth, user.birthday));
			if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
				printf(" gender='%c'", user.gender);
		}
		printf("><ip>");
		xml_fputs(self ? user.lasthost : mask_host(user.lasthost), stdout);
		printf("</ip><nick>");
		xml_fputs(user.username, stdout);
		printf("</nick><ident>");
		char ident[160];
		show_position(&user, ident, sizeof(ident), u.title);
		xml_fputs(ident, stdout);

		uinfo_free(&u);

		printf("</ident><smd>");
		char file[HOMELEN];
		sethomefile(file, user.userid, "plans");
		xml_printfile(file);
		printf("</smd>");

		int num = show_sessions(user.userid);
		if (!num) {
			time_t logout = user.lastlogout;
			if (logout < user.lastlogin) {
				logout = ((time(NULL) - user.lastlogin) / 120) % 47 + 1
						+ user.lastlogin;
			}
			printf("<logout>%s</logout>",
					format_time(logout, TIME_FORMAT_XML));
		}
		// TODO: mail
	} else {
		printf("<bbsqry id='%s'>", userid);
	}
	print_session();
	printf("</bbsqry>");
	return 0;
}

