#ifndef FB_SESSION_H
#define FB_SESSION_H

#include <stdint.h>

typedef int64_t session_id_t;
#define PRIdSID  PRId64
#define DBIdSID  "l"

#define db_get_session_id(res, row, col)  db_get_bigint(res, row, col)

enum {
	SESSION_KEY_LEN = 20,

	SESSION_TELNET = 0,
	SESSION_WEB = 1,

	SESSION_PLAIN = 0,
	SESSION_SECURE = 1,

	SESSION_FLAG_WEB = 0x1,
	SESSION_FLAG_SECURE = 0x2,
};

typedef struct {
	session_id_t id;
	user_id_t uid;
	int pid;
	int flag;
} bbs_session_t;

extern session_id_t session_new_id(void);
extern session_id_t session_new(const char *key, session_id_t sid, user_id_t uid,
		const char *ip_addr, bool is_web, bool is_secure);
extern int session_destroy(session_id_t sid);

extern int set_idle_time(session_id_t sid, fb_time_t t);
extern fb_time_t get_idle_time(session_id_t sid);
extern int set_current_board(session_id_t sid, int bid);
extern int get_current_board(session_id_t sid);

#endif // FB_SESSION_H
