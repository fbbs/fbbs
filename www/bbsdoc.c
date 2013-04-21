#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

enum {
	TOPICS_PER_PAGE = 13,
	BFIND_MAX = 100,
};

static void print_post(const post_info_t *pi, bool sticky)
{
	int mark = get_post_mark(pi);
	printf("<po %s%sm='%c' owner='%s' time= '%s' id='%"PRIdPID"'>",
			sticky ? "sticky='1' " : "",
			(pi->flag & POST_FLAG_LOCKED) ? "" : "nore='1' ",
			mark, pi->owner, getdatestring(pi->stamp, DATE_XML), pi->id);
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pi->utf8_title, gbk_title);
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

typedef struct {
	int start;
	int total;
	int count;
	int max;
	bool thread;
	bool digest;
} print_post_filter_t;

static int print_post_filter(const void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	print_post_filter_t *ppf = args;
	if (ppf->thread && pib->tid_delta)
		return -1;
	if (ppf->digest && !(pib->flag & POST_FLAG_DIGEST))
		return -1;
	++ppf->total;
	if (ppf->total < ppf->start)
		return -1;
	if (++ppf->count > ppf->max)
		return -1;
	return 0;
}

typedef struct {
	post_index_record_t *pir;
	bool sticky;
} print_post_callback_t;

static int print_post_callback(void *ptr, void *args)
{
	const post_index_board_t *pib = ptr;
	print_post_callback_t *ppc = args;
	post_info_t pi;
	post_index_board_to_info(ppc->pir, pib, &pi, 1);
	print_post(&pi, ppc->sticky);
	return 0;
}

static int print_posts(record_t *record, post_index_record_t *pir,
		int *start, int max, post_list_type_e type, bool sticky)
{
	bool normal = type == POST_LIST_NORMAL;
	print_post_filter_t ppf = {
		.start = normal ? 0 : *start, .max = max,
		.thread = type == POST_LIST_TOPIC,
		.digest = type == POST_LIST_DIGEST,
	};
	print_post_callback_t ppc = { .pir = pir, .sticky = sticky, };

	record_foreach(record, NULL, normal ? *start : 0,
			print_post_filter, &ppf, print_post_callback, &ppc);
	int total = normal ? record_count(record) : ppf.total;

	if (!sticky) {
		if (*start <= 0 || *start > total - max)
			*start = total - max + 1;
		if (*start < 1)
			*start = 1;
	}
	return total;
}

static void print_sticky_posts(int bid, post_list_type_e type,
		post_index_record_t *pir)
{
	record_t record;
	post_index_board_open_sticky(bid, RECORD_READ, &record);
	int start = 0;
	print_posts(&record, pir, &start, MAX_NOTICE, type, true);
	record_close(&record);
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

	int start = strtoll(get_param("start"), NULL, 10);
	int page = strtol(get_param("my_t_lines"), NULL, 10);
	if (page < TLINES || page > 40)
		page = TLINES;

	if (get_doc_mode() != type)
		set_doc_mode(type);

	xml_header(NULL);
	printf("<bbsdoc>");
	print_session();

	brc_fcgi_init(currentuser.userid, board.name);

	post_index_record_t pir;
	post_index_record_open(&pir);
	record_t record;
	post_index_board_open(board.id, RECORD_READ, &record);

	int total = print_posts(&record, &pir, &start, page, type, false);
	record_close(&record);
	if (type != POST_LIST_DIGEST)
		print_sticky_posts(board.id, type, &pir);
	post_index_record_close(&pir);

	char *cgi_name = "";
	if (type == POST_LIST_DIGEST)
		cgi_name = "g";
	else if (type == POST_LIST_TOPIC)
		cgi_name = "t";

	printf("<brd title='%s' desc='%s' bm='%s' total='%d' start='%d' "
			"bid='%d' page='%d' link='%s'", board.name, board.descr, board.bms,
			total, start, board.id, page, cgi_name);
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
#if 0
	post_filter_t filter = { .bid = bid, .type = POST_LIST_NORMAL };
	if (strcaseeq(get_param("mark"), "on"))
		filter.flag |= POST_FLAG_MARKED;
	if (strcaseeq(get_param("nore"), "on"))
		filter.flag |= POST_FLAG_DIGEST;

	const char *uname = get_param("user");
	user_id_t uid = get_user_id(uname);
	if (uid)
		filter.uid = uid;

	query_t *q = query_new(0);
	query_select(q, POST_LIST_FIELDS);
	query_from(q, post_table_name(&filter));
	build_post_filter(q, &filter, NULL);

	long day = strtol(get_param("limit"), NULL, 10);
	if (day < 0)
		day = 0;
	fb_time_t begin = time(NULL) - 24 * 60 * 60 * day;
	query_and(q, "stamp > %t", begin);

	int count = 0;
	const char *names[] = { "t1", "t2", "t3" };
	for (int i = 0; i < ARRAY_SIZE(names); ++i) {
		const char *s = get_param(names[i]);
		if (s && *s) {
			++count;
			query_and(q, "title ILIKE '%%' || %s || '%%'", s); 
		}
	}

	query_orderby(q, "id", false);
	query_limit(q, BFIND_MAX);

	db_res_t *res = query_exec(q);

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
#endif
	print_session();
	printf("</bbsbfind>");
	return 0;
}

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

#if 0
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
#endif
	return 0;
}
