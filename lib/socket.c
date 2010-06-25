#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "fbbs/string.h"

/**
 * Create a unix domain dgram socket and then connect to a server.
 * @param basename The basename of the created socket.
 *                 The actual path is suffixed with pid.
 * @param server The server path.
 * @return fd if OK, -1 on error.
 */
int unix_dgram_connect(const char *basename, const char *server)
{
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	snprintf(un.sun_path, sizeof(un.sun_path), "%s-%d", basename, getpid());
	unlink(un.sun_path);

	int len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(fd, (struct sockaddr *)&un, len) < 0) {
		close(fd);
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strlcpy(un.sun_path, server, sizeof(un.sun_path));
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (connect(fd, (struct sockaddr *)&un, len) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

/**
 * Create and bind a unix datagram socket.
 * @param name The name of the socket.
 * @param qlen The maximum length of queue of pending requests.
 * @return fd if OK, -1 on error.
 */
int unix_dgram_bind(const char *name, int qlen)
{
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	unlink(name);

	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strlcpy(un.sun_path, name, sizeof(un.sun_path));

	int len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (bind(fd, (struct sockaddr *)&un, len) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}
