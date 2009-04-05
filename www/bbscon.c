#include "libweb.h"

static bool bbscon_search(struct boardheader *bp, unsigned int fid, struct fileheader *fp)
{
	if (bp == NULL)
		return false;
	char dir[HOMELEN];
	setwbdir(dir, bp->filename);
	void *ptr;
	size_t size;
	int fd;
	if (!safe_mmapfile(dir, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, &fd))
		return false;
	// Binary search.
	struct fileheader *begin = ptr, *end, *mid;
	end = begin + (size / sizeof(struct fileheader)) - 1;
	while (begin <= end) {
		mid = begin + (end - begin) / 2;
		if (mid->id == fid) {
			*fp = *mid;
			end_mmapfile(ptr, size, fd);
			return true;
		}
		if (mid->id < fid) {
			begin = mid + 1;
		} else {
			end = mid - 1;
		}
	}
	end_mmapfile(ptr, size, fd);
	return false;
}

int bbscon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal(HTTP_STATUS_BADREQUEST, "您选择的是一个目录");
	unsigned int fid = strtoul(getparm("f"), NULL, 10);
	struct fileheader fp;
	if (!bbscon_search(bp, fid, &fp))
		http_fatal(HTTP_STATUS_NOTFOUND, "错误的文章");
	return 0;
}
