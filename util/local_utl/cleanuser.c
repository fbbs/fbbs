#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "bbs.h"

static void post_add(FILE *fp, const struct userec *user, fb_time_t now)
{
	fprintf(fp, "\033[1;37m%s \033[m(\033[1;33m%s\033[m) 共上站 "
			"\033[1;32m%d\033[m 次 [\033[1;3%dm%s\033[m]\n",
			user->userid, user->username, user->numlogins,
			(user->gender == 'F') ? 5 : 6,
			horoscope(user->birthmonth, user->birthday));
	fprintf(fp, "上 次 在:[\033[1;32m%s\033[m] 从 [\033[1;32m%s\033[m] "
			"到本站一游。\n", getdatestring(user->lastlogin, DATE_ZH),
			(user->lasthost[0] == '\0' ? "(不详)" : user->lasthost));
	fprintf(fp, "离站时间:[\033[1;32m%s\033[m] ",
			getdatestring(user->lastlogout, DATE_ZH));

	int exp = countexp(user);
	int perf = countperf(user);
#ifdef SHOW_PERF
	fprintf(fp, "表现值:%d(\033[1;33m%s\033[m)\n", perf, cperf(perf));
#else
	fprintf(fp, "表现值:[\033[1;33m%s\033[m]\n", cperf(perf));
#endif

#ifdef ALLOWGAME
	fprintf(fp, "银行存款: [\033[1;32m%d元\033[m] "
			"目前贷款: [\033[1;32m%d元\033[m](\033[1;33m%s\033[m) "
			"经验值：[\033[1;32m%d\033[m](\033[1;33m%s\033[m)。\n",
			user->money, user->bet, cmoney(user->money - user->bet),
			exp, cexpstr(exp));
	fprintf(fp, "文 章 数: [\033[1;32m%d\033[m] "
			"奖章数: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 生命力："
			"[\033[1;32m%d\033[m] 网龄[\033[1;32m%"PRIdFBT"天\033[m]\n\n",
			user->numposts, user->nummedals, cnummedals(user->nummedals),
			compute_user_value(user), (now - user->firstlogin) / 86400);
#else
	fprintf(fp, "文 章 数:[\033[1;32m%d\033[m] 经 验 值:"
#ifdef SHOWEXP
			"%d(\033[1;33m%-10s\033[m)"
#else
			"[\033[1;33m%-10s\033[m]"
#endif
			" 生命力:[\033[1;32m%d\033[m] 网龄[\033[1;32m%d天\033[m]\n\n",
			user->numposts,
#ifdef SHOWEXP
			exp,
#endif
			cexpstr(exp), compute_user_value(user),
			(now - user->firstlogin) / 86400);
#endif
}

int main(int argc, char **argv)
{
	bool pretend = (argc != 2) || (strcasecmp(argv[1], "-f") != 0);

	int fd;
	FILE *log, *data, *post;

	if (!pretend) {
		int fd = open(BBSHOME"/tmp/killuser", O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd < 0)
			return EXIT_FAILURE;
		unlink(BBSHOME"/tmp/killuser");
	
		log = fopen(BBSHOME"/tomb/log", "w+");
		if (!log)
			return EXIT_FAILURE;

		data = fopen(BBSHOME"/tomb/PASSWDS", "w+");
		if (!data)
			return EXIT_FAILURE;

		post = fopen(BBSHOME"/tomb/post", "w+");
		if (!post)
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

			post_add(post, &user, now);
			fwrite(&user, sizeof(user), 1, data);
			fprintf(log, "%s\n", user.userid);
			
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
		}
	}

	if (!pretend) {
		fclose(post);
		fclose(data);
		fclose(log);
		close(fd);
	}
	return EXIT_SUCCESS;
}
