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
	post_index_record_t *pir;
	int start;
	int total;
	int count;
	int max;
	bool thread;
	bool digest;
	bool sticky;
} print_post_callback_t;

static int print_post_callback(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	print_post_callback_t *ppc = args;

	if (ppc->thread && pib->tid_delta)
		return -1;
	if (ppc->digest && !(pib->flag & POST_FLAG_DIGEST))
		return -1;
	++ppc->total;
	if (ppc->total < ppc->start)
		return -1;
	if (++ppc->count > ppc->max)
		return -1;

	post_info_t pi;
	post_index_board_to_info(ppc->pir, pib, &pi, 1);
	print_post(&pi, ppc->sticky);
	return 0;
}

static int print_posts(record_t *record, post_index_record_t *pir,
		int *start, int max, post_list_type_e type, bool sticky)
{
	bool normal = type == POST_LIST_NORMAL;
	print_post_callback_t ppc = {
		.pir = pir, .sticky = sticky,
		.start = normal ? 0 : *start, .max = max,
		.thread = type == POST_LIST_TOPIC,
		.digest = type == POST_LIST_DIGEST,
	};

	record_foreach(record, NULL, normal ? *start : 0,
			print_post_callback, &ppc);
	int total = normal ? record_count(record) : ppc.total;

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
	int replies;
} post_thread_stat_t;

typedef struct {
	post_thread_stat_t *pts;
	int capacity;
	int size;
} update_thread_stat_t;

typedef struct {
	post_id_t tid;
	post_id_t last_id;
	fb_time_t stamp;
	fb_time_t last_stamp;
	int flag;
	int replies;
	int comments;
	user_id_t uid;
	char owner[IDLEN + 1];
	char last_replier[IDLEN + 1];
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
} post_thread_info_t;

static int cmp(const void *t, const void *p)
{
	const post_id_t *tid = t;
	const post_thread_stat_t *pts = p;
	return *tid - pts->tid;
}

static int update_thread_stat(void *r, void *args, int offset)
{
	const post_index_board_t *pib = r;
	update_thread_stat_t *uts = args;

	post_id_t tid = pib->id - pib->tid_delta;
	post_thread_stat_t *pts = bsearch(&tid, uts->pts, uts->size,
			sizeof(*pts), cmp);

	if (!pts && (!uts->size || tid > uts->pts[uts->size - 1].tid)
			&& uts->size < uts->capacity && !pib->tid_delta) {
		pts = uts->pts + uts->size++;
		pts->tid = tid;
		pts->replies = 0;
	}
	if (pts) {
		pts->last_id = pib->id;
		++pts->replies;
	}
	return 0;
}

static int thread_compare(const void *r1, const void *r2)
{
	const post_thread_stat_t *pts1 = r1, *pts2 = r2;
	return pts1->last_id - pts2->last_id;
}

static int prepare_threads(int bid, post_thread_stat_t **pts)
{
	if (!pts)
		return -1;

	record_t record;
	if (post_index_board_open(bid, RECORD_READ, &record) < 0)
		return -1;

	int posts = record_count(&record), threads = -1;
	if (posts > 0) {
		*pts = malloc(sizeof(**pts) * posts);
		if (*pts) {
			update_thread_stat_t uts = { .pts = *pts, .capacity = posts };
			record_foreach(&record, NULL, 0, update_thread_stat, &uts);
			qsort(uts.pts, uts.size, sizeof(*uts.pts), thread_compare);
			threads = uts.size;
		}
	}

	record_close(&record);
	return threads;
}

static void construct_post_thread_info(post_index_record_t *pir,
		const post_thread_stat_t *pts, post_thread_info_t *pti)
{
	memset(pti, 0, sizeof(*pti));
	pti->tid = pts->tid;
	pti->last_id = pts->last_id;
	pti->replies = pts->replies;

	post_index_t pi;
	if (post_index_record_read(pir, pts->tid, &pi)) {
		pti->flag = pi.flag;
		pti->stamp = pi.stamp;
		pti->uid = pi.uid;
		strlcpy(pti->owner, pi.owner, sizeof(pti->owner));
		strlcpy(pti->utf8_title, pi.utf8_title, sizeof(pti->utf8_title));
	}
	if (pti->replies > 1 && post_index_record_read(pir, pts->last_id, &pi)) {
		pti->last_stamp = pi.stamp;
		strlcpy(pti->last_replier, pi.owner, sizeof(pti->last_replier));
	}
}

static void print_post_thread_info(const post_thread_info_t *pti)
{
	printf("<po gid='%"PRIdPID"' m='%c' posts='%d'",
			pti->tid, get_post_mark_raw(pti->tid, pti->flag), pti->replies);
	printf(" owner='%s' potime='%s'",
			pti->owner, getdatestring(pti->stamp, DATE_XML));
	if (pti->replies > 1)
		printf(" upuser='%s' uptime='%s'", pti->last_replier,
				getdatestring(pti->last_stamp, DATE_XML));
	printf(" lastpage='%u'", pti->last_id);
	printf(">");

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pti->utf8_title, gbk_title);
	xml_fputs2(gbk_title, 0, stdout);
	printf("</po>\n");
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

	int count = TOPICS_PER_PAGE;
	int end = strtoll(get_param("start"), NULL, 10);

	post_thread_stat_t *pts = NULL;
	int threads = prepare_threads(board.id, &pts);

	if (end <= 0 || end > threads)
		end = threads;
	int begin = end - count;
	if (begin < 0)
		begin = 0;

	xml_header(NULL);
	printf("<forum title='%s' desc='%s' bm='%s' bid='%d' next='%d'>",
			board.name, board.descr, board.bms, board.id, begin);
	print_board_logo(board.name);
	print_session();

	if (pts) {
		post_index_record_t pir;
		post_index_record_open(&pir);

		for (int i = end - 1; i >= begin; --i) {
			post_thread_info_t pti;
			construct_post_thread_info(&pir, pts + i, &pti);
			print_post_thread_info(&pti);
		}

		post_index_record_close(&pir);
		free(pts);
	}
	printf("</forum>");
	return 0;
}
