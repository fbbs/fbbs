#include <sys/types.h>
#include <unistd.h>

#include "fbbs/mdbi.h"
#include "fbbs/session.h"

enum {
	IDLE_TIME_REFRESH_THRESHOLD = 5,
	ONLINE_FOLLOWS_COUNT_REFRESH_INTERVAL = 15,
	ONLINE_COUNT_REFRESH_INTERVAL = 15,
};

bbs_session_t session;

#define ONLINE_COUNT_CACHE_KEY  "c:online"

int count_online(void)
{
	int cached = mdb_integer(-1, "GET", ONLINE_COUNT_CACHE_KEY);
	if (cached >= 0)
		return cached;

	int online = 0;
	db_res_t *res = db_query("SELECT count(*) FROM sessions WHERE active");
	if (res && db_res_rows(res) > 0)
		online = db_get_bigint(res, 0, 0);
	db_clear(res);

	mdb_res_t *r = mdb_cmd("SET", ONLINE_COUNT_CACHE_KEY" %d", online);
	mdb_clear(r);
	r = mdb_cmd("EXPIRE", ONLINE_COUNT_CACHE_KEY" %d",
			ONLINE_COUNT_REFRESH_INTERVAL);
	mdb_clear(r);

	return online;
}

void update_peak_online(int online)
{
	mdb_res_t *r = mdb_cmd("SET", "c:max_online %d", online);
	mdb_clear(r);
}

int get_peak_online(void)
{
	return mdb_integer(0, "GET", "c:max_online");
}

session_id_t session_new_id(void)
{
	db_res_t *res = db_query("SELECT nextval('sessions_id_seq')");
	if (!res)
		return 0;

	session_id_t sid = db_get_session_id(res, 0, 0);
	db_clear(res);
	return sid;
}

session_id_t session_new(const char *key, session_id_t sid, user_id_t uid,
		const char *ip_addr, bool is_web, bool is_secure, bool visible,
		int duration)
{
	int pid = is_web ? 0 : getpid();
	if (!sid)
		sid = session_new_id();

	fb_time_t now = time(NULL);
	fb_time_t expire = now + duration;

	db_res_t *res = db_cmd("INSERT INTO sessions (id, session_key, user_id,"
			" pid, ip_addr, web, secure, stamp, expire, visible) VALUES"
			" (%"DBIdSID", %s, %"DBIdUID", %d, %s, %b, %b, %t, %t, %b)", sid,
			key, uid, pid, ip_addr, is_web, is_secure, now, expire, visible);
	if (res) {
		db_clear(res);
		session.id = sid;
		set_idle_time(sid, time(NULL));
		return sid;
	} else {
		return 0;
	}
}

static void purge_session_cache(session_id_t sid)
{
	mdb_res_t *r = mdb_cmd("ZREM", "current_board %"PRIdSID, sid);
	mdb_clear(r);

	r = mdb_cmd("ZREM", "idle %"PRIdSID, sid);
	mdb_clear(r);
}

int session_destroy(session_id_t sid)
{
	db_res_t *res = db_cmd("DELETE FROM sessions WHERE id = %"DBIdSID, sid);
	db_clear(res);

	purge_session_cache(sid);

	return !res;
}

int session_inactivate(session_id_t sid)
{
	db_res_t *res = db_cmd("UPDATE sessions SET active = FALSE"
			" WHERE id=%"DBIdSID, sid);
	db_clear(res);

	purge_session_cache(sid);

	return !res;
}

int set_idle_time(session_id_t sid, fb_time_t t)
{
	mdb_res_t *res = mdb_cmd("ZADD", "idle %"PRIdFBT" %"PRIdSID, t, sid);
	mdb_clear(res);
	return !res;
}

void cached_set_idle_time(void)
{
	time_t now = time(NULL);
	if (now > session.idle + IDLE_TIME_REFRESH_THRESHOLD)
		set_idle_time(session.id, now);
	session.idle = now;
}

fb_time_t get_idle_time(session_id_t sid)
{
	return (fb_time_t) mdb_integer(0, "ZSCORE", "idle %"PRIdSID, sid);
}

int set_current_board(int bid)
{
	mdb_res_t *res = mdb_cmd("ZADD", "current_board %d %"PRIdSID, bid, session.id);
	mdb_clear(res);
	return !res;
}

int get_current_board(session_id_t sid)
{
	return (int) mdb_integer(0, "ZSCORE", "current_board %"PRIdSID, sid);
}

int set_user_status(int status)
{
	session.status = status;
	mdb_res_t *res = mdb_cmd("HSET", "user_status %"PRIdSID" %d",
			session.id, status);
	mdb_clear(res);
	return !res;
}

int get_user_status(session_id_t sid)
{
	return (int) mdb_integer(0, "HGET", "user_status %"PRIdSID, sid);
}

int set_visibility(bool visible)
{
	db_res_t *res = db_cmd("UPDATE sessions SET visible = %b"
			" WHERE id = %"DBIdSID, visible, session.id);
	db_clear(res);
	return !res;
}

db_res_t *get_sessions_of_followings(void)
{
	return db_query("SELECT " ACTIVE_SESSION_FIELDS ", f.notes"
			" FROM sessions s JOIN follows f ON s.user_id = f.user_id"
			" JOIN users u ON s.user_id = u.id"
			" WHERE s.active AND f.follower = %"DBIdUID, session.uid);
}

db_res_t *get_active_sessions(void)
{
	return db_query(ACTIVE_SESSION_QUERY);
}

basic_session_info_t *get_sessions(user_id_t uid)
{
	return db_query("SELECT " BASIC_SESSION_INFO_FIELDS " FROM sessions s"
			" WHERE active AND user_id = %"DBIdUID, uid);
}

basic_session_info_t *get_my_sessions(void)
{
	return get_sessions(session.uid);
}

static basic_session_info_t *basic_sessions_of_follows(void)
{
	return db_query("SELECT "BASIC_SESSION_INFO_FIELDS
			" FROM sessions s JOIN follows f ON s.user_id = f.user_id"
			" WHERE s.active AND f.follower = %"DBIdUID, session.uid);
}

int online_follows_count(bool visible_only)
{
	static time_t uptime = 0;
	static int count = 0;

	time_t now = time(NULL);
	if (now <= uptime + ONLINE_FOLLOWS_COUNT_REFRESH_INTERVAL)
		return count;
	uptime = now;

	basic_session_info_t *s = basic_sessions_of_follows();
	if (s) {
		if (!visible_only) {
			count = basic_session_info_count(s);
		} else {
			count = 0;
			for (int i = 0; i < basic_session_info_count(s); ++i) {
				if (basic_session_info_visible(s, i))
					++count;
			}
		}
	} else {
		count = 0;
	}
	basic_session_info_clear(s);
	return count;
}

void remove_web_session_cache(user_id_t uid, const char *key)
{
	mdb_res_t *r = mdb_cmd("HDEL", WEB_SESSION_HASH_KEY" %"PRIdUID":%s",
			uid, key);
	mdb_clear(r);
}

/**
 * Get descriptions of user status.
 * @param status user status.
 * @return a string describing user status.
 */
const char *status_descr(int status)
{
	switch (status) {
		case ST_IDLE:
			return "无所事事";
		case ST_NEW:
			return "新站友注册";
		case ST_LOGIN:
			return "进入本站";
		case ST_DIGEST:
			return "汲取精华";
		case ST_MMENU:
			return "游大街";
		case ST_ADMIN:
			return "修路铺桥";
		case ST_SELECT:
			return "选择讨论区";
		case ST_READBRD:
			return "览遍天下";
		case ST_READNEW:
			return "览新文章";
		case ST_READING:
			return "品味文章";
		case ST_POSTING:
			return "文豪挥笔";
		case ST_MAIL:
			return "处理信笺";
		case ST_SMAIL:
			return "寄语信鸽";
		case ST_RMAIL:
			return "阅览信笺";
		case ST_TMENU:
			return "上鹊桥";
		case ST_LUSERS:
			return "环顾四方";
		case ST_FRIEND:
			return "夜探好友";
		case ST_MONITOR:
			return "探视民情";
		case ST_QUERY:
			return "查询网友";
		case ST_TALK:
			return "鹊桥细语";
		case ST_PAGE:
			return "巴峡猿啼";
		case ST_CHAT2:
			return "燕园夜话";
		case ST_CHAT1:
			return "燕园夜话";
		case ST_LAUSERS:
			return "探视网友";
		case ST_XMENU:
			return "系统资讯";
		case ST_BBSNET:
#ifdef FDQUAN
			return "有泉穿梭";
#else
			return "饮复旦泉";
#endif
		case ST_EDITUFILE:
			return "编辑个人档";
		case ST_EDITSFILE:
			return "动手动脚";
		case ST_SYSINFO:
			return "检查系统";
		case ST_DICT:
			return "翻查字典";
		case ST_LOCKSCREEN:
			return "屏幕锁定";
		case ST_NOTEPAD:
			return "留言板";
		case ST_GMENU:
			return "工具箱";
		case ST_MSG:
			return "送讯息";
		case ST_USERDEF:
			return "自订参数";
		case ST_EDIT:
			return "修改文章";
		case ST_OFFLINE:
			return "自杀中..";
		case ST_EDITANN:
			return "编修精华";
		case ST_LOOKMSGS:
			return "查看讯息";
		case ST_WFRIEND:
			return "寻人名册";
		case ST_WNOTEPAD:
			return "欲走还留";
		case ST_BBSPAGER:
			return "网路传呼";
		case ST_M_BLACKJACK:
			return "★黑甲克★";
		case ST_M_XAXB:
			return "★猜数字★";
		case ST_M_DICE:
			return "★西八拉★";
		case ST_M_GP:
			return "金扑克梭哈";
		case ST_M_NINE:
			return "天地九九";
		case ST_WINMINE:
			return "键盘扫雷";
		case ST_M_BINGO:
			return "宾果宾果";
		case ST_FIVE:
			return "决战五子棋";
		case ST_MARKET:
			return "交易市场";
		case ST_PAGE_FIVE:
			return "邀请下棋";
		case ST_CHICK:
			return "电子小鸡";
		case ST_MARY:
			return "超级玛丽";
		case ST_CHICKEN:
			return "星空战斗鸡";
		case ST_GOODWISH:
			return "给朋友祝福";
		case ST_GIVEUPBBS:
			return "戒网中";
		case ST_UPLOAD:
			return "上传文件";
		case ST_PROP:
			return "聚宝盆";
		case ST_MY_PROP:
			return "藏经阁";
		default:
			return "去了哪里!?";
	}
}
