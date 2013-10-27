#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"

struct BCACHE *brdshm = NULL;

int resolve_boards(void)
{
	if (!brdshm) {
		int iscreate = 0;
		brdshm = attach_shm2("BCACHE_SHMKEY", 3693, sizeof(*brdshm),
				&iscreate);
		if (!brdshm)
			return -1;
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

static int bcache_lock(void)
{
	int lockfd;
	lockfd = creat("bcache.lock", 0600);
	if (lockfd < 0) {
		report(strerror(errno), "");
		return -1;
	}
	file_lock_all(lockfd, FILE_WRLCK);
	return lockfd;
}

static void bcache_unlock(int fd)
{
	file_lock_all(fd, FILE_UNLCK);
	close(fd);
}

unsigned int get_nextid2(int bid)
{
	int fd = bcache_lock();
	int ret = ++(brdshm->bstatus[bid].nowid);
	bcache_unlock(fd);
	return ret;
}

int get_nextid(const char *bname)
{
	db_res_t *res = db_query("SELECT id FROM boards WHERE name = %s", bname);
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
