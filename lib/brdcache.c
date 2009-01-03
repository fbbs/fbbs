#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

struct BCACHE *brdshm = NULL;
struct boardheader *bcache = NULL;
int numboards = -1;

//////////// Functions related to shared memory. ////////////

static void attach_err(int shmkey, const char *name, int err)
{
	char buf[STRLEN];
	sprintf(buf, "Error! %s error #%d! key = %x.\n", name, err, shmkey);
	write(STDOUT_FILENO, buf, strlen(buf));
	exit(1);
}

static int search_shmkey(const char *keyname)
{
	int i = 0, found = 0;
	while (shmkeys[i].key != NULL) {
		if (strcmp(shmkeys[i].key, keyname) == 0) {
			found = shmkeys[i].value;
			break;
		}
		i++;
	}
	if (found == 0) {
		char buf[STRLEN];
		sprintf(buf, "search_shmkey(): cannot found %s SHMKEY entry!",
				keyname);
		report(buf, currentuser.userid);
	}
	return found;
}

void *attach_shm(const char *shmstr, int defaultkey, int shmsize)
{
	void *shmptr;
	int shmkey, shmid;

	shmkey = search_shmkey(shmstr);
	if (shmkey < 1024)
		shmkey = defaultkey;
	shmid = shmget(shmkey, shmsize, 0);
	if (shmid < 0) {
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0640);
		if (shmid < 0)
			attach_err(shmkey, "shmget", errno);
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
			attach_err(shmkey, "shmat", errno);
		memset(shmptr, 0, shmsize);
	} else {
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
			attach_err(shmkey, "shmat", errno);
	}
	return shmptr;
}

void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate)
{
	void *shmptr;
	int shmkey, shmid;

	shmkey = search_shmkey(shmstr);
	if (shmkey < 1024)
	shmkey = defaultkey;
	shmid = shmget(shmkey, shmsize, 0);
	if (shmid < 0) {
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0644);
		*iscreate = 1;
		if (shmid < 0)
		attach_err(shmkey, "shmget", errno);
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
		attach_err(shmkey, "shmat", errno);
		memset(shmptr, 0, shmsize);
	} else {
		*iscreate = 0;
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
		attach_err(shmkey, "shmat", errno);
	}
	return shmptr;
}

void remove_shm(const char *shmstr, int defaultkey, int shmsize)
{
	int shmkey, shmid;

	if (shmstr)
		shmkey = sysconf_eval(shmstr);
	else
		shmkey = 0;
	if (shmkey < 1024)
		shmkey = defaultkey;
	shmid = shmget(shmkey, shmsize, 0);
	shmctl(shmid, IPC_RMID, NULL);
}
//////////// End of functions related to shared memory. ////////////