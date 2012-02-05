#include "libweb.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

static int cmp_fid(void *arg, void *buf)
{
	struct fileheader *fp = buf;
	unsigned int *fid = arg;
	return (fp->id == *fid);
}

// Find post whose id = 'fid'.
// If 'fid' > any post's id, return 'end',
// otherwise, return the minimum one among all post whose id > 'fid'.
const struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid)
{
	const struct fileheader *mid;
	while (begin < end) {
		mid = begin + (end - begin) / 2;
		if (mid->id == fid) {
			return mid;
		}
		if (mid->id < fid) {
			begin = mid + 1;
		} else {
			end = mid;
		}
	}
	return begin;
}

int bbscon_search(const char *board, unsigned int fid,
		int action, struct fileheader *fp, bool extra)
{
	if (!board || !fp)
		return -1;

	char dir[HOMELEN];
	setbfile(dir, board, DOT_DIR);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(dir, &m) < 0)
		return -1;

	struct fileheader *begin = m.ptr, *end;
	end = begin + (m.size / sizeof(*begin));
	const struct fileheader *f = dir_bsearch(begin, end, fid);

	if (f != end && f->id == fid) {
		unsigned int gid = f->gid;
		switch (action) {
			case 'p':  // previous post
				--f;
				break;
			case 'n':  // next post
				++f;
				break;
			case 'b':  // previous post of same thread
				while (--f >= begin && f->gid != gid)
					; // null statement
				break;
			case 'a':  // next post of same thread
				while (++f < end && f->gid != gid)
					; // null statement
				break;
			default:
				break;
		}
		if (f >= begin && f < end)
			*fp = *f;
		else
			f = NULL;
	}

	int ret = (f != NULL);
	if (f && extra) {
		if (f == begin)
			ret |= POST_FIRST;
		if (f == end - 1)
			ret |= POST_LAST;
		if (f->id == f->gid)
			ret |= THREAD_FIRST_POST;
		unsigned int gid = f->gid;
		while (++f < end && f->gid != gid)
			;
		if (f >= end)
			ret |= THREAD_LAST_POST;
	}

	mmap_close(&m);
	return ret;
}

int bbscon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	unsigned int fid = strtoul(get_param("f"), NULL, 10);
	const char *action = get_param("a");
	bool sticky = *get_param("s");

	int ret = 0;
	struct fileheader fh;
	if (sticky) {
		char file[HOMELEN];
		setbfile(file, board.name, NOTICE_DIR);
		if (!search_record(file, &fh, sizeof(fh), cmp_fid, &fid))
			return BBS_ENOFILE;
	} else {
		ret = bbscon_search(board.name, fid, *action, &fh, true);
		if (ret <= 0)
			return BBS_ENOFILE;
	}
	fid = fh.id;

	xml_header(NULL);

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();

	printf("<bbscon link='con' bid='%d' anony='%d' attach='%d'%s%s>",
			board.id, anony, maxlen(board.name),
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");

	print_session();

	bool isbm = am_bm(&board);
	bool noreply = fh.accessed[0] & FILE_NOREPLY && !isbm;
	bool self = streq(fh.owner, currentuser.userid);
	printf("<po fid='%u'%s%s%s%s%s%s", fid,
			sticky ? " sticky='1'" : "",
			ret & POST_FIRST ? " first='1'" : "",
			ret & POST_LAST ? " last='1'" : "",
			ret & THREAD_LAST_POST ? " tlast='1'" : "",
			noreply ? " nore='1'" : "",
			self || isbm ? " edit='1'" : "");
	if (fh.reid != fh.id)
		printf(" reid='%u' gid='%u'>", fh.reid, fh.gid);
	else
		printf(">");

	char file[HOMELEN];
	setbfile(file, board.name, fh.filename);
	xml_print_file(ctx.r, file);

	printf("</po></bbscon>");

	brc_fcgi_init(currentuser.userid, board.name);
	brc_addlist(fh.filename);
	brc_update(currentuser.userid, board.name);
	return 0;
}

int bbsgcon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	const char *f = get_param("f");
	if (strstr(f, "..") || strstr(f, "/") || strncmp(f, "G.", 2))
		return BBS_EINVAL;
	xml_header(NULL);
	printf("<bbscon link='gcon' bid='%d'>", board.id);
	print_session();
	printf("<po>");

	char file[HOMELEN];
	setbfile(file, board.name, f);
	xml_print_file(ctx.r, file);

	printf("</po></bbscon>");
	brc_fcgi_init(currentuser.userid, board.name);
	brc_addlist(f);
	brc_update(currentuser.userid, board.name);
	return 0;
}

int xml_print_file(http_req_t *r, const char *file)
{
	if (r->flag & REQUEST_PARSED) {
		int opt = PARSE_NOQUOTEIMG;
		int flag = get_user_flag();
		if (flag & PREF_NOSIG)
			opt |= PARSE_NOSIG;
		if (flag & PREF_NOSIGIMG)
			opt |= PARSE_NOSIGIMG;
		return xml_print_post(file, opt);
	}
	if (!(r->flag & REQUEST_MOBILE))
		return xml_printfile(file, stdout);
	return xml_print_post(file, PARSE_NOSIG);
}
