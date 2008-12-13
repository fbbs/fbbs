/*
 Pirate Bulletin Board System
 Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
 Eagles Bulletin Board System
 Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
 Guy Vega, gtvega@seabass.st.usm.edu
 Dominic Tynes, dbtynes@seabass.st.usm.edu
 Firebird Bulletin Board System
 Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
 Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */
/*
 $Id: bcache.c 367 2007-05-12 17:08:28Z danielfree $
 */

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

struct BCACHE *brdshm=NULL;
struct UCACHE *uidshm=NULL;
struct UTMPFILE *utmpshm=NULL;
struct userec lookupuser;
struct boardheader *bcache=NULL;
int usernumber=0;
int numboards = -1;

//ANONCACHE *anonshm=NULL;

void attach_err(int shmkey, char* name, int err) {
	sprintf(genbuf, "Error! %s error #%d! key = %x.\n", name, err, shmkey);
	write(1, genbuf, strlen(genbuf));
	exit(1);
}

int search_shmkey(char *keyname) {
	int i = 0, found = 0;
	while (shmkeys[i].key != NULL) {
		if (strcmp(shmkeys[i].key, keyname) == 0) {
			found = shmkeys[i].value;
			break;
		}
		i++;
	}
	if (found == 0) {
		sprintf(genbuf, "search_shmkey(): cannot found %s SHMKEY entry!",
				keyname);
		report(genbuf);
	}
	return found;
}

void * attach_shm(char *shmstr, int defaultkey, int shmsize) {
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

void *attach_shm2(shmstr, defaultkey, shmsize, iscreate)
char *shmstr;
int defaultkey, shmsize;
int *iscreate;
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

void remove_shm(char *shmstr, int defaultkey, int shmsize) {
	int shmkey, shmid;

	if (shmstr)
		shmkey = sysconf_eval(shmstr, defaultkey);

	else
		shmkey = 0;
	if (shmkey < 1024)
		shmkey = defaultkey;
	shmid = shmget(shmkey, shmsize, 0);
	shmctl(shmid, IPC_RMID, NULL);
}
//from kbs 2.0
static void bcache_setreadonly(int readonly) {
	int boardfd;
	void *oldptr = bcache;
	munmap((void *)bcache, MAXBOARD * sizeof(struct boardheader));
	if ((boardfd = open(BOARDS, O_RDWR | O_CREAT, 0644)) == -1) {
		report("Can't open " BOARDS "file %s");
		exit(-1);
	}
	if (readonly)
		bcache = (struct boardheader *) mmap(oldptr, MAXBOARD
				* sizeof(struct boardheader), PROT_READ, MAP_SHARED,
				boardfd, 0);
	else
		bcache = (struct boardheader *) mmap(oldptr, MAXBOARD
				* sizeof(struct boardheader), PROT_READ | PROT_WRITE,
				MAP_SHARED, boardfd, 0);
	close(boardfd);
}
static int bcache_lock() {
	int lockfd;

	lockfd = creat("bcache.lock", 0600);
	if (lockfd < 0) {
		report("CACHE:lock bcache:%s", strerror(errno));
		return -1;
	}
	bcache_setreadonly(0);
	flock(lockfd, LOCK_EX);
	return lockfd;
}
static void bcache_unlock(int fd) {
	flock(fd, LOCK_UN);
	bcache_setreadonly(1);
	close(fd);
}
//add end

int getlastpost(char *board, int *lastpost, int *total) {
	struct fileheader fh;
	struct stat st;
	char filename[STRLEN * 2];
	int fd, atotal, offset;

	sprintf(filename, "boards/%s/" DOT_DIR, board);
	if ((fd = open(filename, O_RDONLY)) < 0)
		return 0;
	fstat(fd, &st);
	atotal = st.st_size / sizeof(fh);
	if (atotal <= 0) {
		*lastpost = 0;
		*total = 0;
		close(fd);
		return 0;
	}
	*total = atotal;
	offset = (int) ((char *) &(fh.filename[0]) - (char *) &(fh));
	lseek(fd, (off_t) (offset + (atotal - 1) * sizeof(fh)), SEEK_SET);
	if (read(fd, filename, STRLEN) > 2) {
		*lastpost = atoi(filename+2);
	} else {
		*lastpost = 0;
	}
	close(fd);
	return 0;
}

int updatelastpost(char *board) {
	//struct boardheader *bh;

	//bh = getbcache(board);
	//getlastpost(board, &(bh->lastpost), &(bh->total));
	//return 0;
	int pos;

	pos = getbnum(board); /* board name --> board No. */
	if (pos > 0) {
		getlastpost(board, &brdshm->bstatus[pos - 1].lastpost,
				&brdshm->bstatus[pos - 1].total);
		return 0;
	} else
		return -1;
}

#ifdef NEWONLINECOUNT
void
bonlinesync(time_t now)
{
	int i;
	struct user_info *uentp;

	if (now - brdshm->inboarduptime < 300)
	return;
	brdshm->inboarduptime = now;

	for (i = 0; i < numboards; i++)
	brdshm->bstatus[i].inboard = 0;

	for (i = 0; i < USHM_SIZE; i++) {
		uentp = &(utmpshm->uinfo[i]);
		if (uentp->active && uentp->pid && uentp->currbrdnum)
		brdshm->bstatus[uentp->currbrdnum - 1].inboard++;
	}
}
#endif

int initlastpost=0;
//remove by eefree 06.04.28
//int	fillbcache(struct boardheader *fptr,int index,void *arg)
//{
//	struct boardheader *bptr;
//	if (numboards >= MAXBOARD)
//		return 0;
//	bptr = &bcache[numboards++];
//#ifdef NEWONLINECOUNT
//	memcpy(bptr, fptr, sizeof(struct boardheader) - 3*sizeof(int));
//#else
//	memcpy(bptr, fptr, sizeof(struct boardheader) - 2*sizeof(int));
//#endif
//    if(initlastpost)
//        getlastpost(bptr->filename, &(bptr->lastpost), &(bptr->total) );
//	return 0;
//}

void resolve_boards() {
	int boardfd=-1;
	struct stat st;
	time_t now;
	int iscreate=0;
	//from kds2.0
	if (bcache == NULL) {
		if ((boardfd = open(BOARDS, O_RDWR | O_CREAT, 0644)) == -1) {
			report("Can't open " BOARDS "file");
			exit(-1);
		}
		bcache = (struct boardheader *) mmap(NULL, MAXBOARD
				* sizeof(struct boardheader), PROT_READ, MAP_SHARED,
				boardfd, 0);
		if (bcache == (struct boardheader *) -1) {
			report("Can't map " BOARDS "file ");
			close(boardfd);
			exit(-1);
		}
		close(boardfd);
	}
	//add end
	if (brdshm == NULL) {
		brdshm = attach_shm2("BCACHE_SHMKEY", 3693, sizeof(*brdshm),
				&iscreate);
		if (iscreate) {
			initlastpost=1;
			int i, maxi = -1;
			int fd;
			fd = bcache_lock();
			for (i = 0; i < MAXBOARD; i++) {
				if (bcache[i].filename[0]) {
					int count;
					char filename[STRLEN-8];
					struct fileheader lastfh;
					getlastpost(bcache[i].filename,
							&brdshm->bstatus[i].lastpost,
							&brdshm->bstatus[i].total);
					setbfile(filename, bcache[i].filename, DOT_DIR);
					count=get_num_records(filename,
							sizeof(struct fileheader));
					get_record(filename, &lastfh,
							sizeof(struct fileheader), count-1);
					brdshm->bstatus[i].nowid=lastfh.id+1;
					if (bcache[i].nowid>lastfh.id+1)
						brdshm->bstatus[i].nowid=bcache[i].nowid;
					else
						brdshm->bstatus[i].nowid=lastfh.id+1;
					maxi = i;
				}
			}//end for
			if (maxi != -1)
				brdshm->number = maxi + 1;
			bcache_unlock(fd);
		}//end if(creat)
		numboards = brdshm->number;
	}//end if brdshm=NULL
	now = time(0);
	if (stat(BOARDS, &st) < 0) {
		st.st_mtime = now - 3600;
	}
	if (brdshm->uptime < st.st_mtime || brdshm->uptime < now - 3600) {
		brdshm->uptime=now;
		//remove by eefree 06.04.28
		//log_usies("CACHE", "reload bcache");
		//numboards = 0;
		//apply_record(BOARDS, fillbcache, sizeof(struct boardheader), NULL, 0, 0);
		//brdshm->number = numboards;
		//if(initlastpost)
		//    initlastpost=0;
	}
}
//add from kbs2.0
void detach_boards() {
	munmap((void *)bcache, MAXBOARD * sizeof(struct boardheader));
	bcache=NULL;
	shmdt((void *)brdshm);
	brdshm=NULL;
}
void flush_bcache() {
	int i;
	resolve_boards();
	bcache_setreadonly(0);
	for (i = 0; i < MAXBOARD; i++)
		bcache[i].nowid=brdshm->bstatus[i].nowid;
	msync((void *)bcache, MAXBOARD * sizeof(struct boardheader), MS_SYNC);
	bcache_setreadonly(1);
}
//add end
//added by cometcaptor 2006-10-13 a brdshm patch (temp)
void rebuild_brdshm() {
	int i, maxi = -1;
	int fd;
	fd = bcache_lock();
	for (i = 0; i < MAXBOARD; i++) {
		if (bcache[i].filename[0]) {
			int count;
			char filename[STRLEN-8];
			struct fileheader lastfh;
			getlastpost(bcache[i].filename, &brdshm->bstatus[i].lastpost,
					&brdshm->bstatus[i].total);
			setbfile(filename, bcache[i].filename, DOT_DIR);
			count=get_num_records(filename, sizeof(struct fileheader));
			get_record(filename, &lastfh, sizeof(struct fileheader), count
					-1);
			brdshm->bstatus[i].nowid=lastfh.id+1;
			if (bcache[i].nowid>lastfh.id+1)
				brdshm->bstatus[i].nowid=bcache[i].nowid;
			else
				brdshm->bstatus[i].nowid=lastfh.id+1;
			maxi = i;
		}
	}//end for
	if (maxi != -1)
		brdshm->number = maxi + 1;
	bcache_unlock(fd);
	numboards = brdshm->number;
}
int apply_boards(int (*func) ()) {
	register int i;
	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG || HAS_PERM(bcache[i].level)
				|| (bcache[i].flag & BOARD_NOZAP_FLAG)) {
			if ((bcache[i].flag & BOARD_CLUB_FLAG)&& (bcache[i].flag
					& BOARD_READ_FLAG )&& !chk_currBM(bcache[i].BM, 1)
					&& !isclubmember(currentuser.userid,
							bcache[i].filename))
				continue;
			if ((*func)(&bcache[i]) == QUIT)
				return QUIT;
		} //if
	}
	return 0;
}

//	·µ»Ø°æÃæµÄ»º´æ
struct boardheader * getbcache(char *bname) {
	register int i;
	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (!strncasecmp(bname, bcache[i].filename, STRLEN))
			return &bcache[i];
	}
	return NULL;
}
struct bstat *getbstat(char *bname) {
	register int i;
	for (i = 0; i < numboards; i++) {
		if (!strncasecmp(bname, bcache[i].filename, STRLEN))
			return &brdshm->bstatus[i];
	}
	return NULL;
}
//	¸ù¾Ý°æÃû,·µ»ØÆäÔÚbcacheÖÐµÄ¼ÇÂ¼Î»ÖÃ
int getbnum(char *bname) {
	register int i;
	resolve_boards();

	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG //pÏÞÖÆ°æÃæ
				|| HAS_PERM(bcache[i].level) //È¨ÏÞ×ã¹»
		||(bcache[i].flag & BOARD_NOZAP_FLAG)) {//²»¿Ézap
			if (!strncasecmp(bname, bcache[i].filename, STRLEN)) //ÕÒµ½°æÃû
				return i + 1;
		}
	}
	return 0;
}
int getblankbnum() {
	int i;
	for (i=0; i<MAXBOARD; i++) {
		if (!(bcache[i].filename[0]))
			return i+1;
	}
	return 0;
}

void setoboard(char *bname) {
	register int i;

	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG || HAS_PERM(bcache[i].level)
				|| (bcache[i].flag & BOARD_NOZAP_FLAG)) {
			if (bcache[i].filename[0] != 0 && bcache[i].filename[0] != ' ') {
				strcpy(bname, bcache[i].filename);
				return;
			}
		}
	}
}

/* added by money for check user permission on announce reading 04.02.12 */
int
hasreadperm(bname)
char *bname;
{
	struct boardheader *x;

	x = getbcache(bname);
	if (x == 0) return 0;
	if (x->level == 0 ) return 1;
	if (x->flag & (BOARD_POST_FLAG | BOARD_NOZAP_FLAG)) return 1;
	if (currentuser.userlevel & x->level) return 1;
	return 0;
}
/* added end */

int haspostperm(char *bname) {
	register int i;
	/*	if (digestmode==TRASH_MODE||digestmode==JUNK_MODE){
	 return 0;
	 }
	 */
	if (deny_me(bname)) {
		return 0;
	}

	/*
	 if (strcmp(bname, DEFAULTBOARD) == 0)
	 return 1;
	 *///added by roly 02.01.27 disable postperm in sysop of that has no perm_post
	if ((i = getbnum(bname)) == 0) {
		return 0;
	}
	set_safe_record();
	if (!HAS_PERM(PERM_POST))
		return 0;
	return HAS_PERM(bcache[i - 1].level);
}

int
normal_board(bname)
char *bname;
{
	register int i;
	if (strcmp(bname, DEFAULTBOARD) == 0)
	return 1;
	if ((i = getbnum(bname)) == 0)//°æÃæ²»¿É¼û
	return 0;
	if (bcache[i - 1].flag & BOARD_NOZAP_FLAG)
	return 1;
	if ((bcache[i - 1].flag & BOARD_POST_FLAG) || bcache[i - 1].level)
	return 0;
	return 1;
}

/* add by stiger, µÃµ½hashkey */
int uhashkey(char *userid, char *a1, char *a2) {
	char *c=userid;
	int key=0;

	if ( *c >= 'a' && *c <= 'z') {
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
	while ( *c) {
		key += toupper(*c);
		c++;
	}
	return key%256;
}
/* hashkey ¼ÆËã over */

int apply_users(int (*fptr) (struct userec *)) {
	int i;
	int count;

	count = 0;
	for (i = 0; i < uidshm->number; i++)
		if (fptr) {
			int ret;

			ret = (*fptr)(&uidshm->passwd[i]);
			if (ret == -1)
				break;
			if (ret == 1)
				count++;
		} else
			count++;
	return count;
}

int
fillucache(uentp,index,arg)
struct userec *uentp;
int index;
void *arg;
{
	char a1,a2;
	int key;

	if (usernumber < MAXUSERS) {
		strncpy(uidshm->userid[usernumber], uentp->userid, IDLEN + 1);
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
int del_uidshm(int num, char *userid) {
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

static int shm_lock(char *lockname) {
	int lockfd;

	lockfd = open(lockname, O_RDWR | O_CREAT, 0600);
	if (lockfd < 0) {
		//bbslog("3system", "CACHE:lock ucache:%s", strerror(errno));
		return -1;
	}
	flock(lockfd, LOCK_EX);
	return lockfd;
}

static void shm_unlock(int fd) {
	flock(fd, LOCK_UN);
	close(fd);
}

#define ucache_unlock(fd) shm_unlock(fd)
#define utmp_unlock(fd) shm_unlock(fd)
#define ucache_lock() shm_lock("tmp/.UCACHE.lock")
#define utmp_lock() shm_lock("tmp/.UTMP.lock")

int load_ucache(int reload) {
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

	log_usies("CACHE", "re load ucache");

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

int substitut_record(char *filename, char *rptr, int size, int id) {
	memcpy(&(uidshm->passwd[id-1]), rptr, size);
}

int flush_ucache() {
	int ret;
	ret= substitute_record(PASSFILE, uidshm->passwd, MAXUSERS
			* sizeof(struct userec), 1);
	return ret;
}

void resolve_ucache() {
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

void setuserid(int num, char *userid) {
	if (num > 0 && num <= MAXUSERS) {
		if (num > uidshm->number)
			uidshm->number = num;
		strncpy(uidshm->userid[num - 1], userid, IDLEN + 1);
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

int searchnewuser() {
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

void
getuserid(userid, uid)
char *userid;
int uid;
{
	resolve_ucache();
	strcpy(userid, uidshm->userid[uid - 1]);
}
//Ê¹ÓÃhashº¯ÊýÀ´ËÑË÷ÓÃ»§
//		·µ»ØuseridÔÚ¹²ÏíÄÚ´æÖÐµÄÎ»ÖÃ
int searchuser(char *userid) {
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
/*
 int
 apply_users(func)
 void (*func)() ;
 {
 register int i ;
 resolve_ucache() ;
 for(i=0; i < uidshm->number; i++)
 (*func)(uidshm->userid[i],i+1) ;
 return 0 ;
 }
 */

int getuserec(char *userid, struct userec *u) {
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(u, &(uidshm->passwd[uid-1]), sizeof(struct userec));
	return uid;
}

//  È¡µÃÓÃ»§IDÎªuseridµÄÓÃ»§ÐÅÏ¢,±£´æÔÚcurrentuserÖÐ
int getcurrentuser(char *userid) {
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(&currentuser, &(uidshm->passwd[uid-1]), sizeof(currentuser));
	return uid;
}

int getuser(char * userid) {
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(&lookupuser, &(uidshm->passwd[uid-1]), sizeof(lookupuser));
	return uid;
}

int getuserbyuid(struct userec *u, int uid) {
	memcpy(u, &(uidshm->passwd[uid-1]), sizeof(struct userec));
	return uid;
}

char * u_namearray(buf, pnum, tag)
char buf[][IDLEN + 1], *tag;
int *pnum;
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
void resolve_utmp() {
	if (utmpshm == NULL) {
		utmpshm = attach_shm("UTMP_SHMKEY", 3699, sizeof(*utmpshm));
	}
}
int get_nextid(char* boardname) {
	register int i, ret;
	for (i = 0; i < numboards; i++) {
		if (!strncasecmp(boardname, bcache[i].filename, STRLEN)) { //ÕÒµ½°æÃû
			ret = i;
			int fd;
			fd = bcache_lock();
			brdshm->bstatus[i].nowid++;
			ret=brdshm->bstatus[i].nowid;
			bcache_unlock(fd);
			return ret;
		}
	}
	return 0;
}

int get_nextid_bid(int bid) {
	int ret;
	ret = bid;
	if (ret > 0) {
		int fd;
		fd = bcache_lock();
		brdshm->bstatus[bid-1].nowid++;
		ret=brdshm->bstatus[bid-1].nowid;
		bcache_unlock(fd);
		//substitute_record (BOARDS, &bcache[bid-1], sizeof (struct boardheader), bid);
	}

	return ret;
}

int get_total() {
	resolve_utmp();
	return utmpshm->total_num;
}

int get_status(int uid) {
	resolve_ucache();
	if ( !HAS_PERM(PERM_SEECLOAK) && //µ±Ç°Ê¹ÓÃÕßÃ»ÓÐÕ¾Îñ»ò¿´ÒþÉíÈ¨ÏÞ
			(uidshm->passwd[uid-1].userlevel&PERM_LOGINCLOAK) && //Ëù²ì¿´µÄÓÃ»§ÓµÓÐÒþÉíÈ¨ÏÞ
			(uidshm->passwd[uid-1].flags[0] & CLOAK_FLAG)//Ëù²ì¿´µÄÓÃ»§¿ªÆôÁËÒþÉíÈ¨ÏÞ
	)
		return 0;
	return uidshm->status[uid-1];
}

//int load_anoncache(int reload)
//{
//	int acfd;
//	int iscreate=0;
//	int i;
//
//	acfd=shm_lock("tmp/ACACHE.lock");
//
//	if (anonshm == NULL) {
//      	anonshm = attach_shm2("ACACHE_SHMKEY", 3700, sizeof(ANONCACHE), &iscreate);
//	}
//	log_usies("CACHE", "load anonshm");
//	memset(anonshm,0, sizeof(ANONCACHE));
//	anonshm->freenode=1;
//	for(i=0;i<MAX_ANON-1;i++){
//		anonshm->next[i]=i+2;
//	}
//	shm_unlock(acfd);
//
//	return 0;

//}

//void resolve_anoncache()
//{
//	int iscreate=0;
//	
//	if (anonshm == NULL) {
//		anonshm = attach_shm2("ACACHE_SHMKEY", 3700, sizeof(ANONCACHE), &iscreate);
//	}
//	if (iscreate) {
//      	remove_shm("ACACHE_SHMKEY", 3700, sizeof(ANONCACHE));
//	        sprintf(genbuf, "Error! miscd havn't startup\n");
//        	write(1, genbuf, strlen(genbuf));
//	        exit(1);
//	}
//}

//void refresh_anoncache()
//{
//	int i,j=0,k,acfd;
//	time_t now=time(0);
//	resolve_anoncache();
//	acfd=shm_lock("tmp/ACACHE.lock");
//	i=anonshm->usednode;
//	while(i){
//		if(anonshm->item[i-1]+600<now){
//			anonshm->item[i-1]=0;
//			k=anonshm->next[i-1];
//			if(j){
//				anonshm->next[j-1]=k;
//			}else{
//				anonshm->usednode=k;
//			}
//			anonshm->next[i-1]=anonshm->freenode;
//			anonshm->freenode=i;
//			i=k;
//			anonshm->used--;
//			continue;
//		}
//		j=i;
//		i=anonshm->next[i-1];
//	}
//	shm_unlock(acfd);
//}

//int get_anon()
//{
//	resolve_anoncache();
//	return anonshm->used;
//}

void refresh_utmp() {
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

int getnewutmpent(struct user_info *up) {
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

int apply_ulist(int (*fptr)()) {
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

int
apply_ulist_address(fptr)
int (*fptr) ();
{
	int i, max;
	resolve_utmp();
	max = USHM_SIZE - 1;
	while (max> 0 && utmpshm->uinfo[max].active == 0)
	max--;
	for (i = 0; i <= max; i++) {
		if ((*fptr) (&(utmpshm->uinfo[i])) == QUIT)
		return QUIT;
	}
	return 0;
}

int
search_ulist(uentp, fptr, farg)
struct user_info *uentp;
int (*fptr)();
int farg;
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

int
search_ulistn(uentp, fptr, farg, unum)
struct user_info *uentp;
int (*fptr)();
int farg;
int unum;
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

int t_search_ulist(struct user_info *uentp, int (*fptr) (), int farg, int show, int doTalk) {
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
void update_ulist(struct user_info *uentp, int uent) {
	resolve_utmp();
	if (uent > 0 && uent <= USHM_SIZE) {
		utmpshm->uinfo[uent - 1] = *uentp;
	}
}

void update_utmp() {
	update_ulist(&uinfo, utmpent);
}

/* added by djq 99.7.19*/
/* function added by douglas 990305
 set uentp to the user who is calling me
 solve the "one of 2 line call sb. to five" problem
 */

int
who_callme( uentp, fptr, farg, me )
struct user_info *uentp;
int (*fptr)();
int farg;
int me;
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
