#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

enum {
	MAX_BBSNET_SITES = 54,  ///<
	MAX_SITE_LENGTH = 40,   ///<
	CONNECT_TIMEOUT = 15,   ///<
};

extern char fromhost[];

/**
 *
 */
typedef struct {
	char host[MAX_SITE_LENGTH];  ///<
	char name[MAX_SITE_LENGTH];  ///<
	char ip[IPLEN];              ///<
	int port;                    ///<
} site_t;

static jmp_buf ret_alarm;

/**
 *
 */
static bool can_bbsnet(const char *file, const char *from)
{
	if (streq(from, "127.0.0.1") || streq(from, "localhost"))
		return true;
	FILE *fp = fopen(file, "r");
	if (fp) {
		char buf[256], *ptr, *ch;
		bool allow;
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			ptr = strtok(buf, " \n\t\r");
			if (ptr != NULL && *ptr != '#') {
				allow = (*ptr != '!');
				if (!allow)
					ptr++;
				ch = ptr;
				while (*ch != '\0' && *ch != '*')
					ch++;
				*ch = '\0';
				if (strneq(from, ptr, ch - ptr)) {
					fclose(fp);
					return allow;
				}
			}
		}
		fclose(fp);
	}
	return false;
}

/**
 *
 */
static void exit_bbsnet(int signo)
{
	siglongjmp(ret_alarm, 1);
}

/**
 *
 */
static int bbsnet_log(const site_t *site)
{
	char buf[512];
	time_t now = time(NULL);
	snprintf(buf, sizeof(buf), "%s %s %s (%s)\n", currentuser.userid,
			ctime(&now), site->name, site->ip);
	return file_append("reclog/bbsnet.log", buf);
}

/**
 *
 */
static void do_bbsnet(const site_t *site)
{
	set_user_status(ST_BBSNET);
	screen_clear();
	screen_printf("\033[1;32m连往: %s (%s)\n连不上时请稍候，"
			"%d 秒后将自动退出\n", site->name, site->ip, CONNECT_TIMEOUT);
	screen_flush();

	struct sockaddr_in sock;
	memset(&sock, 0, sizeof(sock));
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = inet_addr(site->ip);
	sock.sin_port = htons(site->port);
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
		return;

	if (!sigsetjmp(ret_alarm, 1)) {
		fb_signal(SIGALRM, exit_bbsnet);
		alarm(CONNECT_TIMEOUT);
		struct hostent *he = gethostbyname(site->ip);
		if (he) {
			memcpy(&sock.sin_addr, he->h_addr_list[0], he->h_length);
		} else {
			if (!inet_aton(site->ip, &sock.sin_addr)) {
				close(fd);
				set_user_status(ST_MMENU);
				return;
			}
		}
		if (connect(fd, (struct sockaddr *)&sock, sizeof(sock)) < 0) {
			close(fd);
			set_user_status(ST_MMENU);
			return;
		}
	} else {
		if (fd > 0)
			close(fd);
		set_user_status(ST_MMENU);
		return;
	}
	fb_signal(SIGALRM, SIG_IGN);

	bbsnet_log(site);
	screen_puts("\033[1;32m已经连接上主机，按'ctrl+]'快速退出。\033[m\n", 0);
	screen_flush();

	struct timeval tv;
	fd_set fds;
	int ret;
	uchar_t buf[2048];
	while (1) {
		tv.tv_sec = IDLE_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		FD_SET(STDIN_FILENO, &fds);

		ret = select(fd + 1, &fds, NULL, NULL, &tv);
		if (ret <= 0)
			break;
		if (FD_ISSET(STDIN_FILENO, &fds)) {
			ret = terminal_read(buf, sizeof(buf));
			if (ret <= 0 || *buf == Ctrl(']'))
				break;
			file_write(fd, buf, ret);
		} else {
			ret = read(fd, buf, sizeof(buf));
			if (ret <= 0)
				break;
			terminal_write(buf, ret);
		}
	}
	close(fd);
	set_user_status(ST_MMENU);
}

/**
 *
 */
static int load_config(const char *file, site_t *sites, int size)
{
	int count = 0;
	FILE *fp = fopen(file, "r");
	char buf[256], *t1, *t2, *t3, *t4;
	site_t *site;
	if (fp) {
		while (count <= size && fgets(buf, sizeof(buf), fp)) {
			t1 = strtok(buf, " \t");
			if (!t1 || *t1 == '#')
				continue;
			t2 = strtok(NULL, " \t\n");
			if (!t2)
				continue;
			t3 = strtok(NULL, " \t\n");
			if (!t3)
				continue;
			t4 = strtok(NULL, " \t\n");
			site = sites + count;
			strlcpy(site->host, t1, sizeof(site->host));
			strlcpy(site->name, t2, sizeof(site->name));
			strlcpy(site->ip, t3, sizeof(site->ip));
			site->port = t4 ? strtol(t4, NULL, 10) : 23;
			count++;
		}
		fclose(fp);
	}
	return count;
}

#ifdef FDQUAN
const static char str[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234";

/**
 *
 */
static void show_site(const site_t *sites, int count, int pos, int cur, int off)
{
	if (pos < count) {
		char buf[MAX_SITE_LENGTH + 22];
		if (pos == cur) {
			snprintf(buf, sizeof(buf), "\033[1;33;40m[%c]\033[42m%s\033[m",
					str[pos], sites[pos].name);
		} else {
			snprintf(buf, sizeof(buf), "\033[1;32;40m %c.\033[m%s",
					str[pos], sites[pos].name);
		}
		screen_replace(pos / 3 + 3, off, buf);
	}
}

static void show_line(const site_t *sites, int count, int line, int cur)
{
	const int col[] = { 3, 28, 53 };
	screen_move_clear(line + 3);
	for (int i = 0; i < 3; ++i) {
		show_site(sites, count, line * 3 + i, cur, col[i]);
	}
}

/**
 *
 */
static void show_sites(const site_t *sites, int count)
{
	screen_clear();
	screen_puts("\033[1;44m穿梭银河\033[K\n\033[m离开[\033[1;32mCtrl-C Ctrl-D\033[m] "
			"选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m,\033[1;32m←\033[m,"
			"\033[1;32m→\033[m]", 0);
	for (int i = 0; i < MAX_BBSNET_SITES / 3; i++) {
		show_line(sites, count, i, -1);
	}
}

/**
 *
 */
static void site_highlight(const site_t *sites, int count, int cur)
{
	show_line(sites, count, cur / 3, cur);
	screen_move_clear(-1);
	screen_printf("\033[1;37;44m%s(%s)\033[K\033[m",
			sites[cur].name, sites[cur].host);
}

/**
 *
 */
static void site_select(const site_t *sites, int count)
{
	int p = 0, old = -1;
	show_sites(sites, count);
	while (1) {
		site_highlight(sites, count, p);
		old = p;
		int ch = igetch();
		const char *ptr;
		switch (ch) {
			case KEY_UP:
				p -= 3;
				if (p < 0)
					p = p % 3 + count - 1;
				break;
			case KEY_DOWN:
				p += 3;
				if (p >= count)
					p %= 3;
				break;
			case KEY_LEFT:
				if (--p < 0)
					p = count - 1;
				break;
			case KEY_RIGHT:
				if (++p >= count)
					p = 0;
				break;
			case '\n':
			case '\r':
				do_bbsnet(sites + p);
				show_sites(sites, count);
				break;
			case Ctrl('C'):
			case Ctrl('D'):
				return;
			default:
				ptr = strchr(str, ch);
				if (ptr)
					p = ptr - str;
				if (p >= count)
					p = old;
				break;
		}
		if (p / 3 != old / 3)
			show_line(sites, count, old / 3, p);
	}
}
#endif

/**
 *
 */
static void bbsnet(const char *config, const char *user)
{
	site_t sites[MAX_BBSNET_SITES];
	int count = load_config(config, sites, MAX_BBSNET_SITES);
	if (count <= 0)
		return;

	signal(SIGALRM, SIG_IGN);
#ifdef FDQUAN
	site_select(sites, count);
#else
	do_bbsnet(sites);
#endif
}

/**
 *
 */
int ent_bnet(void)
{
	if (HAS_PERM(PERM_BLEVELS) || can_bbsnet("etc/bbsnetip", fromhost)) {
		bbsnet("etc/bbsnet.ini", currentuser.userid);
	} else {
		screen_clear();
		screen_puts("抱歉，由于您是校内用户，您无法使用本穿梭功能...\n"
				"请直接连往复旦泉站：telnet 10.8.225.9", 0);
		pressanykey();
	}
	return 0;
}

/**
 *
 */
int ent_bnet2(void)
{
	if (HAS_PERM(PERM_BLEVELS) || can_bbsnet("etc/bbsnetip2", fromhost)) {
		bbsnet("etc/bbsnet2.ini", currentuser.userid);
	} else {
		screen_clear();
		screen_puts("抱歉，您所处的位置无法使用本穿梭功能...", 0);
		pressanykey();
	}
	return 0;
}
