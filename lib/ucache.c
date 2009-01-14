// Handle user cache.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

struct UTMPFILE *utmpshm = NULL;
struct UCACHE *uidshm = NULL;
struct userec lookupuser;

int cmpuids(const char *uid, const struct userec *up)
{
	return !strncasecmp(uid, up->userid, sizeof(up->userid));
}

int dosearchuser(const char *userid, struct userec *user, int *unum)
{
	int id;

	if ((id = getuser(userid)) != 0) {
		if (cmpuids(userid, &lookupuser)) {
			memcpy(user, &lookupuser, sizeof(*user));
			return *unum = id;
		}
	}
	memset(user, 0, sizeof(*user));
	return *unum = 0;
}

