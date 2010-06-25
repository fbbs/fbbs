#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "fbbs/site.h"
#include "fbbs/socket.h"
#include "fbbs/string.h"
#include "fbbs/util.h"

#define CACHE_SOCKET BBSHOME"/tmp/cache-socket"

enum {
	QLEN = 20,
	BUFLEN = 1024,
};

/**
 * Exit.
 * @param notused Not used.
 */
static void close_daemon(int notused)
{
	exit(0);
}

static int process(const char *buf, int bytes,
		struct sockaddr *addr, socklen_t *len)
{
	return 0;
}

int main(void)
{
	start_daemon();

	fb_signal(SIGTERM, close_daemon);

	setgid((gid_t)BBSGID);
	setuid((uid_t)BBSUID);
	chdir(BBSHOME);
	umask(S_IWGRP | S_IWOTH);

	int fd = unix_dgram_bind(CACHE_SOCKET, QLEN);
	if (fd < 0)
		return EXIT_FAILURE;
	
	char buf[BUFLEN];
	struct sockaddr addr;
	socklen_t addrlen;
	while (1) {
		int bytes = recvfrom(fd, buf, sizeof(buf), 0, &addr, &addrlen);
		if (bytes > 0) {
			process(buf, bytes, &addr, &addrlen);
		}
	}
	return EXIT_SUCCESS;
}
