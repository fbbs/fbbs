#ifndef FB_SESSION_H
#define FB_SESSION_H

#include <stdint.h>
#include "fbbs/time.h"
#include "fbbs/user.h"

typedef int64_t session_id_t;
#define PRIdSID  PRId64
#define DBIdSID  "l"

#define db_get_session_id(res, row, col)  db_get_bigint(res, row, col)

#define ACTIVE_SESSION_FIELDS \
	"s.id, s.user_id, u.name, s.visible, s.ip_addr, s.web"

#define ACTIVE_SESSION_QUERY \
	"SELECT " ACTIVE_SESSION_FIELDS \
	" FROM sessions s JOIN users u ON s.user_id = u.id"

enum {
	SESSION_KEY_LEN = 40,

	SESSION_TELNET = 0,
	SESSION_WEB = 1,

	SESSION_PLAIN = 0,
	SESSION_SECURE = 1,

	SESSION_FLAG_WEB = 0x1,
	SESSION_FLAG_SECURE = 0x2,
	SESSION_FLAG_INVISIBLE = 0x4,
};

enum {
	ST_IDLE = 0,
	ST_TALK = 1,
	ST_NEW  = 2,
	ST_READNEW = 3,
	ST_POSTING = 4,
	ST_MAIL = 5,
	ST_LAUSERS  = 6,
	ST_LUSERS = 7,
	ST_SMAIL = 8,
	ST_RMAIL = 9,
	ST_MMENU = 10,
	ST_TMENU = 11,
	ST_XMENU = 12,
	ST_READING = 13,
	ST_PAGE = 14,
	ST_ADMIN = 15,
	ST_READBRD = 16,
	ST_SELECT = 17,
	ST_LOGIN = 18,
	ST_MONITOR = 19,
	ST_CHAT1 = 20,
	ST_CHAT2 = 21,
	ST_EDITUFILE = 26,
	ST_EDITSFILE = 27,
	ST_QUERY = 28,
	ST_CNTBRDS = 29,
	ST_VISIT = 31,
	ST_BBSNET = 33,
	ST_DIGEST = 34,
	ST_FRIEND = 35,
	ST_YANKING = 36,
	ST_GMENU = 38,
	ST_DICT = 39,
	ST_LOCKSCREEN = 40,
	ST_NOTEPAD = 41,
	ST_MSG = 42,
	ST_USERDEF = 43,
	ST_EDIT = 44,
	ST_OFFLINE = 45,
	ST_EDITANN = 46,
	ST_LOOKMSGS = 49,
	ST_WFRIEND = 50,
	ST_SYSINFO	= 51,
	ST_WNOTEPAD = 54,
	ST_BBSPAGER = 55,
	ST_M_BLACKJACK	= 56,
	ST_M_XAXB = 57,
	ST_M_DICE = 58,
	ST_M_GP = 59,
	ST_M_NINE = 60,
	ST_M_BINGO = 61,
	ST_WINMINE	= 62,
	ST_FIVE = 63,
	ST_PAGE_FIVE = 64,
	ST_MARKET = 65,
	ST_CHICK = 66,
	ST_MARY = 67,
	ST_CHICKEN = 68,
	ST_GOODWISH = 69,
	ST_GIVEUPBBS = 70,
	ST_UPLOAD = 71,
	ST_PROP,
	ST_MY_PROP,
	ST_WWW = 0x40000000,
};

typedef struct {
	session_id_t id;
	user_id_t uid;
	int pid;
	int flag;
	int status;
	fb_time_t idle;
	bool visible;
} bbs_session_t;

extern session_id_t session_new_id(void);
extern session_id_t session_new(const char *key, session_id_t sid, user_id_t uid,
		const char *ip_addr, bool is_web, bool is_secure, int duration);
extern int session_destroy(session_id_t sid);
extern int session_inactivate(session_id_t sid);

extern int set_idle_time(session_id_t sid, fb_time_t t);
extern void cached_set_idle_time(void);
extern fb_time_t get_idle_time(session_id_t sid);
extern int set_current_board(int bid);
extern int get_current_board(session_id_t sid);
extern int set_user_status(int status);
extern int get_user_status(session_id_t sid);
extern int set_visibility(bool visible);

extern db_res_t *get_sessions_of_followings(void);
extern db_res_t *get_active_sessions(void);

#define BASIC_SESSION_INFO_FIELDS  "s.id, s.pid, s.visible, s.web"

typedef db_res_t basic_session_info_t;
extern basic_session_info_t *get_sessions(user_id_t uid);
extern basic_session_info_t *get_my_sessions(void);
#define basic_session_info_count(r)  db_res_rows(r)
#define basic_session_info_sid(r, i)  db_get_session_id(r, i, 0)
#define basic_session_info_pid(r, i)  db_get_integer(r, i, 1)
#define basic_session_info_visible(r, i)  db_get_bool(r, i, 2)
#define basic_session_info_web(r, i)  db_get_session_id(r, i, 3)
#define basic_session_info_clear(r)  db_clear(r)

extern int online_count(void);
extern int get_peak_online(void);
extern void update_peak_online(int online);
extern int online_follows_count(bool visible_only);

extern const char *get_status_color(int status, bool visible, bool web);

#endif // FB_SESSION_H
