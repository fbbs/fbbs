#include <ev.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include "site.h"
#include "fbbs/fileio.h"
#include "fbbs/parcel.h"
#include "fbbs/string.h"
#include "fbbs/util.h"

enum {
	DEFAULT_MAX_SERVERS = 1,
	DEFAULT_MAX_CLIENTS = 10,

	REMOTE_NULL = -1,
	REMOTE_BUSY = -2,
};

typedef struct {
	int pid;
	int remote_fd;
	int received;
	int length;
	bool client;
	ev_io watcher;
} connection_t;

typedef struct {
	int pid;
	int fd;
} server_t;

static const char *server_path;

static connection_t *connections;
static server_t *servers;
static int server_size;

static int max_servers;
static int max_connections;

static int socket_path_length;

static ev_io socket_watcher;

static bool proxy_shutdown;

static void connection_reset(connection_t *conn)
{
	conn->pid = -1;
	conn->remote_fd = REMOTE_NULL;
	conn->received = 0;
	conn->length = 0;
}

static int check_max_clients(int max_clients)
{
	if (max_clients <= 0)
		max_clients = DEFAULT_MAX_CLIENTS;

	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim) < 0)
		return -1;

	max_connections = max_clients + max_servers + 10;
	if (max_connections >= rlim.rlim_cur) {
		rlim.rlim_max = rlim.rlim_cur = max_connections;
		if (setrlimit(RLIMIT_NOFILE, &rlim) < 0)
			return -1;
	}

	connections = malloc(sizeof(*connections) * max_connections);
	if (!connections)
		return -1;
	for (int i = 0; i < max_connections; ++i) {
		connection_reset(connections + i);
	}
	return max_clients;
}

static server_t *find_server(int fd)
{
	for (int i = 0; i < max_servers; ++i) {
		if (servers[i].fd == fd)
			return servers + i;
	}
	return NULL;
}

static bool spawn_server(server_t *server)
{
	server->fd = -1;
	int pid = fork();
	if (pid > 0) {
		server->pid = pid;
	} else if (pid == 0) {
		execl(server_path, server_path, NULL);
	} else {
		return false;
	}
	return true;
}

static bool valid_remote_fd(int local_fd)
{
	connection_t *conn = connections + local_fd;
	if (conn->client) {
		return conn->remote_fd >= 0 && conn->remote_fd < max_connections;
	} else {
		return conn->remote_fd >= 0 && conn->remote_fd < max_connections
			&& connections[conn->remote_fd].remote_fd == local_fd;
	}
}

static void connection_error_callback(int fd)
{
	connection_t *conn = connections + fd;
	bool client = conn->client;
	int remote_fd = valid_remote_fd(fd) ? conn->remote_fd : REMOTE_NULL;

	connection_reset(conn);
	ev_io_stop(EV_DEFAULT_ &conn->watcher);
	close(fd);

	if (client) {
		if (remote_fd >= 0) {
			server_t *server = find_server(remote_fd);
			if (server && server->pid >= 0)
				kill(server->pid, SIGHUP);
		}
	} else {
		server_t *server = find_server(fd);
		if (server) {
			server->pid = -1;
			server->fd = -1;
			--server_size;
			if (proxy_shutdown) {
				if (!server_size)
					ev_break(EV_DEFAULT_ EVBREAK_ALL);
			} else {
				spawn_server(server);
			}
		}
	}
}

static int assign_server(int client_fd)
{
	for (int i = 0; i < max_servers; ++i) {
		int fd = servers[i].fd;
		if (fd >= 0 && fd < max_connections) {
			connection_t *conn = connections + fd;
			if (!conn->client && conn->remote_fd < 0) {
				conn->remote_fd = client_fd;
				return fd;
			}
		}
	}
	return REMOTE_BUSY;
}

static void data_received_callback(int fd, int bytes)
{
	connection_t *conn = connections + fd;
	while (bytes > 0) {
		uchar_t buf[4096];
		int rc = file_read(fd, buf,
				bytes >= sizeof(buf) ? sizeof(buf) : bytes);
		if (rc <= 0) {
			connection_error_callback(fd);
			return;
		} else {
			int received = conn->received;
			conn->received += rc;
			if (received < PARCEL_SIZE_LENGTH) {
				for (int i = received; i < PARCEL_SIZE_LENGTH
						&& i < conn->received; ++i) {
					conn->length |= (buf[i - received] << (i * 8));
				}
			}

			if (conn->received >= PARCEL_SIZE_LENGTH
					&& conn->received > conn->length) {
				connection_error_callback(fd);
				return;
			}

			if (conn->client && conn->remote_fd == REMOTE_NULL) {
				conn->remote_fd = assign_server(fd);
			}

			int remain = conn->received < PARCEL_SIZE_LENGTH
					? -1 : conn->length - received;
			if (valid_remote_fd(fd)) {
				file_write(conn->remote_fd, buf,
						remain > 0 && remain < rc ? remain : rc);
			}

			if ((!conn->client || conn->remote_fd == REMOTE_BUSY)
					&& conn->received >= PARCEL_SIZE_LENGTH
					&& conn->received == conn->length) {
				conn->received = 0;
				conn->length = 0;
				conn->remote_fd = REMOTE_NULL;
				break;
			}
		}
		bytes -= rc;
	}
	if (bytes > 0)
		connection_error_callback(fd);
}

static void fd_callback(EV_P_ ev_io *w, int revents)
{
	if (revents & EV_READ) {
		int bytes;
		ioctl(w->fd, FIONREAD, &bytes);
		if (bytes > 0) {
			data_received_callback(w->fd, bytes);
			return;
		}
	}
	connection_error_callback(w->fd);
}

static void socket_callback(EV_P_ ev_io *w, int revents)
{
	if (revents & EV_READ) {
		struct sockaddr_un un;
		memset(&un, 0, sizeof(un));
		socklen_t len = sizeof(un);
		int fd = accept(w->fd, (struct sockaddr *) &un, &len);
		if (fd < 0)
			return;

		int flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);

		if (fd < max_connections
				&& strlen(un.sun_path) > socket_path_length + 8) {
			int pid = strtol(un.sun_path + socket_path_length + 8, NULL, 10);
			if (pid > 0) {
				connection_t *conn = connections + fd;
				connection_reset(conn);
				conn->pid = pid;
				conn->client = (un.sun_path[socket_path_length + 1] == 'c');

				if (!conn->client) {
					for (int i = 0; i < max_servers; ++i) {
						if (servers[i].pid == pid) {
							servers[i].fd = fd;
							++server_size;
							break;
						}
					}
				}

				ev_io_init(&conn->watcher, fd_callback, fd, EV_READ);
				ev_io_start(EV_DEFAULT_ &conn->watcher);
				return;
			}
		}
		close(fd);
	}
}

static int bind_unix_path(const char *path)
{
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (fd < 0)
		return -1;

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/proxy", path);

	(void) unlink(addr.sun_path);
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0
			|| listen(fd, SOMAXCONN) != 0) {
		close(fd);
		return -1;
	}

	socket_path_length = strlen(path);

	ev_io_init(&socket_watcher, socket_callback, fd, EV_READ);
	ev_io_start(EV_DEFAULT_ &socket_watcher);
	return fd;
}

static void reap_child(int signum)
{
	int pid, status;
	while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
		;
	}
}

static void kill_servers(void)
{
	for (int i = 0; i < max_servers; ++i) {
		int pid = servers[i].pid;
		if (pid > 0)
			kill(pid, SIGTERM);
	}
}


static void shutdown_backend(int signum)
{
	proxy_shutdown = true;
	kill_servers();
}

int main(int argc, char **argv)
{
	const char *socket_path = getenv("FBBS_SOCKET_PATH");
	server_path = getenv("FBBS_SERVER_PATH");
	if (!socket_path || !server_path)
		return EXIT_FAILURE;

	start_daemon();

	const char *fbbs_max_servers = getenv("FBBS_MAX_SERVERS");
	if (fbbs_max_servers)
		max_servers = strtol(fbbs_max_servers, NULL, 10);
	if (max_servers <= 0)
		max_servers = DEFAULT_MAX_SERVERS;
	servers = malloc(sizeof(*servers) * max_servers);
	if (!servers)
		return EXIT_FAILURE;
	for (int i = 0; i < max_servers; ++i) {
		servers[i].pid = -1;
		servers[i].fd = -1;
	}

	int max_clients = 0;
	const char *fbbs_max_clients = getenv("FBBS_MAX_CLIENTS");
	if (fbbs_max_clients)
		max_clients = strtol(fbbs_max_clients, NULL, 10);
	max_clients = check_max_clients(max_clients);
	if (max_clients <= 0)
		return EXIT_FAILURE;

	if (setgid(BBSGID) != 0)
		return EXIT_FAILURE;
	if (setuid(BBSUID) != 0)
		return EXIT_FAILURE;

	int fd = bind_unix_path(socket_path);
	if (fd < 0)
		return EXIT_FAILURE;

	fb_signal(SIGPIPE, SIG_IGN);
	fb_signal(SIGCHLD, reap_child);
	fb_signal(SIGTERM, shutdown_backend);

	for (int i = 0; i < max_servers; ++i) {
		if (!spawn_server(servers + i)) {
			kill_servers();
			return EXIT_FAILURE;
		}
	}

	ev_run(EV_DEFAULT_ 0);
	return EXIT_SUCCESS;
}
