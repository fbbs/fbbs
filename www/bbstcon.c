#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

extern const struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid);

// If 'action' == 'n', return at most 'count' posts
// after 'fid' in thread 'gid', otherwise, posts before 'fid'.
// Return NULL if not found, otherwise, memory should be freed by caller.
static struct fileheader *bbstcon_search(const char *board, unsigned int *gid,
		unsigned int fid, char action, int *count, int *flag)
{
	if (!board || !count || *count < 1)
		return NULL;
	struct fileheader *fh = malloc(sizeof(struct fileheader) * (*count));
	if (!fh)
		return NULL;

	// Open index file.
	char dir[HOMELEN];
	setbfile(dir, board, DOT_DIR);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(dir, &m) < 0) {
		free(fh);
		return NULL;
	}

	struct fileheader *begin = m.ptr, *end;
	end = begin + (m.size / sizeof(*begin));
	const struct fileheader *f = dir_bsearch(begin, end, fid), *last = NULL;
	const struct fileheader *h;

	*flag = 0;
	if (action == 'a') {
		while (++f < end && f->id != f->gid)
			;
		if (f < end)
			*gid = f->id;
	}
	for (h = f; ++h < end && h->id != h->gid; )
		;
	if (h >= end)
		*flag |= THREAD_LAST;

	// find prev thread
	if (action == 'b') {
		while (--f >= begin && f->id != f->gid)
			;
		if (f >= begin)
			*gid = f->id;
	}
	for (h = f; --h >= begin && h->id != h->gid; )
		;
	if (h < begin)
		*flag |= THREAD_FIRST;

	int c = 0;
	if (action != 'p') {  // forward
		if (action == 'n' && f != end && f->id == fid)
			++f;	// skip current post.
		for (; f < end; ++f) {
			if (f->gid == *gid) {
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
		for (--f; f >= begin && f->id >= *gid; --f) {
			if (f->gid == *gid) {
				fh[--c] = *f;
				if (c == 0)
					break;
			}
		}
		*count -= c;
	}

	if (last) {
		while (++last < end && last->gid != *gid)
			;
	}
	if (!last || last >= end)
		*flag |= THREAD_LAST_POST;

	mmap_close(&m);
	return fh;
}

int bbstcon_main(void)
{
	board_t board;
	int bid = strtol(get_param("bid"), NULL, 10);
	if (bid <= 0) {
		get_board(get_param("board"), &board);
	} else {
		get_board_by_bid(bid, &board);
	}
	if (!board.id || !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	unsigned int gid = strtoul(get_param("g"), NULL, 10);
	unsigned int fid = strtoul(get_param("f"), NULL, 10);
	char action = *(get_param("a"));
	if (gid == 0)
		gid = fid;

	int count = POSTS_PER_PAGE;
	int c = count, flag = 0;
	struct fileheader *fh = bbstcon_search(board.name, &gid, fid, action, &c, &flag);
	if (!fh)
		return BBS_ENOFILE;
	if (action == 'a' || action == 'b')
		action = 'n';

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();

	struct fileheader *begin, *end;
	xml_header(NULL);
	printf("<bbstcon bid='%d' gid='%u' anony='%d' page='%d'"
			" attach='%d'%s%s%s%s%s>",
			bid, gid, anony, count, maxlen(board.name),
			flag & THREAD_LAST_POST ? " last='1'" : "",
			flag & THREAD_LAST ? " tlast='1'" : "",
			flag & THREAD_FIRST ? " tfirst='1'" : "",
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");
	print_session();

	bool isbm = am_bm(&board);
	if (action != 'p') {
		begin = fh;
		end = fh + c;
	} else {
		begin = fh + count - c;
		end = fh + count;
	}
	char file[HOMELEN];
	brc_fcgi_init(currentuser.userid, board.name);
	for (; begin != end; ++begin) {
		printf("<po fid='%u' owner='%s'%s>", begin->id, begin->owner,
				!isbm && begin->accessed[0] & FILE_NOREPLY ? " nore='1'" : "");
		setbfile(file, board.name, begin->filename);
		xml_print_file(file);
		puts("</po>");
		brc_addlist_legacy(begin->filename);
	}
	free(fh);
	puts("</bbstcon>");
	brc_update(currentuser.userid, board.name);
	return 0;
}

int web_sigopt(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	bool hidesig = streq(get_param("hidesig"), "on");
	bool hideimg = streq(get_param("hideimg"), "on");

	int flag = get_user_flag();

	if (hidesig)
		flag |= PREF_NOSIG;
	else
		flag &= ~PREF_NOSIG;
	if (hideimg)
		flag |= PREF_NOSIGIMG;
	else
		flag &= ~PREF_NOSIGIMG;

	set_user_flag(flag);

	html_header();
	printf("<title>success</title></head><body></body></html>");
	return 0;
}
