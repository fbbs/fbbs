#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

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

int bbscon_search(int bid, post_id_t pid, post_id_t tid, int action,
		bool extra, post_info_full_t *p)
{
	char op = '=';
	switch (action) {
		case 'p':
		case 'b':
			op = '<';
			break;
		case 'n':
		case 'a':
			op = '>';
			break;
	}

	char query[256];
	snprintf(query, sizeof(query), "SELECT " POST_LIST_FIELDS_FULL
			" FROM posts WHERE board = %d AND id %c %%"DBIdPID, bid, op);
	size_t size = sizeof(query) - strlen(query);
	char *q = query;

	if (action == 'a' || action == 'b')
		strappend(&q, &size, " AND gid = %%"DBIdPID);
	if (op == '<')
		strappend(&q, &size, " ORDER BY id DESC");
	if (op == '>')
		strappend(&q, &size, " ORDER BY id ASC");

	db_res_t *res;
	if (action == 'a' || action == 'b')
		res = db_query(query, pid, tid);
	else
		res = db_query(query, pid);

	int ret = res && db_res_rows(res) > 0;
	if (ret)
		res_to_post_info_full(res, 0, p);
	else
		db_clear(res);

	if (ret && extra) {
//		if (f == begin)
//			ret |= POST_FIRST;
//		if (f == end - 1)
//			ret |= POST_LAST;
		if (p->p.id == p->p.tid)
			ret |= THREAD_FIRST_POST;
//		if (f >= end)
//			ret |= THREAD_LAST_POST;
	}

	return ret;
}

int bbscon_search_pid(int bid, post_id_t pid, post_info_full_t *p)
{
	return bbscon_search(bid, pid, 0, 0, false, p);
}

int bbscon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = strtol(get_param("f"), NULL, 10);
	post_id_t tid = strtol(get_param("t"), NULL, 10);
	char action = *get_param("a");

	post_info_full_t info;
	int ret = bbscon_search(board.id, pid, tid, action, true, &info);
	if (!ret)
		return BBS_ENOFILE;

	xml_header(NULL);

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();
	printf("<bbscon link='con' bid='%d' anony='%d' attach='%d'%s%s>",
			board.id, anony, maxlen(board.name),
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");

	print_session();

	bool isbm = am_bm(&board);
	bool self = (session.uid == info.p.uid);
	printf("<po fid='%'"PRIdPID"%s%s%s%s%s%s", pid,
			(info.p.flag & POST_FLAG_STICKY) ? " sticky='1'" : "",
			ret & POST_FIRST ? " first='1'" : "",
			ret & POST_LAST ? " last='1'" : "",
			ret & THREAD_LAST_POST ? " tlast='1'" : "",
			(info.p.flag & POST_FLAG_LOCKED) ? " nore='1'" : "",
			self || isbm ? " edit='1'" : "");
	if (info.p.reid != info.p.id)
		printf(" reid='%u' gid='%u'>", info.p.reid, info.p.tid);
	else
		printf(">");

	xml_print_post_wrapper(info.content, info.length);

	printf("</po></bbscon>");

	brc_fcgi_init(currentuser.userid, board.name);
	brc_mark_as_read(info.p.id);
	brc_update(currentuser.userid, board.name);

	free_post_info_full(&info);
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
	xml_print_post_wrapper(file, 0);

	printf("</po></bbscon>");
	brc_fcgi_init(currentuser.userid, board.name);
	brc_addlist_legacy(f);
	brc_update(currentuser.userid, board.name);
	return 0;
}

int xml_print_post_wrapper(const char *str, size_t size)
{
	if (request_type(REQUEST_PARSED)) {
		int opt = PARSE_NOQUOTEIMG;
		int flag = get_user_flag();
		if (flag & PREF_NOSIG)
			opt |= PARSE_NOSIG;
		if (flag & PREF_NOSIGIMG)
			opt |= PARSE_NOSIGIMG;
		return xml_print_post(str, size, opt);
	}
	if (!request_type(REQUEST_MOBILE)) {
		xml_fputs2(str, size, stdout);
		return 0;
	}
	return xml_print_post(str, size, PARSE_NOSIG);
}
