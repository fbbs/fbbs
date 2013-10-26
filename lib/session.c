#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "fbbs/mdbi.h"
#include "fbbs/session.h"

enum {
	IDLE_TIME_REFRESH_THRESHOLD = 5,
	ONLINE_FOLLOWS_COUNT_REFRESH_INTERVAL = 15,
	ONLINE_COUNT_REFRESH_INTERVAL = 15,
};

typedef struct {
	session_id_t id;
	user_id_t uid;
	int pid;
	int flag;
	session_status_e status;
	fb_time_t idle;
	bool visible;
} bbs_session_t;

static bbs_session_t session;

session_id_t session_id(void)
{
	return session.id;
}

void session_set_id(session_id_t sid)
{
	session.id = sid;
}

user_id_t session_uid(void)
{
	return session.uid;
}

void session_set_uid(user_id_t uid)
{
	session.uid = uid;
}

int session_pid(void)
{
	return session.pid;
}

void session_set_pid(int pid)
{
	session.pid = pid;
}

fb_time_t session_idle(void)
{
	return session.idle;
}

session_status_e session_status(void)
{
	return session.status;
}

bool session_visible(void)
{
	return session.visible;
}

void session_set_visibility(bool visible)
{
	session.visible = visible;
}

void session_clear(void)
{
	memset(&session, 0, sizeof(session));
	session.visible = true;
}

/** 在线人数 @mdb_string */
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

	mdb_cmd("SET", ONLINE_COUNT_CACHE_KEY" %d", online);
	mdb_cmd("EXPIRE", ONLINE_COUNT_CACHE_KEY" %d",
			ONLINE_COUNT_REFRESH_INTERVAL);
	return online;
}

/** 最大在线人数 @mdb_string */
#define MAX_ONLINE_CACHE_KEY "c:max_online"

void update_peak_online(int online)
{
	mdb_cmd("SET", MAX_ONLINE_CACHE_KEY" %d", online);
}

int get_peak_online(void)
{
	return mdb_integer(0, "GET", MAX_ONLINE_CACHE_KEY);
}

session_id_t session_new_id(void)
{
	db_res_t *res = db_query("SELECT nextval('sessions_id_seq')");
	if (!res)
		return 0;

	session.id = db_get_session_id(res, 0, 0);
	db_clear(res);
	return session.id;
}

session_id_t session_new(const char *key, session_id_t sid, user_id_t uid,
		const char *ip_addr, bool is_web, bool is_secure, bool visible,
		int duration)
{
	int pid = is_web ? 0 : getpid();
	if (!sid)
		sid = session_new_id();

	fb_time_t now = fb_time();
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
		session.id = 0;
		return 0;
	}
}

/** 会话所在版面 @mdb_sorted_set */
#define SESSION_BOARD_KEY "current_board"
/** 会话最后活动时间 @mdb_sorted_set */
#define SESSION_IDLE_KEY "idle"

static void purge_session_cache(session_id_t sid)
{
	mdb_cmd("ZREM", SESSION_BOARD_KEY" %"PRIdSID, sid);
	mdb_cmd("ZREM", SESSION_IDLE_KEY" %"PRIdSID, sid);
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
	return !mdb_cmd("ZADD", SESSION_IDLE_KEY" %"PRIdFBT" %"PRIdSID, t, sid);
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
	return (fb_time_t) mdb_integer(0, "ZSCORE", SESSION_IDLE_KEY" %"PRIdSID,
			sid);
}

int set_current_board(int bid)
{
	return !mdb_cmd("ZADD", SESSION_BOARD_KEY" %d %"PRIdSID, bid,
			session.id);
}

int get_current_board(session_id_t sid)
{
	return (int) mdb_integer(0, "ZSCORE", SESSION_BOARD_KEY" %"PRIdSID, sid);
}

int count_onboard(int bid)
{
	return (int) mdb_integer(0, "ZCOUNT", SESSION_BOARD_KEY" %d %d", bid, bid);
}

/** 会话最新状态 @mdb_hash */
#define SESSION_STATUS_KEY "user_status"

int set_user_status(int status)
{
	session.status = status;
	return !mdb_cmd("HSET", SESSION_STATUS_KEY" %"PRIdSID" %d", session.id,
			status);
}

session_status_e get_user_status(session_id_t sid)
{
	return (int) mdb_integer(0, "HGET", SESSION_STATUS_KEY" %"PRIdSID, sid);
}

bool session_toggle_visibility(void)
{
	db_res_t *res = db_cmd("UPDATE sessions SET visible = %b"
			" WHERE id = %"DBIdSID, !session.visible, session.id);
	db_clear(res);
	if (res)
		session.visible = !session.visible;
	return session.visible;
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
	mdb_cmd("HDEL", WEB_SESSION_HASH_KEY" %"PRIdUID":%s", uid, key);
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
			//% "无所事事"
			return "\xce\xde\xcb\xf9\xca\xc2\xca\xc2";
		case ST_NEW:
			//% "新站友注册"
			return "\xd0\xc2\xd5\xbe\xd3\xd1\xd7\xa2\xb2\xe1";
		case ST_LOGIN:
			//% "进入本站"
			return "\xbd\xf8\xc8\xeb\xb1\xbe\xd5\xbe";
		case ST_DIGEST:
			//% "汲取精华"
			return "\xbc\xb3\xc8\xa1\xbe\xab\xbb\xaa";
		case ST_MMENU:
			//% "游大街"
			return "\xd3\xce\xb4\xf3\xbd\xd6";
		case ST_ADMIN:
			//% "修路铺桥"
			return "\xd0\xde\xc2\xb7\xc6\xcc\xc7\xc5";
		case ST_SELECT:
			//% "选择讨论区"
			return "\xd1\xa1\xd4\xf1\xcc\xd6\xc2\xdb\xc7\xf8";
		case ST_READBRD:
			//% "览遍天下"
			return "\xc0\xc0\xb1\xe9\xcc\xec\xcf\xc2";
		case ST_READNEW:
			//% "览新文章"
			return "\xc0\xc0\xd0\xc2\xce\xc4\xd5\xc2";
		case ST_READING:
			//% "品味文章"
			return "\xc6\xb7\xce\xb6\xce\xc4\xd5\xc2";
		case ST_POSTING:
			//% "文豪挥笔"
			return "\xce\xc4\xba\xc0\xbb\xd3\xb1\xca";
		case ST_MAIL:
			//% "处理信笺"
			return "\xb4\xa6\xc0\xed\xd0\xc5\xbc\xe3";
		case ST_SMAIL:
			//% "寄语信鸽"
			return "\xbc\xc4\xd3\xef\xd0\xc5\xb8\xeb";
		case ST_RMAIL:
			//% "阅览信笺"
			return "\xd4\xc4\xc0\xc0\xd0\xc5\xbc\xe3";
		case ST_TMENU:
			//% "上鹊桥"
			return "\xc9\xcf\xc8\xb5\xc7\xc5";
		case ST_LUSERS:
			//% "环顾四方"
			return "\xbb\xb7\xb9\xcb\xcb\xc4\xb7\xbd";
		case ST_FRIEND:
			//% "夜探好友"
			return "\xd2\xb9\xcc\xbd\xba\xc3\xd3\xd1";
		case ST_MONITOR:
			//% "探视民情"
			return "\xcc\xbd\xca\xd3\xc3\xf1\xc7\xe9";
		case ST_QUERY:
			//% "查询网友"
			return "\xb2\xe9\xd1\xaf\xcd\xf8\xd3\xd1";
		case ST_TALK:
			//% "鹊桥细语"
			return "\xc8\xb5\xc7\xc5\xcf\xb8\xd3\xef";
		case ST_PAGE:
			//% "巴峡猿啼"
			return "\xb0\xcd\xcf\xbf\xd4\xb3\xcc\xe4";
		case ST_CHAT2:
			//% "燕园夜话"
			return "\xd1\xe0\xd4\xb0\xd2\xb9\xbb\xb0";
		case ST_CHAT1:
			//% "燕园夜话"
			return "\xd1\xe0\xd4\xb0\xd2\xb9\xbb\xb0";
		case ST_LAUSERS:
			//% "探视网友"
			return "\xcc\xbd\xca\xd3\xcd\xf8\xd3\xd1";
		case ST_XMENU:
			//% "系统资讯"
			return "\xcf\xb5\xcd\xb3\xd7\xca\xd1\xb6";
		case ST_BBSNET:
#ifdef FDQUAN
			//% "有泉穿梭"
			return "\xd3\xd0\xc8\xaa\xb4\xa9\xcb\xf3";
#else
			//% "饮复旦泉"
			return "\xd2\xfb\xb8\xb4\xb5\xa9\xc8\xaa";
#endif
		case ST_EDITUFILE:
			//% "编辑个人档"
			return "\xb1\xe0\xbc\xad\xb8\xf6\xc8\xcb\xb5\xb5";
		case ST_EDITSFILE:
			//% "动手动脚"
			return "\xb6\xaf\xca\xd6\xb6\xaf\xbd\xc5";
		case ST_SYSINFO:
			//% "检查系统"
			return "\xbc\xec\xb2\xe9\xcf\xb5\xcd\xb3";
		case ST_DICT:
			//% "翻查字典"
			return "\xb7\xad\xb2\xe9\xd7\xd6\xb5\xe4";
		case ST_LOCKSCREEN:
			//% "屏幕锁定"
			return "\xc6\xc1\xc4\xbb\xcb\xf8\xb6\xa8";
		case ST_NOTEPAD:
			//% "留言板"
			return "\xc1\xf4\xd1\xd4\xb0\xe5";
		case ST_GMENU:
			//% "工具箱"
			return "\xb9\xa4\xbe\xdf\xcf\xe4";
		case ST_MSG:
			//% "送讯息"
			return "\xcb\xcd\xd1\xb6\xcf\xa2";
		case ST_USERDEF:
			//% "自订参数"
			return "\xd7\xd4\xb6\xa9\xb2\xce\xca\xfd";
		case ST_EDIT:
			//% "修改文章"
			return "\xd0\xde\xb8\xc4\xce\xc4\xd5\xc2";
		case ST_OFFLINE:
			//% "自杀中.."
			return "\xd7\xd4\xc9\xb1\xd6\xd0..";
		case ST_EDITANN:
			//% "编修精华"
			return "\xb1\xe0\xd0\xde\xbe\xab\xbb\xaa";
		case ST_LOOKMSGS:
			//% "查看讯息"
			return "\xb2\xe9\xbf\xb4\xd1\xb6\xcf\xa2";
		case ST_WFRIEND:
			//% "寻人名册"
			return "\xd1\xb0\xc8\xcb\xc3\xfb\xb2\xe1";
		case ST_WNOTEPAD:
			//% "欲走还留"
			return "\xd3\xfb\xd7\xdf\xbb\xb9\xc1\xf4";
		case ST_BBSPAGER:
			//% "网路传呼"
			return "\xcd\xf8\xc2\xb7\xb4\xab\xba\xf4";
		case ST_M_BLACKJACK:
			//% "★黑甲克★"
			return "\xa1\xef\xba\xda\xbc\xd7\xbf\xcb\xa1\xef";
		case ST_M_XAXB:
			//% "★猜数字★"
			return "\xa1\xef\xb2\xc2\xca\xfd\xd7\xd6\xa1\xef";
		case ST_M_DICE:
			//% "★西八拉★"
			return "\xa1\xef\xce\xf7\xb0\xcb\xc0\xad\xa1\xef";
		case ST_M_GP:
			//% "金扑克梭哈"
			return "\xbd\xf0\xc6\xcb\xbf\xcb\xcb\xf3\xb9\xfe";
		case ST_M_NINE:
			//% "天地九九"
			return "\xcc\xec\xb5\xd8\xbe\xc5\xbe\xc5";
		case ST_WINMINE:
			//% "键盘扫雷"
			return "\xbc\xfc\xc5\xcc\xc9\xa8\xc0\xd7";
		case ST_M_BINGO:
			//% "宾果宾果"
			return "\xb1\xf6\xb9\xfb\xb1\xf6\xb9\xfb";
		case ST_FIVE:
			//% "决战五子棋"
			return "\xbe\xf6\xd5\xbd\xce\xe5\xd7\xd3\xc6\xe5";
		case ST_MARKET:
			//% "交易市场"
			return "\xbd\xbb\xd2\xd7\xca\xd0\xb3\xa1";
		case ST_PAGE_FIVE:
			//% "邀请下棋"
			return "\xd1\xfb\xc7\xeb\xcf\xc2\xc6\xe5";
		case ST_CHICK:
			//% "电子小鸡"
			return "\xb5\xe7\xd7\xd3\xd0\xa1\xbc\xa6";
		case ST_MARY:
			//% "超级玛丽"
			return "\xb3\xac\xbc\xb6\xc2\xea\xc0\xf6";
		case ST_CHICKEN:
			//% "星空战斗鸡"
			return "\xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6";
		case ST_GOODWISH:
			//% "给朋友祝福"
			return "\xb8\xf8\xc5\xf3\xd3\xd1\xd7\xa3\xb8\xa3";
		case ST_GIVEUPBBS:
			//% "戒网中"
			return "\xbd\xe4\xcd\xf8\xd6\xd0";
		case ST_UPLOAD:
			//% "上传文件"
			return "\xc9\xcf\xb4\xab\xce\xc4\xbc\xfe";
		case ST_PROP:
			//% "聚宝盆"
			return "\xbe\xdb\xb1\xa6\xc5\xe8";
		case ST_MY_PROP:
			//% "藏经阁"
			return "\xb2\xd8\xbe\xad\xb8\xf3";
		default:
			//% "去了哪里!?"
			return "\xc8\xa5\xc1\xcb\xc4\xc4\xc0\xef!?";
	}
}
