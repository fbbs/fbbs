// Handle user cache.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

struct UTMPFILE *utmpshm = NULL;
struct UCACHE *uidshm = NULL;
struct userec lookupuser;

int cmpuids(void *uid, void *up)
{
	return !strncasecmp((char *)uid, ((struct userec *)up)->userid,
		sizeof(((struct userec *)up)->userid));
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

// Returns hashkey of 'userid'.
// The hashkey is the sum of ASCII codes of the third to the last letter
// in uppercase.
// a1 is 0~25 (A-Z or a-z respectively) for the first letter.
// a2 is 0~25 (A-Z or a-z respectively) for the second letter.
int uhashkey(const char *userid, char *a1, char *a2)
{
	char *c = userid;
	int key = 0;

	if (*c >= 'a' && *c <= 'z') {
		*a1 = *c - 'a';
	} else if ( *c >= 'A' && *c <= 'Z') {
		*a1 = *c - 'A';
	} else {
		*a1 = 0;
	}
	c++;

	if ( *c >= 'a' && *c <= 'z') {
		*a2 = *c - 'a';
	} else if ( *c >= 'A' && *c <= 'Z') {
		*a2 = *c - 'A';
	} else {
		*a2 = 0;
	}
	c++;

	while (*c != '\0') {
		key += toupper(*c);
		c++;
	}
	return key % 256;
}
