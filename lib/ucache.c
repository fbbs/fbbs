// Handle user cache.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

// The starting address of cache for online users.
struct UTMPFILE *utmpshm = NULL;
// The starting address of cache for all users.
struct UCACHE *uidshm = NULL;
// A global variable to hold result when searching users.
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
		}
	}
	return count;
}

/* hash É¾³ý */
int del_uidshm(int num, char *userid)
{
	int a1, a2;
	int key;
	int i;

	if (num <= 0 || num > MAXUSERS)
		return;

	key = uhashkey(userid, &a1, &a2);
	i=uidshm->hash[a1][a2][key];
	if (i<=0)
		return;
	for (; i!=num && i>0; i=uidshm->next[i-1])
		;
	if (i!= num || i<=0)
		return;
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

}
/* endof hashÉ¾³ý */

// Places an exclusive lock on file 'lockname'.
// Returns file descriptor if OK, -1 on error.
static int shm_lock(const char *lockname)
{
	int lockfd;

	lockfd = open(lockname, O_RDWR | O_CREAT, 0600);
	if (lockfd < 0) {
		return -1;
	}
	if (flock(lockfd, LOCK_EX) == -1)
		return -1;
	return lockfd;
}

// Removes an existing lock held by this process on file descriptor 'fd'.
static void shm_unlock(int fd)
{
	flock(fd, LOCK_UN);
	close(fd);
}

#define ucache_unlock(fd) shm_unlock(fd)
#define utmp_unlock(fd) shm_unlock(fd)
#define ucache_lock() shm_lock("tmp/.UCACHE.lock")
#define utmp_lock() shm_lock("tmp/.UTMP.lock")

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
	int count = 0;
	for (i = 0; i < MAXUSERS; i++)
		count = fillucache(&(uidshm->passwd[i]), count);
	uidshm->number = count;
	uidshm->uptime = time(NULL);

	// Unlock cache.
	ucache_unlock(fd);

	return 0;
}

int substitut_record(char *filename, char *rptr, int size, int id)
{
	memcpy(&(uidshm->passwd[id-1]), rptr, size);
}

// Flushes cache for all users to PASSFILE.
int flush_ucache(void)
{
	return substitute_record(PASSFILE, uidshm->passwd,
			sizeof(uidshm->passwd), 1);
}

// Exits if 'uidshm' == NULL and shared memory does not exist.
// Does nothing otherwise.
void resolve_ucache(void)
{
	int iscreate = 0;
	if (uidshm == NULL) {
		uidshm = attach_shm2("UCACHE_SHMKEY", 3696, sizeof(*uidshm),
				&iscreate);
		if (uidshm == NULL)
			exit(1);
	}
	if (iscreate) {
		remove_shm("UCACHE_SHMKEY", 3696, sizeof(*uidshm));
		report("Error: miscd is not running!", "");
		exit(1);
	}
}

void setuserid(int num, char *userid)
{
	if (num > 0 && num <= MAXUSERS) {
		if (num > uidshm->number)
			uidshm->number = num;
		strlcpy(uidshm->userid[num - 1], userid, IDLEN + 1);
		/* hash Ìî³ä */
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
					;//ÕÒµ½Ò»¸ö¿ÕÎ»ÖÃ

				uidshm->next[i-1] = num;
				uidshm->prev[num-1] = i;
				uidshm->next[num-1] = 0;
			}
		}
		/* end of hash Ìî³ä */
	}
}

int searchnewuser(void)
{
	register int num, i;
	resolve_ucache();
	num = uidshm->number;
	for (i = 0; i < num; i++)
		if (uidshm->userid[i][0] == '\0')
			return i + 1;
	if (num < MAXUSERS)
		return (num + 1);
	return 0;
}

// Get 'userid' according to ('uid' - 1).
void getuserid(char *userid, int uid, size_t len)
{
	resolve_ucache();
	strlcpy(userid, uidshm->userid[uid - 1], len);
}

// Returns the place of 'userid' in cache for all users, 0 if not found.
int searchuser(const char *userid)
{
	register int i;
	int a1, a2;
	int key;

	resolve_ucache();
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
	resolve_ucache();
	*u = uidshm->passwd[uid - 1];
	return uid;
}

int get_status(int uid)
{
	resolve_ucache();
	if (!HAS_PERM(PERM_SEECLOAK)
			&& (uidshm->passwd[uid - 1].userlevel & PERM_LOGINCLOAK)
			&& (uidshm->passwd[uid - 1].flags[0] & CLOAK_FLAG))
		return 0;
	return uidshm->status[uid - 1];
}

// If 'utmpshm' == NULL, gets shared memory for online users
// and puts its starting address in utmpshm.
void resolve_utmp(void)
{
	if (utmpshm == NULL) {
		utmpshm = attach_shm("UTMP_SHMKEY", 3699, sizeof(*utmpshm));
		if (utmpshm == NULL)
			exit(1);
	}
}

// Returns (non-realtime) count of all users.
int allusers(void)
{
	resolve_ucache();
	return uidshm->number;
}

// Returns (non-realtime) count of online users.
int get_online(void)
{
	resolve_utmp();
	return utmpshm->total_num;
}

// Refreshes utmp(cache for online users.)
int refresh_utmp(void)
{
	int utmpfd, ucachefd;
	struct user_info *uentp;
	int n;
	int count = 0; // Online users count.
	time_t now;

	resolve_utmp();
	resolve_ucache();
	now = time(NULL);
	// Lock caches.
	utmpfd = utmp_lock();
	if (utmpfd == -1)
		return -1;
	ucachefd = ucache_lock();
	if (ucachefd == -1)
		return -1;

	memset(uidshm->status, 0, sizeof(uidshm->status));
	for (n = 0; n < USHM_SIZE; n++) {
		uentp = &(utmpshm->uinfo[n]);
		if (uentp->active && uentp->pid) {
			 // See if pid exists.
			if (bbskill(uentp->pid, 0) == -1) {
				memset(uentp, 0, sizeof(struct user_info));
				continue;
			} else {
				// Kick idle users out.
				if (uentp->mode != BBSNET
						&& now - uentp->idle_time > IDLE_TIMEOUT) {
					bbskill(uentp->pid, SIGHUP);
					memset(uentp, 0, sizeof(struct user_info));
				} else {
					// Increase status.
					uidshm->status[uentp->uid - 1]++;
					// Count online users.
					++count;
				}
			}
		}
	}
	utmpshm->total_num = count;
	// Get count of all users from ucache.
	utmpshm->usersum = uidshm->number;
	// Unlock caches.
	ucache_unlock(ucachefd);
	utmp_unlock(utmpfd);
	return count;
}

int getnewutmpent(struct user_info *up)
{
	int utmpfd, ucachefd;
	struct user_info *uentp;
	int i;

	resolve_utmp();
	resolve_ucache();

	utmpfd=utmp_lock();
	if (utmpfd == -1) {
		return -1;
	}

	if (utmpshm->max_login_num < get_online())
		utmpshm->max_login_num = get_online();
	for (i = 0; i < USHM_SIZE; i++) {
		uentp = &(utmpshm->uinfo[i]);
		if (!uentp->active || !uentp->pid)
			break;
	}
	if (i >= USHM_SIZE) {
		utmp_unlock(utmpfd);
		return -2;
	}
	utmpshm->uinfo[i] = *up;
	utmpshm->total_num++;

	utmp_unlock(utmpfd);

	ucachefd=ucache_lock();
	uidshm->status[up->uid-1]++;
	ucache_unlock(ucachefd);

	return i + 1;
}

int apply_ulist(int (*fptr)())
{
	struct user_info *uentp, utmp;
	int i, max;

	resolve_utmp();
	max = USHM_SIZE - 1;
	while (max > 0 && utmpshm->uinfo[max].active == 0)
		max--;
	for (i = 0; i <= max; i++) {
		uentp = &(utmpshm->uinfo[i]);
		utmp = *uentp;
		if ((*fptr)(&utmp) == QUIT)
			return QUIT;
	}
	return 0;
}

int search_ulist(struct user_info *uentp, int (*fptr)(), int farg)
{
	int i;
	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
		if ((*fptr) (farg, uentp))
		return i + 1;
	}
	return 0;
}

int search_ulistn(struct user_info *uentp, int (*fptr)(), int farg, int unum)
{
	int i, j;
	j = 1;
	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
		if ((*fptr) (farg, uentp)) {
			if (j == unum)
			return i + 1;
			else
			j++;
		}
	}
	return 0;
}

// Copies user_info *'uentp' to ('uent'th - 1) entry of utmp.
void update_ulist(struct user_info *uentp, int uent)
{
	resolve_utmp();
	if (uent > 0 && uent <= USHM_SIZE) {
		utmpshm->uinfo[uent - 1] = *uentp;
	}
}

/* added by djq 99.7.19*/
/* function added by douglas 990305
 set uentp to the user who is calling me
 solve the "one of 2 line call sb. to five" problem
 */

int who_callme(struct user_info *uentp, int (*fptr)(), int farg, int me)
{
	int i;

	resolve_utmp();
	for( i = 0; i < USHM_SIZE; i++ )
	{
		*uentp = utmpshm->uinfo[ i ];
		if( (*fptr)( farg, uentp ) && uentp->destuid==me )
		return i+1;
	}
	return 0;
}

