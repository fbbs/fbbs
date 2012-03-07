#ifndef FB_SESSION_H
#define FB_SESSION_H

#include <stdint.h>

typedef int64_t session_id_t;
#define PRIdSID  "l"

#define db_get_session_id(res, row, col)  db_get_bigint(res, row, col)

enum {
	NO_SESSION_KEY = 0,
	WITH_SESSION_KEY = 1,
	SESSION_KEY_LEN = 20,
};

extern session_id_t session_new_id(void);
extern char *session_generate_key(char *buf, size_t size);
extern session_id_t session_new(const char *key, session_id_t sid, user_id_t uid,
		const char *ip_addr);

#endif // FB_SESSION_H
