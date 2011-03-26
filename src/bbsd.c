#include <arpa/telnet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/wait.h>

#ifdef SSHBBS
#include "libssh/libssh.h"
#include "libssh/server.h"
#endif // SSHBBS
#include "bbs.h"
#include "mmap.h"
#include "fbbs/fileio.h"
#include "fbbs/string.h"

#ifndef NSIG
#ifdef _NSIG
#define NSIG _NSIG
#else
#define NSIG 65
#endif
#endif

#ifdef SSHBBS
#define KEYS_FOLDER BBSHOME"/etc/ssh"
#define PID_FILE	BBSHOME"/reclog/sshbbsd.pid"
#else // SSHBBS
#define PID_FILE	BBSHOME"/reclog/bbsd.pid"
#endif // SSHBBS
#define LOG_FILE	BBSHOME"/reclog/bbsd.log"
#define NOLOGIN		BBSHOME"/NOLOGIN"
#ifdef IP_2_NAME
#define IPNAME_FILE BBSHOME"/etc/hosts"
#endif // IP_2_NAME

enum {
	DEFAULT_PORT = 12345, ///< default listening port number
	SOCKET_QLEN = 4, ///< maximum length of the queue of pending connections
};

extern char fromhost[];

// TODO: deprecate this
char genbuf[1024]; ///< global buffer for strings. 

#ifdef SSHBBS
ssh_channel ssh_chan;
#endif // SSHBBS

typedef void (*sighandler_t)(int);

static sighandler_t fb_signal(int signum, sighandler_t handler)
{
	struct sigaction act, oact;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signum == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef  SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if (sigaction(signum, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}

/**
 * Get remote ip address.
 * @param from socket struct
 * @note the result is stored in the global variable 'fromhost'.
 */
static void get_ip_addr(const struct sockaddr_storage *sa)
{
	char host[IP_LEN];
	getnameinfo((struct sockaddr *)sa, sizeof(*sa), host, sizeof(host),
			NULL, 0, NI_NUMERICHOST);

	strlcpy(fromhost, host, IP_LEN);
#ifdef IP_2_NAME
	FILE *fp = fopen(BBSHOME"/etc/hosts", "r");
	if (!fp)
		return;
	char buf[STRLEN], *field;
	while (fgets(buf, sizeof(buf), fp)) {
		field = strtok(buf, " \r\n\t");
		if (field != NULL && *field != '#') {
			if (strcmp(fromhost, field) == 0) {
				field = strtok(NULL, " \r\n\t");
				if (field)
					strlcpy(fromhost, field, IP_LEN);
				break;
			}
		}
	}
	fclose(fp);
#endif // IP_2_NAME
}

/**
 * Wait for an child to terminate.
 * @param notused only to match sighandler_t prototype
 */
static void reapchild(int notused)
{
	int state, pid;
	while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0)
		;
}

/**
 * Daemonize the process.
 */
static void start_daemon(void)
{
	int n, pid;

	umask(0); // Clear file creation mask.
	n = getdtablesize(); // Get maximum number of file descriptors.

	// Fork to ensure not being a process group leader.
	if ((pid = fork()) < 0) {
		printf("Cannot fork.\n");
		exit(1);
	} else if (pid != 0) {
		exit(0);
	}
	if (setsid() == -1)
		exit(1);

	// Fork again.
	if ((pid = fork()) < 0) {
		exit(1);
	} else if (pid != 0) {
		exit(0);
	}

	// Close all open file descriptors.
	while (n)
		close(--n);

	// Ignore all signals (except SIGKILL & SIGSTOP of course)
	for (n = 1; n <= NSIG; n++)
		signal(n, SIG_IGN);
}

/**
 * Exit.
 * @param notused only to match sighandler_t prototype
 */
static void close_daemon(int notused)
{
	exit(0);
}

/**
 * Log bbsd events.
 * @param str string to log.
 * @return 0 on success, -1 on error.
 */
static int bbsd_log(const char *str)
{
	char buf[256];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	snprintf(buf, sizeof(buf), "%.2d/%.2d/%.2d %.2d:%.2d:%.2d bbsd[%d]: %s",
			t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour,
			t->tm_min, t->tm_sec, getpid(), str);
	return file_append(LOG_FILE, buf);
}

#ifndef SSHBBS
// TODO: rewrite this
static void telnet_init(void)
{
	static unsigned char svr[] = { IAC, WILL, TELOPT_ECHO, IAC, WILL, TELOPT_SGA };
	write_stdout(svr, sizeof(svr));
}

/**
 * Port Binding.
 * @param port The port number.
 * @param fds The pollfd array.
 * @param nfds Number of pollfd elements.
 * @return Number of binded sockets.
 */
static int bind_port(const char *port, struct pollfd *fds, int nfds)
{
	char buf[80];

	struct addrinfo hints, *ai;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	int e = getaddrinfo(NULL, port, &hints, &ai);
	if (e)
		return -1;
	
	int n = 0;
	for (struct addrinfo *p = ai; n < nfds && p; p = p->ai_next) {
		int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (fd < 0) {
			freeaddrinfo(ai);
			return 0;
		}

		int on = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (p->ai_family == AF_INET6)
			setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));

		if (bind(fd, p->ai_addr, p->ai_addrlen) != 0
				|| listen(fd, SOMAXCONN) != 0) {
			sprintf(buf, "%s() can't bind/listen to %s errno %d\n",
					__func__, port, errno);
			bbsd_log(buf);
			close(fd);
		} else {
			fds[n].fd = fd;
			fds[n].events = POLLIN;
			++n;
			sprintf(buf, "started on port %s\n", port);
			bbsd_log(buf);
		}
	}

	return n;
}

#ifdef NOLOGIN
/**
 * Deny new login if NOLOGIN file exists.
 * @param fd output file descriptor
 * @return 0 if login is permitted, -1 otherwise
 * @note '\\r' is not added, so prompt message is not recommended to exceed
 *       1 line.
 */
static int check_nologin(int fd)
{
	const char banner[] = "本站目前暂停登录，原因如下: \r\n";
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(NOLOGIN, &m) == 0) {
		write(fd, banner, sizeof(banner));
		write(fd, m.ptr, m.size);
		mmap_close(&m);
		close(fd);
		return -1;
	}
	return 0;
}
#endif // NOLOGIN

#else // SSHBBS
static bool channel_reply(ssh_session session, enum ssh_channel_requests_e req)
{
	ssh_message msg;
	bool expect = false;
	do {
		msg = ssh_message_get(session);
		if (msg && ssh_message_type(msg)==SSH_REQUEST_CHANNEL &&
				ssh_message_subtype(msg) == req) {
			expect = true;
			ssh_message_channel_request_reply_success(msg);
			break;
		}
		if (!expect) {
			ssh_message_reply_default(msg);
		}
		ssh_message_free(msg);
	} while (msg && !expect);
	if (!expect)
		return false;
	return true;
}

/**
 *
 */
static ssh_channel sshbbs_accept(ssh_bind sshbind, ssh_session session)
{
	ssh_message msg;
	ssh_channel chan = NULL;
	bool auth = false;
	int ret;
	int attempt = 0;
	if (ssh_accept(session) == 0) {
		do {
			msg = ssh_message_get(session);
			if (!msg)
				break;
			switch (ssh_message_type(msg)) {
				case SSH_REQUEST_AUTH:
					switch(ssh_message_subtype(msg)) {
						case SSH_AUTH_METHOD_PASSWORD:
							ret = bbs_auth(ssh_message_auth_user(msg),
									ssh_message_auth_password(msg));
							switch (ret) {
								case BBS_ENOUSR:
								case BBS_EWPSWD:
									attempt++;
									ssh_message_reply_default(msg);
									break;
								case 0:
									auth = true;
									ssh_message_auth_reply_success(msg, 0);
									break;
								default:
									ssh_disconnect(session);
									return NULL;
							}
							break;
						case SSH_AUTH_METHOD_NONE:
						default:
							ssh_message_auth_set_methods(msg, 
									SSH_AUTH_METHOD_PASSWORD);
							ssh_message_reply_default(msg);
							attempt++;
							break;
					}
					break;
				default:
					ssh_message_reply_default(msg);
			}
			ssh_message_free(msg);
		} while (!auth && attempt < LOGINATTEMPTS);
	}
	if (!auth) {
		ssh_disconnect(session);
		return NULL;
	}
	
    do {
        msg = ssh_message_get(session);
        if (msg) {
			switch(ssh_message_type(msg)) {
				case SSH_REQUEST_CHANNEL_OPEN:
					if (ssh_message_subtype(msg) == SSH_CHANNEL_SESSION) {
						chan = ssh_message_channel_request_open_reply_accept(msg);
                        break;
                    }
                default:
					ssh_message_reply_default(msg);
            }
            ssh_message_free(msg);
        }
    } while (msg && !chan);
	if (!chan) {
        ssh_disconnect(session);
        return NULL;
    }

	if (!channel_reply(session, SSH_CHANNEL_REQUEST_PTY)
			|| !channel_reply(session, SSH_CHANNEL_REQUEST_SHELL)) {
		ssh_disconnect(session);
		return NULL;
	}
	return chan;
}
#endif // SSHBBS

static int accept_connection(int fd, int nfds, const struct sockaddr_storage *p
#ifdef SSHBBS
		, ssh_bind sshbind, ssh_session session
#endif
)
{
	pid_t pid = fork();
	if (pid < 0)
		return -1;

	if (pid == 0) {
		while (--nfds >= 0)
			close(nfds);
		dup2(fd, STDIN_FILENO);
		get_ip_addr(p);
#ifdef SSHBBS
		ssh_chan = sshbbs_accept(sshbind, session);
		if (!ssh_chan)
			exit(1);
#else // SSHBBS
		close(fd);
		dup2(STDIN_FILENO, STDOUT_FILENO);
		telnet_init();
#endif // SSHBBS
		start_client();
	} else {
		close(fd);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf("Usage: %s [port]\n", argv[0]);
		return EXIT_FAILURE;
	}

	start_daemon();

	fb_signal(SIGCHLD, reapchild);
	fb_signal(SIGTERM, close_daemon);

#ifdef SSHBBS
	ssh_bind sshbind = ssh_bind_new();
	ssh_session session;
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY,
			KEYS_FOLDER"/ssh_host_dsa_key");
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
			KEYS_FOLDER"/ssh_host_rsa_key");
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY,
			SSH_LOG_NOLOG);
#endif // SSHBBS

	char buf[80];
	int nfds;
#ifdef SSHBBS
	// libssh server does not support ipv6 until upcoming version 0.5,
	// so let's just bind to ipv4 socket.
	int port = strtol(argv[1], NULL, 10);
	if (port <= 0)
		port = DEFAULT_PORT;
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	ssh_bind_set_blocking(sshbind, 1);
	if (ssh_bind_listen(sshbind) < 0) {
		snprintf(buf, sizeof(buf), "listen failed: %s.\n", ssh_get_error(sshbind));
		bbsd_log(buf);
		exit(1);
	}
	nfds = ssh_bind_get_fd(sshbind) + 1;
#else // SSHBBS
	struct pollfd fds[2];
	nfds = bind_port(argv[1], fds, sizeof(fds) / sizeof(fds[0]));
#endif // SSHBBS
	if (nfds < 0)
		return EXIT_FAILURE;

	// Give up root privileges.
	setgid((gid_t)BBSGID);
	setuid((uid_t)BBSUID);
	chdir(BBSHOME);
	umask(S_IWGRP | S_IWOTH);

	// Log pid.
	sprintf(buf, "%d\n", getpid());
	unlink(PID_FILE);
	file_append(PID_FILE, buf);

	// Main loop.
	while (1) {
		struct sockaddr_storage sa;
		socklen_t slen = sizeof(sa);
#ifdef SSHBBS
		session = ssh_new();
		if (ssh_bind_accept(sshbind, session) == SSH_ERROR) {
			ssh_free(session);
			continue;
		}
		int fd = ssh_get_fd(session);
		getpeername(fd, (struct sockaddr *)&sa, &slen);
		accept_connection(fd, nfds, &sa, sshbind, session);
#else // SSHBBS
		int n = poll(fds, nfds, -1);
		if (n > 0) {
			for (int i = 0; i < nfds; ++i) {
				if (fds[i].revents & POLLIN) {
					int fd = accept(fds[i].fd, (struct sockaddr *)&sa, &slen);
#ifdef NOLOGIN
					if (check_nologin(fd) < 0) {
						close(fd);
					}
#endif // NOLOGIN
					accept_connection(fd, nfds, &sa);
				}
			}
		}

#endif // SSHBBS
	}
	return 0;
}
