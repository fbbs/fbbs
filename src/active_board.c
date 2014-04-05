#include "bbs.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

#define ACTIVE_BOARD_KEY  "active_board"
#define ACTIVE_BOARD_UPTIME_KEY  "active_board_update"
#define ACTIVE_BOARD_NAME  "Notepad"

enum {
	ACTIVE_BOARD_INIT_INTERVAL = 6 * 3600,
	ACTIVE_BOARD_BUFSIZE = 512,
	ACTIVE_BOARD_LINES = 7,
};

static bool active_board_add(const char *content)
{
	char buffer[ACTIVE_BOARD_BUFSIZE * ACTIVE_BOARD_LINES];

	const char *ptr = content, *end = content + strlen(content);
	for (int i = 0; i < 4; ++i) {
		ptr = get_line_end(ptr, end);
	}

	bool finished = false;
	char *buf = buffer;
	size_t size = sizeof(buffer);
	for (int i = 0; i < ACTIVE_BOARD_LINES; ++i) {
		char *lend = (char *) get_line_end(ptr, end);
		if (strneq2(ptr, "--\n"))
			finished = true;
		if (!finished) {
			char line[ACTIVE_BOARD_BUFSIZE];
			convert(env_u2g, ptr, lend - ptr, line, sizeof(line), NULL, NULL);
			strappend(&buf, &size, line);
		} else {
			strappend(&buf, &size, "\n");
		}
		strappend(&buf, &size, "\033[K");
		ptr = lend;
	}

	return mdb_cmd_safe("SADD", "%s %s", ACTIVE_BOARD_KEY, buffer);
}

static record_callback_e active_board_init_callback(void *ptr, void *args,
		int offset)
{
	const post_record_t *pr = ptr;

	if (pr->flag & POST_FLAG_DIGEST) {
		char *content = post_content_get(pr->id, false);
		bool success = content ? active_board_add(content) : false;
		free(content);
		if (success)
			return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static void active_board_clear(void)
{
	mdb_cmd("DEL", ACTIVE_BOARD_KEY);
}

void active_board_init(bool force)
{
	fb_time_t now = fb_time();
	if (!force) {
		fb_time_t last = mdb_integer(0, "GET", ACTIVE_BOARD_UPTIME_KEY);
		if (now - last < ACTIVE_BOARD_INIT_INTERVAL)
			return;
	}
	mdb_cmd("SET", ACTIVE_BOARD_UPTIME_KEY" %"PRIdFBT, now);

	board_t board;
	if (get_board(ACTIVE_BOARD_NAME, &board) <= 0)
		return;

	record_t record;
	if (post_record_open(board.id, &record) < 0)
		return;

	active_board_clear();
	record_foreach(&record, NULL, 0, active_board_init_callback, NULL);

	record_close(&record);
}

void active_board_show(void)
{
	if (!DEFINE(DEF_ACBOARD))
		return;

	mdb_res_t *res = mdb_res("SRANDMEMBER", ACTIVE_BOARD_KEY);
	size_t size;
	const char *str = mdb_string_and_size(res, &size);

	if (str) {
		const char *end = str + size;
		for (int i = 0; i < ACTIVE_BOARD_LINES && str < end; ++i) {
			char line[ACTIVE_BOARD_BUFSIZE];
			const char *lend = get_line_end(str, end);
			if (lend && lend - str < sizeof(line) - 1) {
				memcpy(line, str, lend - str);
				line[lend - str] = '\0';
				move(i + 2, 0);
				showstuff(line);
			}
			str = lend;
		}
	}
	mdb_clear(res);
}
