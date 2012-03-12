#include "libweb.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

/**
 * Print override information.
 * @param buf the starting address of override struct.
 * @param count not used.
 * @param args not used.
 * @return 0.
 */
static int print_override(void *buf, int count, void *args)
{
	override_t *ov = buf;
	printf("<ov id='%s'>", ov->id);
	xml_fputs(ov->exp, stdout);
	printf("</ov>");
	return 0;
}

int bbsfall_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header(NULL);
	printf("<bbsfall>");
	print_session();
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, "friends");
	apply_record(file, print_override, sizeof(override_t), NULL, false,
			false, true);
	printf("</bbsfall>");
	return 0;
}

static int cmpname(void *arg, void *buf)
{
	override_t *ov = (override_t *)buf;
	return !strncasecmp(arg, ov->id, sizeof(ov->id));
}

static int cmp_override(const void *key, const void *buf)
{
	override_t *ov = (override_t *)buf;
	return strncasecmp(((override_t *)key)->id, ov->id, sizeof(ov->id));
}

int bbsfadd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	const char *id = get_param("id");
	const char *desc = get_param("desc");
	if (*id != '\0') {
		override_t ov;
		memset(&ov, 0, sizeof(ov));
		strlcpy(ov.id, id, sizeof(ov.id));
		if (!searchuser(ov.id))
			return BBS_ENOUSR;
		strlcpy(ov.exp, desc, sizeof(ov.exp));
		char file[HOMELEN];
		sethomefile(file, currentuser.userid, "friends");
		if (get_num_records(file, sizeof(ov)) == MAXFRIENDS)
			return BBS_EFRNDQE;
		// TODO: be atomic
		if (!search_record(file, NULL, sizeof(ov), cmpname, ov.id))
			append_record(file, &ov, sizeof(ov));
		printf("Location: fall\n\n");
		return 0;
	}
	xml_header(NULL);
	printf("<bbsfadd>");
	print_session();
	printf("%s</bbsfadd>", id);
	return 0;
}

int bbsfdel_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	const char *user = get_param("u");
	if (*user != '\0') {
		char file[HOMELEN];
		sethomefile(file, currentuser.userid, "friends");
		record_t r;
		if (record_open(file, O_RDWR, &r) < 0)
			return BBS_EINTNL;
		override_t key;
		strlcpy(key.id, user, sizeof(key.id));
		override_t *ptr =
				record_search(&r, &key, sizeof(key), lsearch, cmp_override);
		if (ptr != NULL)
			record_delete(&r, ptr, sizeof(*ptr));
		record_close(&r);
	}
	printf("Location: fall\n\n");
	return 0;
}

static void show_sessions_of_friends(void)
{
	db_res_t *res = get_sessions_of_followings();
	if (!res)
		return;

	fb_time_t now = time(NULL);
	for (int i = 0; i < db_res_rows(res); ++i) {
		bool visible = db_get_bool(res, i, 2);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		session_id_t sid = db_get_session_id(res, i, 0);
		const char *uname = db_get_value(res, i, 1);
		const char *ip = db_get_value(res, i, 3);
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
				uname, mode_type(status), idle, ip);
		xml_fputs(user.username, stdout);
		printf("</ov>");
	}
	db_clear(res);
}

int bbsovr_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header(NULL);
	printf("<bbsovr>");
	print_session();

	show_sessions_of_friends();

	printf("</bbsovr>");
	return 0;
}
