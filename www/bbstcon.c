#include "libweb.h"
#include "mmap.h"
#include "fbbs/web.h"

enum {
	POSTS_PER_PAGE = 20,
};

extern const struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid);

// If 'action' == 'n', return at most 'count' posts
// after 'fid' in thread 'gid', otherwise, posts before 'fid'.
// Return NULL if not found, otherwise, memory should be freed by caller.
static struct fileheader *bbstcon_search(const struct boardheader *bp,
		unsigned int gid, unsigned int fid, char action, int *count, int *flag)
{
	if (!bp || !count || *count < 1)
		return NULL;
	struct fileheader *fh = malloc(sizeof(struct fileheader) * (*count));
	if (!fh)
		return NULL;

	// Open index file.
	char dir[HOMELEN];
	setbfile(dir, bp->filename, DOT_DIR);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(dir, &m) < 0) {
		free(fh);
		return NULL;
	}

	struct fileheader *begin = m.ptr, *end;
	end = begin + (m.size / sizeof(*begin));
	const struct fileheader *f = dir_bsearch(begin, end, fid), *last = NULL;

	int c = 0;
	if (action == 'n') {  // forward
		if (f != end && f->id == fid)
			++f;	// skip current post.
		for (; f < end; ++f) {
			if (f->gid == gid) {
				fh[c++] = *f;
				if (c == *count) {
					last = f;
					break;
				}
			}
		}
		*count = c;
	} else { // backward
		last = f;
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

	*flag = 0;
	if (last) {
		while (++last < end && last->gid != gid)
			;
	}
	if (!last || last >= end)
		*flag |= THREAD_LAST;

	mmap_close(&m);
	return fh;
}

int bbstcon_main(web_ctx_t *ctx)
{
	int bid = strtol(get_param(ctx->r, "bid"), NULL, 10);
	struct boardheader *bp;
	if (bid <= 0) {
		bp = getbcache(get_param(ctx->r, "board"));
		bid = getbnum2(bp);
	} else {
		bp = getbcache2(bid);
	}
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	unsigned int gid = strtoul(get_param(ctx->r, "g"), NULL, 10);
	unsigned int fid = strtoul(get_param(ctx->r, "f"), NULL, 10);
	char action = *(get_param(ctx->r, "a"));
	if (gid == 0) {
		gid = fid;
		fid--;
		action = 'n';
	}

	int count = POSTS_PER_PAGE;
	int c = count, flag = 0;
	struct fileheader *fh = bbstcon_search(bp, gid, fid, action, &c, &flag);
	if (!fh)
		return BBS_ENOFILE;

	struct fileheader *begin, *end;
	xml_header(NULL);
	printf("<bbstcon bid='%d' gid='%u' page='%d'%s>", bid, gid, count,
			flag & THREAD_LAST ? " last='1'" : "");
	print_session(ctx);
	if (action == 'n') {
		begin = fh;
		end = fh + c;
	} else {
		begin = fh + count - c;
		end = fh + count;
	}
	char file[HOMELEN];
	brc_fcgi_init(currentuser.userid, bp->filename);
	for (; begin != end; ++begin) {
		printf("<po fid='%u' owner='%s'>", begin->id, begin->owner);
		setbfile(file, bp->filename, begin->filename);
		xml_print_file(ctx->r, file);
		puts("</po>");
		brc_addlist(begin->filename);
	}
	free(fh);
	puts("</bbstcon>");
	brc_update(currentuser.userid, bp->filename);
	return 0;
}
