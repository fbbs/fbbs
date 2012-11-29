#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "bbs.h"
#include "fbbs/dbi.h"
#include "fbbs/helper.h"
#include "fbbs/uinfo.h"

static void post_add(FILE *fp, const struct userec *user, fb_time_t now)
{
	//% fprintf(fp, "\033[1;37m%s \033[m(\033[1;33m%s\033[m) 共上站 "
	fprintf(fp, "\033[1;37m%s \033[m(\033[1;33m%s\033[m) \xb9\xb2\xc9\xcf\xd5\xbe "
			//% "\033[1;32m%d\033[m 次 [\033[1;3%dm%s\033[m]\n",
			"\033[1;32m%d\033[m \xb4\xce [\033[1;3%dm%s\033[m]\n",
			user->userid, user->username, user->numlogins,
			(user->gender == 'F') ? 5 : 6,
			horoscope(user->birthmonth, user->birthday));
	//% fprintf(fp, "上 次 在:[\033[1;32m%s\033[m] 从 [\033[1;32m%s\033[m] "
	fprintf(fp, "\xc9\xcf \xb4\xce \xd4\xda:[\033[1;32m%s\033[m] \xb4\xd3 [\033[1;32m%s\033[m] "
			//% "到本站一游。\n", getdatestring(user->lastlogin, DATE_ZH),
			"\xb5\xbd\xb1\xbe\xd5\xbe\xd2\xbb\xd3\xce\xa1\xa3\n", getdatestring(user->lastlogin, DATE_ZH),
			//% (user->lasthost[0] == '\0' ? "(不详)" : user->lasthost));
			(user->lasthost[0] == '\0' ? "(\xb2\xbb\xcf\xea)" : user->lasthost));
	//% fprintf(fp, "离站时间:[\033[1;32m%s\033[m] ",
	fprintf(fp, "\xc0\xeb\xd5\xbe\xca\xb1\xbc\xe4:[\033[1;32m%s\033[m] ",
			getdatestring(user->lastlogout, DATE_ZH));

	int exp = countexp(user);
	int perf = countperf(user);
#ifdef SHOW_PERF
	//% fprintf(fp, "表现值:%d(\033[1;33m%s\033[m)\n", perf, cperf(perf));
	fprintf(fp, "\xb1\xed\xcf\xd6\xd6\xb5:%d(\033[1;33m%s\033[m)\n", perf, cperf(perf));
#else
	//% fprintf(fp, "表现值:[\033[1;33m%s\033[m]\n", cperf(perf));
	fprintf(fp, "\xb1\xed\xcf\xd6\xd6\xb5:[\033[1;33m%s\033[m]\n", cperf(perf));
#endif

#ifdef ALLOWGAME
	//% fprintf(fp, "银行存款: [\033[1;32m%d元\033[m] "
	fprintf(fp, "\xd2\xf8\xd0\xd0\xb4\xe6\xbf\xee: [\033[1;32m%d\xd4\xaa\033[m] "
			//% "目前贷款: [\033[1;32m%d元\033[m](\033[1;33m%s\033[m) "
			"\xc4\xbf\xc7\xb0\xb4\xfb\xbf\xee: [\033[1;32m%d\xd4\xaa\033[m](\033[1;33m%s\033[m) "
			//% "经验值：[\033[1;32m%d\033[m](\033[1;33m%s\033[m)。\n",
			"\xbe\xad\xd1\xe9\xd6\xb5\xa3\xba[\033[1;32m%d\033[m](\033[1;33m%s\033[m)\xa1\xa3\n",
			user->money, user->bet, cmoney(user->money - user->bet),
			exp, cexpstr(exp));
	//% fprintf(fp, "文 章 数: [\033[1;32m%d\033[m] "
	fprintf(fp, "\xce\xc4 \xd5\xc2 \xca\xfd: [\033[1;32m%d\033[m] "
			//% "奖章数: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 生命力："
			"\xbd\xb1\xd5\xc2\xca\xfd: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) \xc9\xfa\xc3\xfc\xc1\xa6\xa3\xba"
			//% "[\033[1;32m%d\033[m] 网龄[\033[1;32m%"PRIdFBT"天\033[m]\n\n",
			"[\033[1;32m%d\033[m] \xcd\xf8\xc1\xe4[\033[1;32m%"PRIdFBT"\xcc\xec\033[m]\n\n",
			user->numposts, user->nummedals, cnummedals(user->nummedals),
			compute_user_value(user), (now - user->firstlogin) / 86400);
#else
	//% fprintf(fp, "文 章 数:[\033[1;32m%d\033[m] 经 验 值:"
	fprintf(fp, "\xce\xc4 \xd5\xc2 \xca\xfd:[\033[1;32m%d\033[m] \xbe\xad \xd1\xe9 \xd6\xb5:"
#ifdef SHOWEXP
			"%d(\033[1;33m%-10s\033[m)"
#else
			"[\033[1;33m%-10s\033[m]"
#endif
			//% " 生命力:[\033[1;32m%d\033[m] 网龄[\033[1;32m%"PRIdFBT"天\033[m]\n\n",
			" \xc9\xfa\xc3\xfc\xc1\xa6:[\033[1;32m%d\033[m] \xcd\xf8\xc1\xe4[\033[1;32m%"PRIdFBT"\xcc\xec\033[m]\n\n",
			user->numposts,
#ifdef SHOWEXP
			exp,
#endif
			cexpstr(exp), compute_user_value(user),
			(now - user->firstlogin) / 86400);
#endif
}

typedef struct {
	int fd;
	FILE *log;
	FILE *data;
	FILE *post;
	int lock;
} _my_data_t;
static _my_data_t _env;

static void cleanup(void)
{
	ucache_unlock(_env.lock);
	fclose(_env.post);
	fclose(_env.data);
	fclose(_env.log);
	close(_env.fd);
}

static int init_env(_my_data_t *e)
{
	atexit(cleanup);

	memset(e, 0, sizeof(*e));
	
	e->fd = open("tmp/killuser", O_RDWR | O_CREAT | O_EXCL, 0600);
	if (e->fd < 0)
		return -1;
	
	unlink("tmp/killuser");
	
	e->log = fopen("tomb/log", "w+");
	e->data = fopen("tomb/PASSWDS", "w+");
	e->post = fopen("tomb/post", "w+");
	if (!e->log || !e->data || !e->post)
		return -1;

	e->lock = ucache_lock();
	if (e->lock < 0)
		return -1;

	initialize_environment(INIT_DB);
	return 0;
}

int main(int argc, char **argv)
{
	bool pretend = (argc != 2) || (strcasecmp(argv[1], "-f") != 0);

	if (chdir(BBSHOME) < 0)
		return EXIT_FAILURE;

	if (!pretend) {
		if (init_env(&_env) != 0)
			return EXIT_FAILURE;
		log_usies("CLEAN", "dated users.", NULL);
	}

	fb_time_t now = time(NULL);

	struct userec user, zero;
	memset(&zero, 0, sizeof(zero));
	char file[HOMELEN], buf[HOMELEN];

	for (int i = 0; i < MAXUSERS; ++i) {
		getuserbyuid(&user, i + 1);

		int val = compute_user_value(&user);

		if (user.userid[0] != '\0' && val < 0) {
			user.userid[sizeof(user.userid) - 1] = '\0';

			if (pretend) {
				puts(user.userid);
				continue;
			}

			post_add(_env.post, &user, now);
			fwrite(&user, sizeof(user), 1, _env.data);
			fprintf(_env.log, "%s\n", user.userid);
			
			snprintf(file, sizeof(file), "mail/%c/%s",
					toupper(user.userid[0]), user.userid);
			snprintf(buf, sizeof(buf), "%s~", file);
			rename(file, buf);

			snprintf(file, sizeof(file), "home/%c/%s",
					toupper(user.userid[0]), user.userid);
			snprintf(buf, sizeof(buf), "%s~", file);
			rename(file, buf);

			substitut_record(PASSFILE, &zero, sizeof(zero), i + 1);
			del_uidshm(i + 1, user.userid);

			// for now, we just delete them one by one.
			db_res_t *res = db_cmd("UPDATE users SET alive = FALSE"
					" WHERE lower(name) = lower(%s) AND alive", user.userid);
			db_clear(res);
			remove_user_id_cache(user.userid);
		}
	}

	return EXIT_SUCCESS;
}
