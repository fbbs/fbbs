#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fbbs/util.h"

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

	// Ignore all signals.
	for (n = 1; n <= NSIG; n++)
		signal(n, SIG_IGN);
}

/**
 * BSD semantic signal().
 * @param signum The signal number.
 * @param handler The signal handler.
 * @return The signal handler on success, SIG_ERR on error.
 */
sighandler_t fb_signal(int signum, sighandler_t handler)
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
		return SIG_ERR;
	return oact.sa_handler;
}

void *fb_malloc(size_t size)
{
	void *ptr = malloc(size);
	if (!ptr)
		exit(0);
	return ptr;
}

