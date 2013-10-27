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

struct bstat *getbstat(int bid)
{
	return &brdshm->bstatus[bid];
}
