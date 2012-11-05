#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

#define BFIND_MAX  "100"

enum {
	TOPICS_PER_PAGE = 13,
};

extern const struct fileheader *dir_bsearch(const struct fileheader *begin,
        const struct fileheader *end, unsigned int fid);

static void print_bbsdoc(const post_info_t *p)
{
	int mark = get_post_mark(p);

	printf("<po %s%sm='%c' owner='%s' time= '%s' id='%"PRIdPID"'>",
			(p->flag & POST_FLAG_STICKY) ? "sticky='1' " : "",
			(p->flag & POST_FLAG_LOCKED) ? "nore='1' " : "",
			mark, p->owner, getdatestring(p->stamp, DATE_XML), p->id);

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(p->utf8_title, gbk_title);
	xml_fputs2(gbk_title, 0, stdout);
	printf("</po>\n");
}

extern int web_sector(void);

static void print_board_logo(const char *board)
{
	char path[HOMELEN];

	snprintf(path, sizeof(path), "%s/info/boards/%s/icon.jpg", BBSHOME, board);
	if (dashf(path))
		printf(" icon='../info/boards/%s/icon.jpg'", board);

	snprintf(path, sizeof(path), "%s/info/boards/%s/banner.jpg",
			BBSHOME, board);
	if (dashf(path))
		printf(" banner='../info/boards/%s/banner.jpg'", board);
}

static void print_sticky_posts(int bid, post_list_type_e type)
{
	post_info_t *sticky_posts = NULL;

	int count = _load_sticky_posts(bid, &sticky_posts);
	for (int i = 0; i < count; ++i) {
		print_bbsdoc(sticky_posts + i);
	}

	free(sticky_posts);
}

static bool print_posts(post_filter_t *filter, int limit, bool asc)
{
	query_builder_t *b = build_post_query(filter, asc, limit + 1);
	db_res_t *r = b->query(b);
	query_builder_free(b);

	post_id_t next = true;
	if (r) {
		int rows = db_res_rows(r);
		if (rows > limit) {
			rows = limit;
		} else {
			limit = rows;
			next = false;
		}

		for (int i = asc ? 0 : limit - 1;
				asc ? i < limit : i >= 0;
				i += asc ? 1 : -1) {
			post_info_t info;
			res_to_post_info(r, i, 0, &info);
			print_bbsdoc(&info);
		}
		db_clear(r);
	}
	return next;
}

static int bbsdoc(post_list_type_e type)
{
	board_t board;
	const char *bidstr = get_param("bid");
	if (*bidstr == '\0')
		get_board(get_param("board"), &board);
	else
		get_board_by_bid(strtol(bidstr, NULL, 10), &board);

	if (!board.id || !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return web_sector();

	board_to_gbk(&board);

	post_id_t start = strtol(get_param("start"), NULL, 10);
	char action = *get_param("a");

	int page = strtol(get_param("my_t_lines"), NULL, 10);
	if (page < TLINES || page > 40)
		page = TLINES;

	if (get_doc_mode() != type)
		set_doc_mode(type);

	xml_header(NULL);
	printf("<bbsdoc>");
	print_session();

	brc_fcgi_init(currentuser.userid, board.name);

	post_filter_t filter = {
		.bid = board.id, .type = type, .max = start,
	};
	if (action == 'n') {
		filter.max = 0;
		filter.min = start;
	}
	if (type == POST_LIST_DIGEST)
		filter.flag |= POST_FLAG_DIGEST;

	post_id_t next = print_posts(&filter, page, action == 'n');
	if (type != POST_LIST_DIGEST)
		print_sticky_posts(board.id, type);

	char *cgi_name = "";
	if (type == POST_LIST_DIGEST)
		cgi_name = "g";
	else if (type == POST_LIST_TOPIC)
		cgi_name = "t";

	printf("<brd title='%s' desc='%s' bm='%s' start='%d' "
			"bid='%d' page='%d' link='%s'", board.name, board.descr, board.bms,
			start, board.id, page, cgi_name);
	if (!start || (action == 'n' && !next))
		printf(" bottom='1'");
	if (action != 'n' && !next)
		printf(" top='1'");
	print_board_logo(board.name);
	printf("/>\n</bbsdoc>");

	return 0;
}

int bbsdoc_main(void)
{
	return bbsdoc(POST_LIST_NORMAL);
}

int bbsgdoc_main(void)
{
	return bbsdoc(POST_LIST_DIGEST);
}

int bbstdoc_main(void)
{
	return bbsdoc(POST_LIST_TOPIC);
}
int bbsodoc_main(void)
{
	return bbsdoc(MODE_TOPICS);
}

int bbsbfind_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	int bid = strtol(get_param("bid"), NULL, 10);
	board_t board;
	if (!get_board_by_bid(bid, &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;

	post_filter_t filter = { .bid = bid, .type = POST_LIST_NORMAL };
	if (strcaseeq(get_param("mark"), "on"))
		filter.flag |= POST_FLAG_MARKED;
	if (strcaseeq(get_param("nore"), "on"))
		filter.flag |= POST_FLAG_DIGEST;

	const char *uname = get_param("user");
	user_id_t uid = get_user_id(uname);
	if (uid)
		filter.uid = uid;

	query_builder_t *b = query_builder_new(0);
	b->sappend(b, "SELECT", POST_LIST_FIELDS);
	b->sappend(b, "FROM", "posts.recent");
	build_post_filter(b, &filter, NULL);

	long day = strtol(get_param("limit"), NULL, 10);
	if (day < 0)
		day = 0;
	fb_time_t begin = time(NULL) - 24 * 60 * 60 * day;
	b->sappend(b, "AND", "stamp > %t", begin);

	int count = 0;
	const char *names[] = { "t1", "t2", "t3" };
	for (int i = 0; i < ARRAY_SIZE(names); ++i) {
		const char *s = get_param(names[i]);
		if (s && *s) {
			++count;
			b->sappend(b, "AND", "title ILIKE '%%' || %s || '%%'", s);
		}
	}

	b->sappend(b, "ORDER BY", "id DESC");
	b->append(b, "LIMIT "BFIND_MAX);

	db_res_t *res = b->query(b);
	query_builder_free(b);

	xml_header(NULL);
	printf("<bbsbfind ");
	printf(" bid='%d'", bid);

	if (count || uid) {
		printf(" result='1'>");
		for (int i = 0; i < db_res_rows(res); ++i) {
			post_info_t info;
			res_to_post_info(res, i, 0, &info);
			print_bbsdoc(&info);
		}
	} else {
		printf(">");
	}
	db_clear(res);

	print_session();
	printf("</bbsbfind>");
	return 0;
}

#define POST_THREAD_FIELDS  \
	"tid, stamp, last_id, last_stamp, replies, comments, owner, uname, title"

typedef struct {
	post_id_t tid;
	post_id_t last_id;
	fb_time_t stamp;
	fb_time_t last_stamp;
	int replies;
	int comments;
	user_id_t owner;
	char uname[IDLEN + 1];
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
} post_thread_info_t;

static void res_to_post_thread_info(db_res_t *res, int row,
		post_thread_info_t *p)
{
	p->tid = db_get_post_id(res, row, 0);
	p->last_id = db_get_post_id(res, row, 2);
	p->stamp = db_get_time(res, row, 1);
	p->last_stamp = db_get_time(res, row, 3);
	p->replies = db_get_integer(res, row, 4);
	p->comments = db_get_integer(res, row, 5);
	p->owner = db_get_user_id(res, row, 6);
	strlcpy(p->uname, db_get_value(res, row, 7), sizeof(p->uname));
	strlcpy(p->utf8_title, db_get_value(res, row, 8), sizeof(p->utf8_title));
}

static db_res_t *get_post_threads(int bid, int start, int count)
{
	query_builder_t *b = query_builder_new(0);
	b->sappend(b, "SELECT", POST_THREAD_FIELDS);
	b->sappend(b, "FROM", "posts.threads");
	b->sappend(b, "WHERE", "board = %d", bid);
	if (start)
		b->sappend(b, "AND", "last_id <= %"DBIdPID, start);
	b->sappend(b, "ORDER BY", "last_id DESC");
	b->append(b, "LIMIT %l", count);

	db_res_t *res = b->query(b);
	query_builder_free(b);
	return res;
}

int web_forum(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			&& !get_board(get_param("board"), &board))
		return BBS_ENOBRD;
	if (!has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return web_sector();
	board_to_gbk(&board);

	if (get_doc_mode() != MODE_FORUM)
		set_doc_mode(MODE_FORUM);

	brc_fcgi_init(currentuser.userid, board.name);

	xml_header(NULL);
	printf("<forum title='%s' desc='%s' bm='%s' bid='%d'>", 
			board.name, board.descr, board.bms, board.id);
	print_board_logo(board.name);
	print_session();

	int count = TOPICS_PER_PAGE;
	int start = strtoll(get_param("start"), NULL, 10);

	db_res_t *res = get_post_threads(board.id, start, count);
	int rows = db_res_rows(res);
	for (int i = 0; i < rows; ++i) {
		post_thread_info_t t;
		res_to_post_thread_info(res, i, &t);

		printf("<po gid='%"PRIdPID"' m='%c' posts='%d'",
				t.tid, brc_unread(t.tid) ? '+' : ' ', t.replies);
		printf(" owner='%s' potime='%s'",
				t.uname, getdatestring(t.stamp, DATE_XML));
		if (t.replies > 0)
			printf(" uptime='%s'", getdatestring(t.last_stamp, DATE_XML));
		printf(" lastpage='%u'", t.last_id);
		printf(">");

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(t.utf8_title, gbk_title);
		xml_fputs2(gbk_title, 0, stdout);
		printf("</po>\n");
	}
	db_clear(res);
	printf("</forum>");

	return 0;
}
