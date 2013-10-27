#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fbbs/util.h"

/**
 * Read from /dev/urandom.
 * @param buf The buffer.
 * @param size Bytes to read.
 * @return 0 if OK, -1 on error.
 */
int read_urandom(void *buf, size_t size)
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;
	if (read(fd, buf, size) < size)
		return -1;
	close(fd);
	return 0;
}

/**
 * Get an positive int from /dev/urandom.
 * @return A random integer.
 */
int urandom_pos_int(void)
{
	int i;
	read_urandom(&i, sizeof(i));
	if (i < 0)
		return -i;
	return i;
}

#ifndef NSIG
#ifdef _NSIG
#define NSIG _NSIG
#else
#define NSIG 65
#endif
#endif

/**
 * Daemonize the process.
 */
void start_daemon(void)
{
	umask(0);
	int n = sysconf(_SC_OPEN_MAX);

	int pid = fork();
	if (pid < 0) {
		exit(1);
	} else if (pid != 0) {
		exit(0);
	}
	if (setsid() == -1)
		exit(1);

	if ((pid = fork()) < 0) {
		exit(1);
	} else if (pid != 0) {
		exit(0);
	}

	while (n)
		close(--n);

	for (n = 1; n <= NSIG; n++)
		signal(n, SIG_IGN);
}

sighandler_t fb_signal(int signum, sighandler_t handler)
{
	struct sigaction act;
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

	struct sigaction oact;
	if (sigaction(signum, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
}
