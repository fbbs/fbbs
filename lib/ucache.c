// Handle user cache.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

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

static int usernumber = 0;

static int fillucache(struct userec *uentp)
{
	char a1,a2;
	int key;

	if (usernumber < MAXUSERS) {
		strlcpy(uidshm->userid[usernumber], uentp->userid, IDLEN + 1);
		uidshm->userid[usernumber++][IDLEN] = '\0';

		if(uentp->userid[0]) {
			/* hash Ìî³ä */
			key = uhashkey (uentp->userid, &a1, &a2);

			if( uidshm->hash[a1][a2][key] == 0 ) {
				uidshm->hash[a1][a2][key] = usernumber;
			} else {
				int i;
				for(i=uidshm->hash[a1][a2][key]; uidshm->next[i-1]; i=uidshm->next[i-1]);
				uidshm->next[i-1] = usernumber;
				uidshm->prev[usernumber-1] = i;
			}
			/* end of hash Ìî³ä */
		}
	}
	return 0;
}

/* hash É¾³ý */
int del_uidshm(int num, char *userid)
{
	char a1, a2;
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

static int shm_lock(char *lockname)
{
	int lockfd;

	lockfd = open(lockname, O_RDWR | O_CREAT, 0600);
	if (lockfd < 0) {
		//bbslog("3system", "CACHE:lock ucache:%s", strerror(errno));
		return -1;
	}
	flock(lockfd, LOCK_EX);
	return lockfd;
}

static void shm_unlock(int fd)
{
	flock(fd, LOCK_UN);
	close(fd);
}

#define ucache_unlock(fd) shm_unlock(fd)
#define utmp_unlock(fd) shm_unlock(fd)
#define ucache_lock() shm_lock("tmp/.UCACHE.lock")
#define utmp_lock() shm_lock("tmp/.UTMP.lock")

int load_ucache(int reload)
{
	int ftime;
	int fd;
	int iscreate=0;
	int passwdfd;
	int i;

	fd=ucache_lock();
	if (fd == -1) {
		return -1;
	}
	if (uidshm == NULL) {
		uidshm = attach_shm2("UCACHE_SHMKEY", 3696, sizeof(*uidshm),
				&iscreate);
	}

	log_usies("CACHE", "re load ucache", &currentuser);

	/* load PASSFILE */
	if ((passwdfd = open(PASSFILE, O_RDWR | O_CREAT, 0644)) == -1) {
		//bbslog("3system", "Can't open " PASSFILE "file %s", strerror(errno));
		ucache_unlock(fd);
		exit(-1);
	}
	ftruncate(passwdfd, MAXUSERS * sizeof(struct userec));
	close(passwdfd);
	if (get_records(PASSFILE, uidshm->passwd, sizeof(struct userec), 1,
			MAXUSERS) != MAXUSERS) {
		//bbslog("4system", "PASS file!");
		ucache_unlock(fd);
		return -1;
	}

	memset(uidshm->userid, 0, sizeof(uidshm->userid));
	/* ³õÊ¼?uhash */
	memset(uidshm->hash, 0, sizeof(uidshm->hash));
	memset(uidshm->next, 0, sizeof(uidshm->next));
	memset(uidshm->prev, 0, sizeof(uidshm->prev));
	/* endof ³õÊ¼»¯ */
	usernumber = 0;

	for (i=0; i<MAXUSERS; i++)
		fillucache(&(uidshm->passwd[i]));
	//apply_record(PASSFILE, fillucache, sizeof(struct userec), NULL, 0, 0);
	uidshm->number = usernumber;
	uidshm->uptime = ftime;

	ucache_unlock(fd);

	return 0;

}

int substitut_record(char *filename, char *rptr, int size, int id)
{
	memcpy(&(uidshm->passwd[id-1]), rptr, size);
}

int flush_ucache(void)
{
	int ret;
	ret= substitute_record(PASSFILE, uidshm->passwd, MAXUSERS
			* sizeof(struct userec), 1);
	return ret;
}

void resolve_ucache(void)
{
	//	struct stat st;
	//	int     ftime;
	int iscreate = 0;
	if (uidshm == NULL) {
		uidshm = attach_shm2("UCACHE_SHMKEY", 3696, sizeof(*uidshm),
				&iscreate);
	}
	if (iscreate) {
		remove_shm("UCACHE_SHMKEY", 3696, sizeof(*uidshm));
		sprintf(genbuf, "Error! miscd havn't startup\n");
		write(1, genbuf, strlen(genbuf));
		exit(1);
	}
	/*
	 if (stat(FLUSH, &st) < 0) {
	 ftime = time(0) - 86400;
	 } else  ftime = st.st_mtime;
	 if (uidshm->uptime < ftime) {
	 uidshm->uptime = ftime;
	 log_usies("CACHE", "reload ucache");
	 usernumber = 0;
	 memset(uidshm->hash, sizeof(uidshm->hash));
	 memset(uidshm->next, sizeof(uidshm->next));
	 memset(uidshm->prev, sizeof(uidshm->prev));
	 apply_record(PASSFILE, fillucache, sizeof(struct userec),0,0,0);
	 uidshm->number = usernumber;
	 //uidshm->uptime = ftime;
	 }
	 */
}

void setuserid(int num, char *userid)
{
	if (num > 0 && num <= MAXUSERS) {
		if (num > uidshm->number)
			uidshm->number = num;
		strlcpy(uidshm->userid[num - 1], userid, IDLEN + 1);
		/* hash Ìî³ä */
		if (strcmp(userid, "new") ) {
			char a1, a2;
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

void getuserid(char *userid, int uid)
{
	resolve_ucache();
	strcpy(userid, uidshm->userid[uid - 1]);
}

//Ê¹ÓÃhashº¯ÊýÀ´ËÑË÷ÓÃ»§
//		·µ»ØuseridÔÚ¹²ÏíÄÚ´æÖÐµÄÎ»ÖÃ
int searchuser(char *userid)
{
	register int i;
	char a1, a2;
	int key;
	resolve_ucache();
	if (0) { //²»Ö´ÐÐ´Ë¾ä
		for (i = 0; i < uidshm->number; i++)
			//µÍÐ§µÄÏßÐÔËÑË÷,¿ÉÒÔ¿¼ÂÇÌá¸ßÐ§ÂÊ
			if (!strncasecmp(userid, uidshm->userid[i], IDLEN + 1))
				return i + 1;
		return 0;
	} else {
		/* use hash */
		key = uhashkey(userid, &a1, &a2);
		i = uidshm->hash[a1][a2][key];
		while (i) {
			if (!strncasecmp(userid, uidshm->userid[i-1], IDLEN + 1)) {
				return i;
			}
			i = uidshm->next[i-1];
		}
		return 0;
		/* endof new */
	}
}

int getuserec(char *userid, struct userec *u)
{
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(u, &(uidshm->passwd[uid-1]), sizeof(struct userec));
	return uid;
}

int getuser(char * userid)
{
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(&lookupuser, &(uidshm->passwd[uid-1]), sizeof(lookupuser));
	return uid;
}

int getuserbyuid(struct userec *u, int uid)
{
	memcpy(u, &(uidshm->passwd[uid-1]), sizeof(struct userec));
	return uid;
}

char *u_namearray(char buf[][IDLEN + 1], int *pnum, char *tag)
{
	register struct UCACHE *reg_ushm = uidshm;
	register char *ptr, tmp;
	register int n, total;
	char tagbuf[STRLEN];
	int ch, num = 0;
	resolve_ucache();
	if (*tag == '\0') {
		*pnum = reg_ushm->number;
		return reg_ushm->userid[0];
	}
	for (n = 0; tag[n] != '\0'; n++) {
		tagbuf[n] = chartoupper(tag[n]);
	}
	tagbuf[n] = '\0';
	ch = tagbuf[0];
	total = reg_ushm->number;
	for (n = 0; n < total; n++) {
		ptr = reg_ushm->userid[n];
		tmp = *ptr;
		if (tmp == ch || tmp == ch - 'A' + 'a')
		if (chkstr(tag, tagbuf, ptr))
		strcpy(buf[num++], ptr);
	}
	*pnum = num;
	return buf[0];
}

//Ó³ÉäÓÃ»§Êý¾Ýµ½ÄÚ´æ
void resolve_utmp(void)
{
	if (utmpshm == NULL) {
		utmpshm = attach_shm("UTMP_SHMKEY", 3699, sizeof(*utmpshm));
	}
}

int get_total(void)
{
	resolve_utmp();
	return utmpshm->total_num;
}

int get_status(int uid)
{
	resolve_ucache();
	if ( !HAS_PERM(PERM_SEECLOAK) && //µ±Ç°Ê¹ÓÃÕßÃ»ÓÐÕ¾Îñ»ò¿´ÒþÉíÈ¨ÏÞ
			(uidshm->passwd[uid-1].userlevel&PERM_LOGINCLOAK) && //Ëù²ì¿´µÄÓÃ»§ÓµÓÐÒþÉíÈ¨ÏÞ
			(uidshm->passwd[uid-1].flags[0] & CLOAK_FLAG)//Ëù²ì¿´µÄÓÃ»§¿ªÆôÁËÒþÉíÈ¨ÏÞ
	)
		return 0;
	return uidshm->status[uid-1];
}

void refresh_utmp(void)
{
	int utmpfd, ucachefd;
	struct user_info *uentp;
	register int n;
	time_t now;
	resolve_utmp();

	now = time(0);
	utmpfd=utmp_lock();
	ucachefd=ucache_lock();

	memset(uidshm->status, 0, sizeof(int)*MAXUSERS);
	for (n = 0; n < USHM_SIZE; n++) {
		uentp = &(utmpshm->uinfo[n]);
		if (uentp->active && uentp->pid) {
			if (kill(uentp->pid, 0) == -1) {
				memset(uentp, 0, sizeof(struct user_info));
				continue;
			} else if (uentp->mode!=BBSNET && now - uentp->idle_time > 30
					*60) {
				kill(uentp->pid, SIGHUP);
				memset(uentp, 0, sizeof(struct user_info));
			} else {
				uidshm->status[uentp->uid-1]++;
			}
		}
	}
	utmpshm->usersum = allusers();

	ucache_unlock(ucachefd);
	utmp_unlock(utmpfd);

	//refresh_anoncache();

	//utmpshm->total_num=anonshm->used+num_active_users();
	utmpshm->total_num=num_active_users();
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

	if (utmpshm->max_login_num < count_users)
		utmpshm->max_login_num = count_users;
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

int t_search_ulist(struct user_info *uentp, int (*fptr) (), int farg, int show, int doTalk)
{
	int i, num;
	char col[14];

	resolve_utmp();
	num = 0;
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
		if ((*fptr)(farg, uentp)) {
			if (!uentp->active || !uentp->pid || isreject(uentp)) {
				continue;
			}
			if ( (uentp->invisible==0) ||(uentp->uid == usernum)
					||(uentp->invisible==1) &&HAS_PERM(PERM_SEECLOAK) ) {
				num++;
			} else {
				continue;
			}
			if (!show)
				continue;
			if (num == 1)
				prints("Ä¿Ç° %s ×´Ì¬ÈçÏÂ£º\n", uentp->userid);
			if (uentp->invisible)
				strcpy(col, "[Òþ][1;36m");
			else if (uentp->mode == POSTING || uentp->mode == MARKET)
				strcpy(col, "[1;32m");
			else if (uentp->mode == FIVE || uentp->mode == BBSNET)
				strcpy(col, "[1;33m");
			else
				strcpy(col, "[1m");
			if (doTalk) {
				prints(
						"(%d) ×´Ì¬£º%s%-10s[m£¬À´×Ô£º%.20s\n",
						num,
						col,
						ModeType(uentp->mode),
#ifdef SHOWMETOFRIEND		    
						/* The following line is modified by Amigo 2002.04.02. Let sysop view fromhost at »·¹ËËÄ·½. */
						((uentp->from[22] != 'H')||hisfriend(uentp)
								||HAS_PERM(PERM_USER))?uentp->from:"......"
						);
#else
						/* The following line is modified by Amigo 2002.04.02. Let sysop view fromhost at »·¹ËËÄ·½. */
						((uentp->from[22] != 'H')|| HAS_PERM(PERM_USER)) ? uentp->from
								: "......");
#endif
			} else {
				prints("%s%-10s[m ", col, ModeType(uentp->mode));
				if ((num) % 5 == 0)
					outc('\n');
			}
		}
	}
	if (show)
		outc('\n');
	return num;
}

//¸ü¸ÄµÚuent¸öÓÃ»§µÄÐÅÏ¢,½«ÆäÉèÖÃÎªuentp
void update_ulist(struct user_info *uentp, int uent)
{
	resolve_utmp();
	if (uent > 0 && uent <= USHM_SIZE) {
		utmpshm->uinfo[uent - 1] = *uentp;
	}
}

void update_utmp(void)
{
	update_ulist(&uinfo, utmpent);
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

