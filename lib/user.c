#include "bbs.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/session.h"
#include "fbbs/string.h"

/** 以用户名查用户ID @mdb_hash */
#define USER_ID_HASH_KEY  "user_id"

struct userec currentuser;

static void set_user_id_cache(const char *uname, user_id_t uid)
{
	char name[IDLEN + 1];
	strlcpy(name, uname, sizeof(name));
	strtolower(name, name);

	mdb_cmd("HSET", USER_ID_HASH_KEY" %s %"PRIdUID, name, uid);
}

static user_id_t get_user_id_cache(const char *uname)
{
	char name[IDLEN + 1];
	strlcpy(name, uname, sizeof(name));
	strtolower(name, name);

	return (user_id_t) mdb_integer(-1, "HGET", USER_ID_HASH_KEY" %s", name);
}

void remove_user_id_cache(const char *uname)
{
	char name[IDLEN + 1];
	strlcpy(name, uname, sizeof(name));
	strtolower(name, name);

	mdb_cmd("HDEL", USER_ID_HASH_KEY" %s", name);
}

/**
 * Get user id by name.
 * @param name The user name.
 * @return user id on success, 0 if not exist, -1 on error.
 */
user_id_t get_user_id(const char *name)
{
	user_id_t uid = get_user_id_cache(name);
	if (uid != -1)
		return uid;

	db_res_t *res = db_query("SELECT id FROM alive_users"
			" WHERE lower(name) = lower(%s)", name);
	if (!res)
		return -1;

	uid = 0;
	if (db_res_rows(res) > 0)
		uid = db_get_user_id(res, 0, 0);
	db_clear(res);

	if (uid > 0)
		set_user_id_cache(name, uid);
	return uid;
}

/** 总用户数统计 @mdb_string */
#define USER_COUNT_CACHE_KEY  "c:users"

enum {
	USER_COUNT_REFRESH_INTERVAL = 3600,
};

int get_user_count(void)
{
	int cached = mdb_integer(-1, "GET", USER_COUNT_CACHE_KEY);
	if (cached >= 0)
		return cached;

	int count = 0;
	db_res_t *res = db_query("SELECT count(*) FROM alive_users");
	if (res && db_res_rows(res) > 0)
		count = db_get_bigint(res, 0, 0);
	db_clear(res);

	mdb_cmd("SET", USER_COUNT_CACHE_KEY" %d", count);
	mdb_cmd("EXPIRE", USER_COUNT_CACHE_KEY" %d", USER_COUNT_REFRESH_INTERVAL);
	return count;
}

static int _user_data_add(const char *name, int uid, int field, int delta)
{
	const char *fields[] = { "logins", "posts", "stay" };
	if (field < 0 || field >= ARRAY_SIZE(fields))
		return -1;

	char query[128];
	int bytes = snprintf(query, sizeof(query),
		"UPDATE users SET %s = %s + %d WHERE %s", fields[field],
		fields[field], delta, name ? "lower(name) = lower(%s)" : "id = %d");
	if (bytes >= sizeof(query))
		return -1;

	db_res_t *res;
	if (name)
		res = db_query(query, name);
	else
		res = db_query(query, uid);
	if (!res)
		return -1;
	return 0;
}

int user_data_add_by_name(const char *name, int field, int delta)
{
	return _user_data_add(name, 0, field, delta);
}

int user_data_add(int uid, int field, int delta)
{
	return _user_data_add(NULL, uid, field, delta);
}

int calc_user_stay(bool is_login, bool is_dup, time_t login, time_t logout)
{
	time_t now = time(NULL);
	time_t last = logout > login ? logout : login;
	int stay = now - last;
	if (stay < 0 || (is_login && !is_dup))
		stay = 0;
	return stay;	
}

/** 用户最近发文时间 @mdb_hash */
#define LAST_POST_TIME_KEY "last_post_time"

int set_my_last_post_time(fb_time_t t)
{
	return !mdb_cmd("HSET", LAST_POST_TIME_KEY" %"PRIdUID" %"PRIdFBT,
			session_get_user_id(), t);
}

fb_time_t get_my_last_post_time(void)
{
	return (fb_time_t) mdb_integer(0, "HGET",
			LAST_POST_TIME_KEY" %"PRIdUID, session_get_user_id());
}

static void show_bm(const char *user_name, user_position_callback_t callback,
		void *arg)
{
	char file[HOMELEN], tmp[STRLEN];
	sethomefile(file, user_name, ".bmfile");

	FILE *fp = fopen(file, "r");
	if (fp) {
		callback(tmp, arg, USER_POSITION_BM_BEGIN);
		while (fgets(tmp, sizeof(tmp), fp) != NULL) {
			tmp[strlen(tmp) - 1] = ' ';
			callback(tmp, arg, USER_POSITION_BM);
		}
		fclose(fp);
		callback(tmp, arg, USER_POSITION_BM_END);
	}
}

void user_position(const struct userec *user, const char *title,
		user_position_callback_t callback, void *arg)
{
	if (user->userlevel & PERM_SPECIAL9) {
		if (user->userlevel & PERM_SYSOPS) {
			callback("站长", arg, USER_POSITION_ADMIN);
		} else if (user->userlevel & PERM_ANNOUNCE) {
			callback("站务", arg, USER_POSITION_ADMIN);
		} else if (user->userlevel & PERM_OCHAT) {
			callback("实习站务", arg, USER_POSITION_ADMIN);
		} else if (user->userlevel & PERM_SPECIAL0) {
			callback("站务委员会秘书", arg, USER_POSITION_ADMIN);
		} else {
			callback("离任站务", arg, USER_POSITION_ADMIN);
		}
	} else {
		if ((user->userlevel & PERM_XEMPT)
				&& (user->userlevel & PERM_LONGLIFE)
				&& (user->userlevel & PERM_LARGEMAIL)) {
			callback("荣誉版主", arg, USER_POSITION_ADMIN);
		}
		if (user->userlevel & PERM_BOARDS)
			show_bm(user->userid, callback, arg);
		if (user->userlevel & PERM_ARBI)
			callback("仲裁组", arg, USER_POSITION_ADMIN);
		if (user->userlevel & PERM_SERV)
			callback("培训组", arg, USER_POSITION_ADMIN);
		if (user->userlevel & PERM_SPECIAL2)
			callback("服务组", arg, USER_POSITION_ADMIN);
		if (user->userlevel & PERM_SPECIAL3)
			callback("美工组", arg, USER_POSITION_ADMIN);
		if (user->userlevel & PERM_TECH)
			callback("技术组", arg, USER_POSITION_ADMIN);
	}

	if (title && *title)
		callback(title, arg, USER_POSITION_CUSTOM);
}

typedef struct {
	char *buf;
	size_t size;
} user_position_string_callback_t;

static void user_position_string_callback(const char *position, void *arg,
		user_position_e type)
{
	user_position_string_callback_t *p = arg;
	char **b = &p->buf;
	size_t *s = &p->size;

	switch (type) {
		case USER_POSITION_BM_BEGIN:
			strappend(b, s, "[\033[1;33m");
			break;
		case USER_POSITION_BM:
			strappend(b, s, position);
			break;
		case USER_POSITION_BM_END:
			strappend(b, s, "\033[32m版版主\033[m]");
			break;
		default: {
			char buf[12];
			snprintf(buf, sizeof(buf), "[\033[1;%dm",
					type == USER_POSITION_ADMIN ? 32 : 33);
			strappend(b, s, buf);
			strappend(b, s, position);
			strappend(b, s, "\033[m]");
		}
	}
}

void user_position_string(const struct userec *user, const char *title,
		char *buf, size_t size)
{
	buf[0] = '\0';
	user_position_string_callback_t arg = {
		.buf = buf,
		.size = size,
	};
	user_position(user, title, user_position_string_callback, &arg);

	if (!*buf) {
		user_position_string_callback(BBSNAME_SHORT "网友", &arg,
				USER_POSITION_ADMIN);
	}
}
