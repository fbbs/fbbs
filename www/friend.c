#include "libweb.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/convert.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

int bbsfall_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	xml_header(NULL);
	printf("<bbsfall>");
	print_session();

	following_list_t *fl = following_list_load(session_uid());
	if (fl) {
		for (int i = following_list_rows(fl) - 1; i >= 0; --i) {
			printf("<ov id='%s'>", following_list_get_name(fl, i));
			xml_fputs(following_list_get_notes(fl, i));
			printf("</ov>");
		}
	}

	printf("</bbsfall>");
	return 0;
}

int bbsfadd_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	const char *uname = get_param("id");
	const char *note = get_param("desc");

	if (*uname) {
		UTF8_BUFFER(note, FOLLOW_NOTE_CCHARS);
		convert_g2u(note, utf8_note);

		follow(session_uid(), uname, utf8_note);

		printf("Location: fall\n\n");
		return 0;
	}
	xml_header(NULL);
	printf("<bbsfadd>");
	print_session();
	printf("%s</bbsfadd>", uname);
	return 0;
}

int bbsfdel_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	const char *uname = get_param("u");
	if (*uname) {
		user_id_t uid = get_user_id(uname);
		if (uid > 0)
			unfollow(session_uid(), uid);
	}
	printf("Location: fall\n\n");
	return 0;
}

static void show_sessions_of_friends(void)
{
	db_res_t *res = get_sessions_of_followings();
	if (!res)
		return;

	fb_time_t now = fb_time();
	for (int i = 0; i < db_res_rows(res); ++i) {
		bool visible = db_get_bool(res, i, 3);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		session_id_t sid = db_get_session_id(res, i, 0);
		const char *uname = db_get_value(res, i, 2);
		const char *ip = db_get_value(res, i, 4);
		fb_time_t refresh = get_idle_time(sid);
		int status = get_user_status(sid);

		struct userec user;
		getuserec(uname, &user);

		int idle;
		if (refresh < 1 || status == ST_BBSNET)
			idle = 0;
		else
			idle = (now - refresh) / 60;

		if (HAS_DEFINE(user.userdefine, DEF_NOTHIDEIP))
			ip = mask_host(ip);
		else
			ip = "......";

		printf("<ov id='%s' action='%s' idle='%d' ip='%s'>",
				uname, status_descr(status), idle, ip);
		xml_fputs(user.username);
		printf("</ov>");
	}
	db_clear(res);
}

int bbsovr_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	xml_header(NULL);
	printf("<bbsovr>");
	print_session();

	show_sessions_of_friends();

	printf("</bbsovr>");
	return 0;
}
