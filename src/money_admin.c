#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/dbi.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

typedef int (*callback_t)(const char *, const char *, money_t);

enum {
	NOTE_CCHARS = 30,
};

static int parse(const char *file, callback_t cb)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
		return -1;

	GBK_UTF8_BUFFER(notes, NOTE_CCHARS);
	gbk_notes[0] = '\0';

	int line = 0;
	char buf[256];
	while (fgets(buf, sizeof(buf), fp)) {
		++line;

		if (buf[0] == '#' || buf[0] == '\n')
			continue;

		if (buf[0] == '=') {
			strlcpy(gbk_notes, buf + 1, sizeof(gbk_notes));
			convert_g2u(gbk_notes, utf8_notes);
		} else {
			char name[IDLEN + 1];
			char *ptr = strtok(buf, " \t");
			strlcpy(name, ptr, sizeof(name));

			ptr = strtok(NULL, " \t");
			if (!ptr) {
				fclose(fp);
				return -line;
			}
			money_t value = strtol(ptr, NULL, 10);

			if (cb) {
				if ((*cb)(utf8_notes, name, value * 100) < 0) {
					fclose(fp);
					return -line;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

static bool check_for_errors(const char *file)
{
	return (parse(file, NULL) < 0);
}

static int callback(const char *note, const char *name, money_t delta)
{
	user_id_t uid = 0;
	db_res_t *res = db_query("SELECT id FROM alive_users"
			" WHERE lower(name) = lower(%s)", name);
	if (db_res_rows(res) == 1)
		uid = db_get_user_id(res, 0, 0);
	db_clear(res);

	if (!uid)
		return -1;

	res = db_cmd("UPDATE payment SET awards = awards + %"DBIdMONEY
			" WHERE user_id = %"DBIdUID, delta, uid);
	if (!res)
		return -1;
	db_clear(res);

	res = db_cmd("INSERT INTO audit.money (user_id, delta, stamp, reason)"
			" VALUES (%"DBIdUID", %"DBIdMONEY","
			" current_timestamp, %s || ' ' || %s)",
			uid, delta, note, currentuser.userid);
	if (!res)
		return -1;
	db_clear(res);

	return 0;
}

static int submit_changes(const char *file)
{
	if (db_begin_trans(env.d) != 0)
		return -1;

	int ret = parse(file, callback);
	db_end_trans(env.d);

	return ret;
}

int grant_money(void)
{
	clear();

	char file[HOMELEN];
	snprintf(file, sizeof(file), "tmp/grant.%s.%d",
			currentuser.userid, getpid());
	f_cp("etc/grant_tmpl", file, O_CREAT);

	bool error = true;
	while (error) {
		int aborted = vedit(file, NA, YEA);
		if (aborted == -1) {
			unlink(file);
			return 0;
		}

		error = check_for_errors(file);
	}

	move(t_lines - 2, 0);
	if (submit_changes(file) == 0) {
		Postfile(file, "Bank_In", "光华币发放记录", 2);
		prints("操作成功!");
	} else {
		prints("操作失败..");
	}
	pressanykey();
	unlink(file);
	return 0;
}
