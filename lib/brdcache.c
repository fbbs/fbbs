#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"

struct BCACHE *brdshm = NULL;

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

// TODO:
int updatelastpost(const board_t *bp)
{
	return getlastpost(bp->name, &brdshm->bstatus[bp->id].lastpost,
			&brdshm->bstatus[bp->id].total);
}

int resolve_boards(void)
{
	if (!brdshm) {
		int iscreate = 0;
		brdshm = attach_shm2("BCACHE_SHMKEY", 3693, sizeof(*brdshm),
				&iscreate);
		if (!brdshm)
			return -1;
		if (iscreate)
			rebuild_brdshm();
	}

	time_t now = time(NULL);
	struct stat st;
	if (stat(BOARDS, &st) < 0) {
		st.st_mtime = now - 3600;
	}
	if (brdshm->uptime < st.st_mtime || brdshm->uptime < now - 3600) {
		brdshm->uptime = now;
	}
	return 0;
}

void rebuild_brdshm(void)
{
	db_res_t *res = db_exec_query(env.d, true, "SELECT id, name FROM boards");
	if (!res || db_res_rows(res) < 1) {
		db_clear(res);
		return;
	}
	for (int i = 0; i < db_res_rows(res); ++i) {
		int id = db_get_integer(res, i, 0);
		const char *name = db_get_value(res, i, 1);

		getlastpost(name, &brdshm->bstatus[id].lastpost,
				&brdshm->bstatus[id].total);

		char filename[STRLEN - 8];
		setbfile(filename, name, DOT_DIR);
		int count = get_num_records(filename, sizeof(struct fileheader));
		
		struct fileheader lastfh;
		get_record(filename, &lastfh, sizeof(struct fileheader), count - 1);
		brdshm->bstatus[id].nowid = lastfh.id + 1;
	}
	db_clear(res);
}

static int bcache_lock(void)
{
	int lockfd;
	lockfd = creat("bcache.lock", 0600);
	if (lockfd < 0) {
		report(strerror(errno), "");
		return -1;
	}
	fb_flock(lockfd, LOCK_EX);
	return lockfd;
}

static void bcache_unlock(int fd)
{
	fb_flock(fd, LOCK_UN);
	close(fd);
}

unsigned int get_nextid2(int bid)
{
	int fd = bcache_lock();
	int ret = ++(brdshm->bstatus[bid].nowid);
	bcache_unlock(fd);
	return ret;
}

int get_nextid(const char *boardname)
{
	db_res_t *res = db_exec_query(env.d, true, "SELECT id FROM boards"
			" WHERE name = %s", boardname);
	if (!res || db_res_rows(res) < 1) {
		db_clear(res);
		return 0;
	}
	int bid = db_get_integer(res, 0, 0);
	db_clear(res);

	return get_nextid2(bid);
}

struct bstat *getbstat(int bid)
{
	return &brdshm->bstatus[bid];
}

void bonlinesync(time_t now)
{
	int i;
	struct user_info *uentp;

	if (now - brdshm->inboarduptime < 300)
		return;
	brdshm->inboarduptime = now;

	for (i = 0; i < MAXBOARD; i++)
		brdshm->bstatus[i].inboard = 0;

	for (i = 0; i < USHM_SIZE; i++) {
		uentp = &(utmpshm->uinfo[i]);
		if (uentp->active && uentp->pid && uentp->currbrdnum)
			brdshm->bstatus[uentp->currbrdnum - 1].inboard++;
	}
}
