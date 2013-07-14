#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

enum {
	THREAD_LAST_POST = 1 << 4,
	THREAD_LAST = 1 << 5,
	THREAD_FIRST = 1 << 6,
};

extern const struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid);

static post_id_t find_next_tid(int bid, post_id_t tid, char action)
{
	post_filter_t filter = { .bid = bid, .type = POST_LIST_TOPIC };
	if (action == 'a')
		filter.min = tid + 1;
	else
		filter.max = tid - 1;

	query_t *q = build_post_query(&filter, action == 'a', 1);
	db_res_t *res = query_exec(q);

	if (res && db_res_rows(res) >= 1) {
		post_info_t info;
		res_to_post_info(res, 0, 0, &info);
		tid = info.tid;
	}
	db_clear(res);
	return tid;
}

// action 'a' next thread
// 'b' previous thread
// 'n' next page
// 'p' previous page
// (none): first page
// TODO: THREAD_FIRST, THREAD_LAST
static post_info_full_t *bbstcon_search(int bid, post_id_t pid, post_id_t *tid,
		char action, int *count, int *flag)
{
	if (action == 'a' || action == 'b') {
		*tid = find_next_tid(bid, *tid, action);
	}

	post_filter_t filter = { .bid = bid, .tid = *tid };
	if (action == 'p')
		filter.max = pid - 1;
	else if (action == 'n')
		filter.min = pid + 1;
	bool asc = !(action == 'b' || action == 'p');

	query_t *q = query_new(0);
	query_select(q, POST_LIST_FIELDS_FULL);
	query_from(q, post_table_name(&filter));
	build_post_filter(q, &filter, &asc);
	query_limit(q, *count + 1);

	db_res_t *res = query_exec(q);
	int rows = db_res_rows(res);
	if (!rows)
		return NULL;

	if (rows <= *count) {
		if (asc)
			*flag |= THREAD_LAST_POST;
		*count = rows;
	}

	post_info_full_t *p = malloc(sizeof(*p) * rows);
	for (int i = 0; i < rows; ++i) {
		res_to_post_info_full(res, i, 0, p + i);
	}
	return p;
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

	post_id_t tid = strtoll(get_param("g"), NULL, 10);
	post_id_t pid = strtoll(get_param("f"), NULL, 10);
	char action = *(get_param("a"));
	if (!tid)
		tid = pid;

	int count = POSTS_PER_PAGE;
	int c = count, flag = 0;
	post_info_full_t *p = bbstcon_search(board.id, pid, &tid, action, &c,
			&flag);
	if (!p)
		return BBS_ENOFILE;

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();
	bool isbm = am_bm(&board);

	xml_header(NULL);
	printf("<bbstcon bid='%d' gid='%"PRIdPID"' anony='%d' page='%d'"
			" attach='%d'%s%s%s%s%s>",
			bid, tid, anony, count, maxlen(board.name),
			flag & THREAD_LAST_POST ? " last='1'" : "",
			flag & THREAD_LAST ? " tlast='1'" : "",
			flag & THREAD_FIRST ? " tfirst='1'" : "",
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");
	print_session();

	brc_initialize(currentuser.userid, board.name);

	bool asc = action != 'p';
	if (c > count)
		c = count;
	for (post_info_full_t *ip = asc ? p : p + c - 1;
			asc ? ip < p + c : ip >= p;
			ip += asc ? 1 : -1) {
		printf("<po fid='%"PRIdPID"' owner='%s'%s>", ip->p.id, ip->p.owner,
				!isbm && (ip->p.flag & POST_FLAG_LOCKED) ? " nore='1'" : "");
		xml_print_post_wrapper(ip->content, ip->length);
		puts("</po>");
		brc_mark_as_read(ip->p.stamp);
	}
	puts("</bbstcon>");
	free_post_info_full(p);

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
