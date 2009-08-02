#include "libweb.h"

enum {
	POSTS_PER_PAGE = 20,
};

// If 'action' == 'n', return at most 'count' posts
// after 'fid' in thread 'gid', otherwise, posts before 'fid'.
// Return NULL if not found, otherwise, memory should be freed by caller.
static struct fileheader *bbstcon_search(const struct boardheader *bp,
		unsigned int gid, unsigned int fid, char action, int *count)
{
	if (bp == NULL || count == NULL || *count < 1)
		return NULL;
	struct fileheader *fh = malloc(sizeof(struct fileheader) * (*count));
	if (fh == NULL)
		return NULL;

	// Open index file.
	char dir[HOMELEN];
	setbfile(dir, bp->filename, DOT_DIR);
	void *ptr;
	size_t size;
	int fd = mmap_open(dir, MMAP_RDONLY, &ptr, &size);
	if (fd < 0) {
		free(fh);
		return NULL;
	}
	struct fileheader *begin = ptr, *end;
	end = begin + (size / sizeof(*begin));

	// Search 'fid'.
	const struct fileheader *f = dir_bsearch(begin, end, fid);
	int c = 0;
	if (action == 'n') {  // forward
		if (f != end && f->id == fid)
			++f;	// skip current post.
		for (; f < end; ++f) {
			if (f->gid == gid) {
				fh[c++] = *f;
				if (c == *count)
					break;
			}
		}
		*count = c;
	} else { // backward
		c = *count;
		for (--f; f >= begin && f->id >= gid; --f) {
			if (f->gid == gid) {
				fh[--c] = *f;
				if (c == 0)
					break;
			}
		}
		*count -= c;
	}
	mmap_close(ptr, size, fd);
	return fh;
}

// TODO: brc
int bbstcon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal("您选择的是一个目录");
	unsigned int gid = strtoul(getparm("g"), NULL, 10);
	unsigned int fid = strtoul(getparm("f"), NULL, 10);
	char action = *(getparm("a"));
	if (gid == 0) {
		gid = fid;
		fid--;
		action = 'n';
	}

	int count = POSTS_PER_PAGE;
	int c = count;
	struct fileheader *fh = bbstcon_search(bp, gid, fid, action, &c);
	if (fh == NULL)
		http_fatal2(HTTP_STATUS_INTERNAL_ERROR, "内部错误");
	struct fileheader *begin, *end;
	xml_header("bbstcon");
	printf("<bbstcon><bid>%d</bid><gid>%u</gid><page>%d</page>",
			bid, gid, count);
	if (action == 'n') {
		begin = fh;
		end = fh + c;
	} else {
		begin = fh + count - c;
		end = fh + count;
	}
	char file[HOMELEN];
	for (; begin != end; ++begin) {
		printf("<post><fid>%u</fid><author>%s</author><content>",
				begin->id, begin->owner);
		setbfile(file, bp->filename, begin->filename);
		xml_printfile(file, stdout);
		puts("</content></post>");
	}
	free(fh);
	puts("</bbstcon>");
	return 0;
}
