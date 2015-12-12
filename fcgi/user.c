#include <math.h>
#include "libweb.h"
#include "record.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/money.h"
#include "fbbs/string.h"
#include "fbbs/uinfo.h"
#include "fbbs/web.h"

/**
 * Check user info validity.
 * @return empty string on success, error msg otherwise.
 */
static char *check_info(void)
{
	unsigned char *nick;
	nick = (unsigned char *)web_get_param("nick");
	unsigned char *t2 = nick;
	while (*t2 != '\0') {
		if (*t2 < 0x20 || *t2 == 0xFF)
			//% return "昵称太短或包含非法字符";
			return "\xea\xc7\xb3\xc6\xcc\xab\xb6\xcc\xbb\xf2\xb0\xfc\xba\xac\xb7\xc7\xb7\xa8\xd7\xd6\xb7\xfb";
		t2++;
	}
	strlcpy(currentuser.username, (char *)nick, sizeof(currentuser.username));

	// TODO: more accurate birthday check.
	const char *tmp = web_get_param("year");
	long num = strtol(tmp, NULL, 10);
	if (num < 1910 || num > 1998)
		//% return "错误的出生年份";
		return "\xb4\xed\xce\xf3\xb5\xc4\xb3\xf6\xc9\xfa\xc4\xea\xb7\xdd";
	else
		currentuser.birthyear = num - 1900;

	tmp = web_get_param("month");
	num = strtol(tmp, NULL, 10);
	if (num <= 0 || num > 12)
		//% return "错误的出生月份";
		return "\xb4\xed\xce\xf3\xb5\xc4\xb3\xf6\xc9\xfa\xd4\xc2\xb7\xdd";
	else
		currentuser.birthmonth = num;

	tmp = web_get_param("day");
	num = strtol(tmp, NULL, 10);
	if (num <= 0 || num > 31)
		//% return "错误的出生日期";
		return "\xb4\xed\xce\xf3\xb5\xc4\xb3\xf6\xc9\xfa\xc8\xd5\xc6\xda";
	else
		currentuser.birthday = num;

	tmp = web_get_param("gender");
	if (*tmp == 'M')
		currentuser.gender = 'M';
	else
		currentuser.gender = 'F';

	save_user_data(&currentuser);
	return "";
}

int bbsinfo_main(void)
{
	if (!session_get_id())
		return BBS_ELGNREQ;
	web_parse_post_data();
	const char *type = web_get_param("type");
	xml_header(NULL);
	if (*type != '\0') {
		printf("<bbsinfo>");
		print_session();
		printf("%s</bbsinfo>", check_info());
	} else {
		printf("<bbsinfo post='%d' login='%d' stay='%d' "
				"since='%s' host='%s' year='%d' month='%d' "
				"day='%d' gender='%c' ", currentuser.numposts,
				currentuser.numlogins, currentuser.stay / 60,
				format_time(currentuser.firstlogin, TIME_FORMAT_XML),
				currentuser.lasthost, currentuser.birthyear,
				currentuser.birthmonth, currentuser.birthday,
				currentuser.gender);
		printf(" last='%s'><nick>",
				format_time(currentuser.lastlogin, TIME_FORMAT_XML));
		xml_fputs(currentuser.username);
		printf("</nick>");
		print_session();
		printf("</bbsinfo>");
	}
	return 0;
}

static int set_password(const char *orig, const char *new1, const char *new2)
{
	if (!passwd_check(currentuser.userid, orig))
		return BBS_EWPSWD;

	if (!streq(new1, new2))
		return BBS_EINVAL;

	if (strlen(new1) < 2)
		return BBS_EINVAL;

	return (passwd_set(currentuser.userid, new1) == 0 ? 0 : BBS_EINTNL);
}

int bbspwd_main(void)
{
	if (!session_get_id())
		return BBS_ELGNREQ;
	web_parse_post_data();
	xml_header(NULL);
	printf("<bbspwd ");
	const char *pw1 = web_get_param("pw1");
	if (*pw1 == '\0') {
		printf(" i='i'>");
		print_session();
		printf("</bbspwd>");
		return 0;
	}
	printf(">");
	const char *pw2 = web_get_param("pw2");
	const char *pw3 = web_get_param("pw3");
	switch (set_password(pw1, pw2, pw3)) {
		case BBS_EWPSWD:
			//% printf("密码错误");
			printf("\xc3\xdc\xc2\xeb\xb4\xed\xce\xf3");
			break;
		case BBS_EINVAL:
			//% printf("新密码不匹配 或 新密码太短");
			printf("\xd0\xc2\xc3\xdc\xc2\xeb\xb2\xbb\xc6\xa5\xc5\xe4 \xbb\xf2 \xd0\xc2\xc3\xdc\xc2\xeb\xcc\xab\xb6\xcc");
			break;
		case BBS_EINTNL:
			//% printf("内部错误");
			printf("\xc4\xda\xb2\xbf\xb4\xed\xce\xf3");
		default:
			break;
	}
	print_session();
	printf("</bbspwd>");
	return 0;
}

typedef struct mail_count_t {
	time_t limit;
	int count;
} mail_count_t;

static int count_new_mail(void *buf, int count, void *args)
{
	struct fileheader *fp = buf;
	mail_count_t *cp = args;
	time_t date = getfiletime(fp);
	if (date < cp->limit)
		return QUIT;
	if (!(fp->accessed[0] & FILE_READ))
		cp->count++;
	return 0;
}

int bbsidle_main(void)
{
	if (!session_get_id())
		return BBS_ELGNREQ;
	printf("Content-type: text/xml; charset="CHARSET"\n\n"
			"<?xml version=\"1.0\" encoding=\""CHARSET"\"?>\n<bbsidle");
	char file[HOMELEN];
	setmdir(file, currentuser.userid);
	mail_count_t c;
	c.limit = time(NULL) - 24 * 60 * 60 * NEWMAIL_EXPIRE;
	c.count = 0;
	apply_record(file, count_new_mail, sizeof(struct fileheader), &c, false,
			true, true);
	printf(" mail='%d'></bbsidle>", c.count);
	return 0;
}

static int edit_user_file(const char *file, const char *desc, const char *submit)
{
	if (!session_get_id())
		return BBS_ELGNREQ;
	char buf[HOMELEN];
	sethomefile(buf, currentuser.userid, file);
	web_parse_post_data();
	const char *text = web_get_param("text");
	if (*text != '\0') {
		int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			return BBS_EINTNL;

		file_lock_all(fd, FILE_WRLCK);
		file_write(fd, text, strlen(text));
		file_lock_all(fd, FILE_UNLCK);
		close(fd);

		xml_header(NULL);
		printf("<bbseufile desc='%s'>", desc);
		print_session();
		printf("</bbseufile>");
	} else {
		xml_header(NULL);
		printf("<bbseufile desc='%s' submit='%s'><text>", desc, submit);
		xml_printfile(buf);
		printf("</text>");
		print_session();
		printf("</bbseufile>");
	}
	return 0;
}

int bbsplan_main(void)
{
	//% return edit_user_file("plans", "编辑说明档", "plan");
	return edit_user_file("plans", "\xb1\xe0\xbc\xad\xcb\xb5\xc3\xf7\xb5\xb5", "plan");
}

int bbssig_main(void)
{
	//% return edit_user_file("signatures", "编辑签名档", "sig");
	return edit_user_file("signatures", "\xb1\xe0\xbc\xad\xc7\xa9\xc3\xfb\xb5\xb5", "sig");
}

static int show_sessions(user_id_t user_id)
{
	if (user_id <= 0)
		return 0;
	db_res_t *res = db_query("SELECT id, visible, web FROM sessions s"
			" WHERE active AND user_id = %"DBIdUID, user_id);
	if (!res)
		return 0;

	int num = 0;
	fb_time_t now = fb_time();
	for (int i = 0; i < db_res_rows(res); ++i) {
		bool visible = db_get_bool(res, i, 1);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		++num;

		session_id_t sid = db_get_session_id(res, i, 0);
		bool web = db_get_bool(res, i, 2);

		fb_time_t refresh = session_get_idle(sid);
		int status = get_user_status(sid);

		int idle;
		if (refresh < 1 || status == ST_BBSNET)
			idle = 0;
		else
			idle = (now - refresh) / 60;

		char buf[32];
		const char *descr = session_status_descr(status);
		if (web_request_type(UTF8))
			convert_g2u(descr, buf);
		else
			strlcpy(buf, descr, sizeof(buf));
		printf("<st vis='%d' web='%d' idle='%d' desc='%s'/>",
				visible, web, idle, buf);
	}

	db_clear(res);
	return num;
}

// Convert exp to icons.
static int iconexp(int exp, int *repeat)
{
	int i = 0, j;

	if (exp < 0)
		j = -1;
	else {
		exp = sqrt(exp / 5);
		i = exp / 10;
		i = i > 5 ? 5 : i;
		j = exp - i * 10;
		j = j > 9 ? 9 : j;
	}
	*repeat = ++j;
	return i;
}

static const char *perf_string(int perf)
{
	static UTF8_BUFFER(str, 4);
	const char *gbk_perf = cperf(perf);
	if (web_request_type(UTF8)) {
		convert_g2u(gbk_perf, utf8_str);
		return utf8_str;
	}
	return gbk_perf;
}

static const char *horoscope_string(const struct userec *user)
{
	static UTF8_BUFFER(horo, 3);
	const char *gbk_horo = horoscope(user->birthmonth, user->birthday);
	if (web_request_type(UTF8)) {
		convert_g2u(gbk_horo, utf8_horo);
		return utf8_horo;
	}
	return gbk_horo;
}

static int session_info(user_id_t user_id, json_array_t *sessions)
{
	if (user_id <= 0)
		return 0;
	db_res_t *res = db_query("SELECT id, visible, web FROM sessions s"
			" WHERE active AND user_id = %"DBIdUID, user_id);
	if (!res)
		return 0;

	int num = 0;
	fb_time_t now = fb_time();
	for (int i = 0; i < db_res_rows(res); ++i) {
		bool visible = db_get_bool(res, i, 1);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		++num;

		session_id_t session_id = db_get_session_id(res, i, 0);
		bool web = db_get_bool(res, i, 2);

		fb_time_t refresh = session_get_idle(session_id);
		int status = get_user_status(session_id);

		int idle;
		if (refresh < 1 || status == ST_BBSNET)
			idle = 0;
		else
			idle = (now - refresh) / 60;

		char buf[32];
		const char *descr = session_status_descr(status);
		convert_g2u(descr, buf);

		json_object_t *obj = json_object_new();
		if (!visible)
			json_object_bool(obj, "hidden", true);
		json_object_bool(obj, "web", web);
		json_object_integer(obj, "idle", idle);
		json_object_string(obj, "status", buf);
		json_array_append(sessions, obj, JSON_OBJECT);
	}

	db_clear(res);
	return num;
}

typedef struct {
	json_object_t *pos;
	json_array_t *bm;
	json_array_t *admin;
} user_position_json_callback_t;

static void user_position_json_callback(const char *position, void *arg,
		user_position_e type)
{
	user_position_json_callback_t *a = arg;
	switch (type) {
		case USER_POSITION_BM_BEGIN: {
			a->bm = json_array_new();
			break;
		}
		case USER_POSITION_BM:
			json_array_string(a->bm, position);
			break;
		case USER_POSITION_BM_END:
			break;
		case USER_POSITION_CUSTOM:
			a->pos = json_object_new();
			json_object_string(a->pos, "custom", position);
			break;
		default:
			if (!a->admin)
				a->admin = json_array_new();
			json_array_string(a->admin, position);
	}
}

static void user_position_json(const struct userec *user, const char *title,
		json_object_t *object)
{
	user_position_json_callback_t arg = {
		.pos = NULL, .bm = NULL, .admin = NULL,
	};
	user_position(user, title, user_position_json_callback, &arg);

	if (arg.pos || arg.bm || arg.admin) {
		if (!arg.pos)
			arg.pos = json_object_new();
		if (arg.bm)
			json_object_append(arg.pos, "bm", arg.bm, JSON_ARRAY);
		if (arg.admin)
			json_object_append(arg.pos, "admin", arg.admin, JSON_ARRAY);

		json_object_append(object, "position", arg.pos, JSON_OBJECT);
	}
}

int api_user(void)
{
	if (!session_get_id())
		return WEB_ERROR_LOGIN_REQUIRED;

	struct userec user;
	if (!getuserec(web_get_param("name"), &user))
		return WEB_ERROR_USER_NOT_FOUND;

	bool simple = web_get_param_long("s");
	bool self = streq(currentuser.userid, user.userid);

	json_object_t *object = json_object_new();
	web_set_response(object, JSON_OBJECT);

	json_object_string(object, "name", user.userid);
	json_object_integer(object, "logins", user.numlogins);
	json_object_integer(object, "posts", user.numposts);

	UTF8_BUFFER(nick, NAMELEN / 2);
	convert_g2u(user.username, utf8_nick);
	json_object_string(object, "nick", utf8_nick);

	if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)) {
		json_object_string(object, "horo", horoscope_string(&user));
		if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX)) {
			char gender[] = { user.gender, '\0' };
			json_object_string(object, "gender", gender);
		}
	}

	uinfo_t u;
	uinfo_load(user.userid, &u);
#ifdef ENABLE_BANK
	if (!simple) {
		if (self || HAS_PERM2(PERM_OCHAT, &currentuser)) {
			json_object_integer(object, "money", TO_YUAN_INT(u.money));
		}
		json_object_integer(object, "contrib", TO_YUAN_INT(u.contrib));
		json_object_integer(object, "rank", (int) u.rank * 100);
	}
#endif
	user_position_json(&user, u.title, object);
	uinfo_free(&u);

	if (simple)
		return WEB_OK;

	json_object_bigint(object, "login", time_to_ms(user.lastlogin));
	json_object_integer(object, "perf", countperf(&user));
	json_object_integer(object, "hp", compute_user_value(&user));
	json_object_string(object, "ip",
			self ? user.lasthost : mask_host(user.lasthost));

	char file[HOMELEN];
	sethomefile(file, user.userid, "plans");
	char *plan = file_read_all(file);
	if (plan) {
		size_t len = strlen(plan) * 2 + 1;
		char *s = malloc(len);
		convert_g2u(plan, s);
		json_object_string(object, "plan", s);
		free(s);
		free(plan);
	}

	json_array_t *sessions = json_array_new();
	json_object_append(object, "sessions", sessions, JSON_ARRAY);

	user_id_t user_id = get_user_id(user.userid);
	int num = session_info(user_id, sessions);
	if (!num) {
		fb_time_t logout = user.lastlogout;
		if (logout < user.lastlogin) {
			logout = ((time(NULL) - user.lastlogin) / 120) % 47 + 1
				+ user.lastlogin;
		}
		json_object_bigint(object, "logout", time_to_ms(logout));
	}

	return WEB_OK;
}

int bbsqry_main(void)
{
	char user_name[IDLEN + 1];
	strlcpy(user_name, web_get_param("u"), sizeof(user_name));
	if (!session_get_id())
		return BBS_ELGNREQ;
	struct userec user;
	int uid;
	xml_header(NULL);
	uid = getuserec(user_name, &user);

	bool self = streq(currentuser.userid, user.userid);

	if (uid != 0) {
		int level, repeat;
		level = iconexp(countexp(&user), &repeat);		
		printf("<bbsqry id='%s' login='%d' lastlogin='%s' "
				"perf='%s' post='%d' hp='%d' level='%d' repeat='%d' ",
				user.userid, user.numlogins,
				format_time(user.lastlogin, TIME_FORMAT_XML),
				perf_string(countperf(&user)), user.numposts,
				compute_user_value(&user), level, repeat);

		uinfo_t u;
		uinfo_load(user.userid, &u);
#ifdef ENABLE_BANK
		if (self || HAS_PERM2(PERM_OCHAT, &currentuser)) {
			printf("money='%d' ", TO_YUAN_INT(u.money));
		}
		printf("contrib='%d' rank='%.1f' ",
				TO_YUAN_INT(u.contrib), PERCENT_RANK(u.rank));
#endif
		if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)) {
			printf("horo='%s'",
					horoscope_string(&user));
			if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
				printf(" gender='%c'", user.gender);
		}
		printf("><ip>");
		xml_fputs(self ? user.lasthost : mask_host(user.lasthost));
		printf("</ip><nick>");
		xml_fputs4(user.username, 0);
		printf("</nick><ident>");

		GBK_UTF8_BUFFER(ident, 80);
		user_position_string(&user, u.title, utf8_ident, sizeof(utf8_ident));
		convert_u2g(utf8_ident, gbk_ident);
		xml_fputs4(gbk_ident, 0);

		uinfo_free(&u);

		printf("</ident><smd>");
		char file[HOMELEN];
		sethomefile(file, user.userid, "plans");
		xml_printfile(file);
		printf("</smd>");

		user_id_t user_id = get_user_id(user.userid);
		int num = show_sessions(user_id);
		if (!num) {
			fb_time_t logout = user.lastlogout;
			if (logout < user.lastlogin) {
				logout = ((time(NULL) - user.lastlogin) / 120) % 47 + 1
						+ user.lastlogin;
			}
			printf("<logout>%s</logout>",
					format_time(logout, TIME_FORMAT_XML));
		}
		// TODO: mail
	} else {
		printf("<bbsqry id='%s'>", user_name);
	}
	print_session();
	printf("</bbsqry>");
	return 0;
}
