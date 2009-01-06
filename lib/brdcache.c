#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

struct BCACHE *brdshm = NULL;
struct boardheader *bcache = NULL;
int numboards = -1;
static int initlastpost = 0;

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

static void bcache_setreadonly(int readonly)
{
	int boardfd;
	void *oldptr = bcache;

	munmap((void *)bcache, MAXBOARD * sizeof(struct boardheader));
	if ((boardfd = open(BOARDS, O_RDWR | O_CREAT, 0644)) == -1) {
		report("Can't open " BOARDS "file %s", "");
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

static int bcache_lock(void)
{
	int lockfd;

	lockfd = creat("bcache.lock", 0600);
	if (lockfd < 0) {
		report(strerror(errno), "");
		return -1;
	}
	bcache_setreadonly(0);
	flock(lockfd, LOCK_EX);
	return lockfd;
}

static void bcache_unlock(int fd)
{
	flock(fd, LOCK_UN);
	bcache_setreadonly(1);
	close(fd);
}

static int getlastpost(const char *board, int *lastpost, int *total)
{
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

int updatelastpost(const char *board)
{
	int pos;

	pos = getbnum(board, &currentuser);
	if (pos > 0) {
		getlastpost(board, &brdshm->bstatus[pos - 1].lastpost,
				&brdshm->bstatus[pos - 1].total);
		return 0;
	} else
		return -1;
}

void resolve_boards(void)
{
	int boardfd=-1;
	struct stat st;
	time_t now;
	int iscreate=0;

	if (bcache == NULL) {
		if ((boardfd = open(BOARDS, O_RDWR | O_CREAT, 0644)) == -1) {
			report("Can't open " BOARDS "file", "");
			exit(-1);
		}
		bcache = (struct boardheader *) mmap(NULL, MAXBOARD
				* sizeof(struct boardheader), PROT_READ, MAP_SHARED,
				boardfd, 0);
		if (bcache == (struct boardheader *) -1) {
			report("Can't map " BOARDS "file ", "");
			close(boardfd);
			exit(-1);
		}
		close(boardfd);
	}
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
							sizeof(struct fileheader), count - 1);
					brdshm->bstatus[i].nowid = lastfh.id + 1;
					if (bcache[i].nowid > lastfh.id + 1)
						brdshm->bstatus[i].nowid = bcache[i].nowid;
					else
						brdshm->bstatus[i].nowid = lastfh.id + 1;
					maxi = i;
				}
			}
			if (maxi != -1)
				brdshm->number = maxi + 1;
			bcache_unlock(fd);
		}
		numboards = brdshm->number;
	}
	now = time(0);
	if (stat(BOARDS, &st) < 0) {
		st.st_mtime = now - 3600;
	}
	if (brdshm->uptime < st.st_mtime || brdshm->uptime < now - 3600) {
		brdshm->uptime = now;
	}
}

void flush_bcache(void)
{
	int i;
	resolve_boards();
	bcache_setreadonly(0);
	for (i = 0; i < MAXBOARD; i++)
		bcache[i].nowid = brdshm->bstatus[i].nowid;
	msync((void *)bcache, MAXBOARD * sizeof(struct boardheader), MS_SYNC);
	bcache_setreadonly(1);
}

void rebuild_brdshm(void)
{
	int i, maxi = -1;
	int fd;
	fd = bcache_lock();
	for (i = 0; i < MAXBOARD; i++) {
		if (bcache[i].filename[0]) {
			int count;
			char filename[STRLEN - 8];
			struct fileheader lastfh;
			getlastpost(bcache[i].filename, &brdshm->bstatus[i].lastpost,
					&brdshm->bstatus[i].total);
			setbfile(filename, bcache[i].filename, DOT_DIR);
			count = get_num_records(filename, sizeof(struct fileheader));
			get_record(filename, &lastfh, sizeof(struct fileheader), count
					-1);
			brdshm->bstatus[i].nowid = lastfh.id + 1;
			if (bcache[i].nowid > lastfh.id + 1)
				brdshm->bstatus[i].nowid = bcache[i].nowid;
			else
				brdshm->bstatus[i].nowid = lastfh.id + 1;
			maxi = i;
		}
	}
	if (maxi != -1)
		brdshm->number = maxi + 1;
	bcache_unlock(fd);
	numboards = brdshm->number;
}

int get_nextid(const char* boardname)
{
	register int i, ret;
	for (i = 0; i < numboards; i++) {
		if (!strncasecmp(boardname, bcache[i].filename, STRLEN)) {
			ret = i;
			int fd;
			fd = bcache_lock();
			brdshm->bstatus[i].nowid++;
			ret = brdshm->bstatus[i].nowid;
			bcache_unlock(fd);
			return ret;
		}
	}
	return 0;
}

int getblankbnum(void)
{
	int i;
	for (i = 0; i < MAXBOARD; i++) {
		if (!(bcache[i].filename[0]))
			return i + 1;
	}
	return 0;
}

//	返回版面的缓存
struct boardheader *getbcache(const char *bname)
{
	register int i;

	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (!strncasecmp(bname, bcache[i].filename, STRLEN))
			return &bcache[i];
	}
	return NULL;
}

struct bstat *getbstat(const char *bname)
{
	register int i;

	for (i = 0; i < numboards; i++) {
		if (!strncasecmp(bname, bcache[i].filename, STRLEN))
			return &brdshm->bstatus[i];
	}
	return NULL;
}

//	根据版名,返回其在bcache中的记录位置
int getbnum(const char *bname, const struct userec *cuser)
{
	register int i;

	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG //p限制版面
			|| HAS_PERM2(bcache[i].level, cuser)
		||(bcache[i].flag & BOARD_NOZAP_FLAG)) {//不可zap
			if (!strncasecmp(bname, bcache[i].filename, STRLEN)) //找到版名
				return i + 1;
		}
	}
	return 0;
}

int apply_boards(int (*func) (), const struct userec *cuser)
{
	register int i;
	resolve_boards();
	for (i = 0; i < numboards; i++) {
		if (bcache[i].flag & BOARD_POST_FLAG
			|| HAS_PERM2(bcache[i].level, cuser)
			|| (bcache[i].flag & BOARD_NOZAP_FLAG)) {
			if ((bcache[i].flag & BOARD_CLUB_FLAG)
				&& (bcache[i].flag	& BOARD_READ_FLAG)
				&& !chkBM(bcache + i, cuser)
				&& !isclubmember(cuser->userid, bcache[i].filename))
				continue;
			if ((*func)(&bcache[i]) == QUIT)
				return QUIT;
		}
	}
	return 0;
}

#ifdef NEWONLINECOUNT
void bonlinesync(time_t now)
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
