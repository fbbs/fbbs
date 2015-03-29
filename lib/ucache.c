// Handle user cache.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include "record.h"

#include "fbbs/dbi.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/ucache.h"

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

enum {
	MAX_LOGINS_NORMAL = 2,   ///< max logins for a single account.
	MAX_LOGINS_BM = 4,       ///< max logins for a board manager.
	MAX_LOGINS_DIRECTOR = 6, ///< max logins for a director(zhanwu).
};

// The starting address of cache for all users.
struct UCACHE *uidshm = NULL;
// A global variable to hold result when searching users.
struct userec lookupuser;

int cmpuids(const void *uid, const void *up)
{
	return !strncasecmp((const char *)uid, ((const struct userec *)up)->userid,
		sizeof(((const struct userec *)up)->userid));
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
int uhashkey(const char *userid, int *a1, int *a2)
{
	const char *c = userid;
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

// Put userid(in struct uentp) into cache for all users.
// Find a proper entry of user hash.
static int fillucache(const struct userec *uentp, int count)
{
	int a1, a2;
	int key;

	if (count < MAXUSERS) {
		strlcpy(uidshm->userid[count++], uentp->userid, sizeof(uidshm->userid[0]));
		if(uentp->userid[0] != '\0') {
			key = uhashkey(uentp->userid, &a1, &a2);
			if (uidshm->hash[a1][a2][key] == 0) {
				// If the hash entry is empty, put 'count' in it.
				uidshm->hash[a1][a2][key] = count;
			} else {
				// Put 'count' into the doubly linked list.
				int i = uidshm->hash[a1][a2][key];
				while (uidshm->next[i - 1] != 0)
					i = uidshm->next[i - 1];
				uidshm->next[i - 1] = count;
				uidshm->prev[count - 1] = i;
			}
			return 1;
		}
	}
	return 0;
}

/* hash 删除 */
int del_uidshm(int num, char *userid)
{
	int a1, a2;
	int key;
	int i;

	if (num <= 0 || num > MAXUSERS)
		return 0;

	key = uhashkey(userid, &a1, &a2);
	i=uidshm->hash[a1][a2][key];
	if (i<=0)
		return 0;
	for (; i!=num && i>0; i=uidshm->next[i-1])
		;
	if (i!= num || i<=0)
		return 0;
	if (uidshm->next[i-1]) {
		if (uidshm->prev[i-1]) {
			uidshm->next[ uidshm->prev[i-1]-1 ] = uidshm->next[i-1];
			uidshm->prev[ uidshm->next[i-1]-1 ] = uidshm->prev[i-1];
		} else {
			uidshm->hash[a1][a2][key] = uidshm->next[i-1];
			uidshm->prev[ uidshm->next[i-1]-1 ] = 0;
		}
	} else {
		if (uidshm->prev[i-1]) {
			uidshm->next[ uidshm->prev[i-1]-1 ] = 0;
		} else {
			uidshm->hash[a1][a2][key] = 0;
		}
	}
	uidshm->next[i-1]=0;
	uidshm->prev[i-1]=0;
	uidshm->userid[i-1][0]='\0';
	return 1;
}
/* endof hash删除 */

// Places an exclusive lock on file 'lockname'.
// Returns file descriptor if OK, -1 on error.
static int shm_lock(const char *lockname)
{
	int fd = open(lockname, O_RDWR | O_CREAT, 0600);
	if (fd < 0) {
		return -1;
	}
	if (file_lock_all(fd, FILE_WRLCK) == -1) {
		close(fd);
		return -1;
	}
	return fd;
}

// Removes an existing lock held by this process on file descriptor 'fd'.
static void shm_unlock(int fd)
{
	file_lock_all(fd, FILE_UNLCK);
	close(fd);
}

#define utmp_unlock(fd) shm_unlock(fd)
#define utmp_lock() shm_lock("tmp/.UTMP.lock")

int ucache_lock(void)
{
	return shm_lock("tmp/.UCACHE.lock");
}

void ucache_unlock(int fd)
{
	shm_unlock(fd);
}

// Loads PASSFILE into cache for all users.
// Returns 0 on success, -1 on error.
int load_ucache(void)
{
	int fd, iscreate = 0, passwdfd, i;

	// Lock cache.
	fd = ucache_lock();
	if (fd == -1) {
		return -1;
	}

	// Get shared memory.
	if (uidshm == NULL) {
		uidshm = attach_shm2("UCACHE_SHMKEY", 3696, sizeof(*uidshm),
				&iscreate);
		if(uidshm == NULL)
			exit(1);
	}
	log_usies("CACHE", "reload ucache", NULL);

	// Load PASSFILE.
	if ((passwdfd = open(PASSFILE, O_RDWR | O_CREAT, 0644)) == -1) {
		ucache_unlock(fd);
		exit(-1);
	}
	ftruncate(passwdfd, MAXUSERS * sizeof(struct userec));
	close(passwdfd);
	if (get_records(PASSFILE, uidshm->passwd, sizeof(struct userec), 1,
			MAXUSERS) != MAXUSERS) {
		ucache_unlock(fd);
		return -1;
	}

	// Initialize 'userid' and hash.
	memset(uidshm->userid, 0, sizeof(uidshm->userid));
	memset(uidshm->hash, 0, sizeof(uidshm->hash));
	memset(uidshm->next, 0, sizeof(uidshm->next));
	memset(uidshm->prev, 0, sizeof(uidshm->prev));

	// Fill cache.
	int last = 0;
	for (i = 0; i < MAXUSERS; i++) {
		if (fillucache(&(uidshm->passwd[i]), i))
			last = i;
	}
	uidshm->number = ++last;
	uidshm->uptime = time(NULL);

	// Unlock cache.
	ucache_unlock(fd);

	return 0;
}

int substitut_record(char *filename, const void *rptr, size_t size, int id)
{
	memcpy(&(uidshm->passwd[id - 1]), rptr, size);
	return 0;
}

// Flushes cache for all users to PASSFILE.
int flush_ucache(void)
{
	return substitute_record(PASSFILE, uidshm->passwd,
			sizeof(uidshm->passwd), 1);
}

// Returns -1 if 'uidshm' == NULL and shared memory does not exist.
// Otherwise does nothing and returns 0.
int resolve_ucache(void)
{
	int iscreate = 0;
	if (uidshm == NULL) {
		uidshm = attach_shm2("UCACHE_SHMKEY", 3696, sizeof(*uidshm),
				&iscreate);
		if (uidshm == NULL)
			return -1;
	}
	if (iscreate) {
		remove_shm("UCACHE_SHMKEY", 3696, sizeof(*uidshm));
		report("Error: miscd is not running!", "");
		return -1;
	}
	return 0;
}

void setuserid(int num, const char *userid)
{
	if (num > 0 && num <= MAXUSERS) {
		if (num > uidshm->number)
			uidshm->number = num;
		strlcpy(uidshm->userid[num - 1], userid, IDLEN + 1);
		/* hash 填充 */
		if (strcmp(userid, "new") ) {
			int a1, a2;
			int key;

			key = uhashkey(userid, &a1, &a2);
			if (uidshm->hash[a1][a2][key] == 0) {
				uidshm->hash[a1][a2][key] = num;
				uidshm->next[num-1]=0;
				uidshm->prev[num-1]=0;
			} else {
				int i;
				for (i=uidshm->hash[a1][a2][key]; uidshm->next[i-1]; i
						=uidshm->next[i-1])
					;//找到一个空位置

				uidshm->next[i-1] = num;
				uidshm->prev[num-1] = i;
				uidshm->next[num-1] = 0;
			}
		}
		/* end of hash 填充 */
	}
}

int searchnewuser(void)
{
	register int num, i;
	if (resolve_ucache() == -1)
		return 0;
	num = uidshm->number;
	for (i = 0; i < num; i++)
		if (uidshm->userid[i][0] == '\0')
			return i + 1;
	if (num < MAXUSERS)
		return (num + 1);
	return 0;
}

// Get 'userid' according to ('uid' - 1).
int getuserid(char *userid, int uid, size_t len)
{
	if (resolve_ucache() == -1)
		return -1;
	strlcpy(userid, uidshm->userid[uid - 1], len);
	return 0;
}

// Returns the place of 'userid' in cache for all users, 0 if not found.
int searchuser(const char *userid)
{
	register int i;
	int a1, a2;
	int key;

	if (resolve_ucache() == -1)
		return 0;
	key = uhashkey(userid, &a1, &a2);
	i = uidshm->hash[a1][a2][key];
	while (i) {
		if (!strncasecmp(userid, uidshm->userid[i - 1],
				sizeof(uidshm->userid[0]))) {
			return i;
		}
		i = uidshm->next[i - 1];
	}
	return 0;
}

// Gets struct userec in cache for all users according to 'userid'.
// struct userec is stored in *'u'. Returns uid.
int getuserec(const char *userid, struct userec *u)
{
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	*u = uidshm->passwd[uid - 1];
	return uid;
}

// Similar to 'getuserec',
// but stores result in global variable 'lookupuser'.
int getuser(const char *userid)
{
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	lookupuser = uidshm->passwd[uid - 1];
	return uid;
}

// Puts struct userec in *'u' according to ('uid' - 1).
int getuserbyuid(struct userec *u, int uid)
{
	if (resolve_ucache() == -1)
		return -1;
	*u = uidshm->passwd[uid - 1];
	return uid;
}

int cmpfnames(void *user, void *over)
{
	const char *userid = user;
	struct override *ov = over;
	return !strcasecmp(userid, ov->id);
}

/*
 * Create a new user (unique in userid).
 * @param user The initial user data.
 * @return 0 if OK, <0 on error.
 */
int create_user(const struct userec *user)
{
	char path[HOMELEN];
	snprintf(path, sizeof(path), "home/%c/%s",
			toupper(user->userid[0]), user->userid);
	if (dashd(path))
		return UCACHE_EEXIST;
	
	db_res_t *res = db_cmd("INSERT INTO users (name, passwd)"
			" VALUES (%s, %s)", user->userid, user->passwd);
	db_clear(res);
	if (!res)
		return UCACHE_EEXIST;

	int fd = ucache_lock();
	if (fd < 0)
		return UCACHE_EINTNL;

	int i = searchnewuser();
	if (i == 0) {
		ucache_unlock(fd);
		return UCACHE_EFULL;
	}

	setuserid(i, user->userid);
	substitut_record(PASSFILE, user, sizeof(*user), i);

	ucache_unlock(fd);

	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "new account from %s", user->lasthost);
	report(buf, user->userid);

	return 0;
}

int get_login_quota(const struct userec *user)
{
	if (strcaseeq("guest", user->userid))
		return MAXGUEST;

	if (HAS_PERM2(PERM_MULTILOG, user))
		return INT_MAX;

	if (HAS_PERM2(PERM_SPECIAL0, user))
		return MAX_LOGINS_DIRECTOR;
	
	if (HAS_PERM2(PERM_BOARDS, user))
		return MAX_LOGINS_BM;

	return MAX_LOGINS_NORMAL;
}
