#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fbbs/bbs.h"
#include "fbbs/util.h"
#include "fbbs/site.h"

/**
 * Wait for an child to terminate.
 * @param notused Not used.
 */
static void reap_child(int notused)
{
	int state, pid;
	while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0)
		;
}

/**
 * Exit.
 * @param notused Not used.
 */
static void close_daemon(int notused)
{
	exit(0);
}

/**
 *
 */
static int bind_port(const char *port, struct pollfd *fds, int nfds)
{
	struct addrinfo *ai;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	int e = getaddrinfo(NULL, port, &hints, &ai);
	if (e)
		return -1;

	int n = 0;
	for (struct addrinfo *aip = ai; n < nfds && aip; aip = aip->ai_next) {
		int fd = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
		if (fd < 0) {
			freeaddrinfo(ai);
			return n;
		}

		int on = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (aip->ai_family == AF_INET6)
			setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));

		if (bind(fd, aip->ai_addr, aip->ai_addrlen) != 0
				|| listen(fd, SOMAXCONN) != 0) {
			close(fd);
		}

		fds[n].fd = fd;
		fds[n].events = POLLIN;
		++n;
	}

	freeaddrinfo(ai);
	return n;
}

static int accept_connection(int fd, int nfds)
{
	struct sockaddr_storage sock;
	socklen_t socklen = sizeof(sock);
	int conn = accept(fd, (struct sockaddr *)&sock, &socklen);
	if (conn < 0)
		return -1;

	int pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) {
		while (--nfds >= 0)
			close(nfds);
		dup2(conn, STDIN_FILENO);
		close(conn);
		dup2(STDIN_FILENO, STDOUT_FILENO);

		char host[IP_LEN];
		getnameinfo((struct sockaddr *)&sock, socklen, host, sizeof(host),
				NULL, 0, NI_NUMERICHOST);

	} else {
		close(conn);
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		printf("Usage: %s [port]\n", argv[0]);
		return EXIT_FAILURE;
	}

	start_daemon();

	fb_signal(SIGCHLD, reap_child);
	fb_signal(SIGTERM, close_daemon);

	struct pollfd fds[2];
	int nfds = bind_port(argv[1], fds, sizeof(fds) / sizeof(fds[0]));

	if (nfds == 0) {
		return EXIT_FAILURE;
	}

	// Give up root privileges.
	setgid((gid_t)BBSGID);
	setuid((uid_t)BBSUID);
	chdir(BBSHOME);
	umask(S_IWGRP | S_IWOTH);

	while (1) {
		int n = poll(fds, nfds, -1);
		if (n > 0) {
			for (int i = 0; i < nfds; ++i) {
				if (fds[i].revents & POLLIN) {
					accept_connection(fds[i].fd, nfds);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
