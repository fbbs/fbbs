#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

enum {
	TOPICS_PER_PAGE = 13,
};

/** 用户web浏览模式 @mdb_hash */
#define POST_LIST_TYPE_KEY "doc_mode"

post_list_type_e get_post_list_type(void)
{
	return (post_list_type_e) mdb_integer(0, "HGET",
			POST_LIST_TYPE_KEY" %"PRIdUID, session_uid());
}

static void set_post_list_type(post_list_type_e type)
{
	mdb_cmd("HSET", POST_LIST_TYPE_KEY" %"PRIdUID" %d", session_uid(), type);
}

const char *get_post_list_type_string(void)
{
	if (!session_id())
		return "";

	post_list_type_e type = get_post_list_type();
	switch (type) {
		case POST_LIST_TOPIC:
			return "t";
		case POST_LIST_FORUM:
			return web_request_type(MOBILE) ? "" : "f";
		default:
			return "";
	}
}

static void print_post(const post_info_t *pi, bool sticky)
{
	int mark = get_post_mark(pi);
	printf("<po %s%sm='%c' owner='%s' time= '%s' id='%"PRIdPID"'>",
			sticky ? "sticky='1' " : "",
			(pi->flag & POST_FLAG_LOCKED) ? "" : "nore='1' ",
			mark, pi->owner, format_time(pi->stamp, TIME_FORMAT_XML), pi->id);
	if (web_request_type(UTF8)) {
		xml_fputs(pi->utf8_title);
	} else {
		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(pi->utf8_title, gbk_title);
		xml_fputs(gbk_title);
	}
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
	post_index_board_t *pibs;
	int start;
	int total;
	int count;
	int max;
	bool thread;
	bool digest;
	bool sticky;
} print_post_callback_t;

static void print_pib(post_index_record_t *pir, const post_index_board_t *pib,
		bool sticky)
{
	post_info_t pi;
	post_index_board_to_info(pir, pib, &pi, 1);
	string_check_tail(pi.utf8_title, NULL);
	print_post(&pi, sticky);
}

static record_callback_e print_post_callback(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	print_post_callback_t *ppc = args;

	if ((ppc->thread && pib->tid_delta)
			|| (ppc->digest && !(pib->flag & POST_FLAG_DIGEST)))
		return RECORD_CALLBACK_CONTINUE;
	if ((++ppc->total < ppc->start) || (!ppc->pibs && ++ppc->count > ppc->max))
		return RECORD_CALLBACK_CONTINUE;

	if (ppc->pibs) {
		memcpy(ppc->pibs + ((ppc->total - 1) % ppc->max), pib,
				sizeof(*ppc->pibs));
	} else {
		print_pib(ppc->pir, pib, ppc->sticky);
	}
	return RECORD_CALLBACK_MATCH;
}

static int print_posts(record_t *record, post_index_record_t *pir,
		int *start, int max, post_list_type_e type, bool sticky)
{
	bool normal = (type == POST_LIST_NORMAL);
	bool save = ((type == POST_LIST_TOPIC || type == POST_LIST_DIGEST)
			&& *start < 0);

	print_post_callback_t ppc = {
		.pir = pir, .sticky = sticky,
		.start = normal ? 0 : *start, .max = max,
		.thread = type == POST_LIST_TOPIC,
		.digest = type == POST_LIST_DIGEST,
		.pibs = save ? malloc(sizeof(post_index_board_t) * max) : NULL,
	};

	record_foreach(record, NULL, normal ? *start : 0,
			print_post_callback, &ppc);
	int total = normal ? record_count(record) : ppc.total;

	if (!sticky) {
		if (*start < 0 || *start > total - max)
			*start = total - max;
		if (*start < 0)
			*start = 0;
	}
	if (save) {
		for (int i = *start; i < total; ++i) {
			print_pib(pir, ppc.pibs + (i % max), false);
		}
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

bool get_board_by_param(board_t *bp)
{
	int bid = strtol(web_get_param("bid"), NULL, 10);
	if (bid > 0 && get_board_by_bid(bid, bp) > 0)
		return has_read_perm(bp);

	const char *bname = web_get_param("board");
	if (bname && *bname)
		return get_board(bname, bp) && has_read_perm(bp);
	return false;
}

static int bbsdoc(post_list_type_e type)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return web_sector();
	session_set_board(board.id);

	int start = strtoll(web_get_param("start"), NULL, 10);
	int page = strtol(web_get_param("my_t_lines"), NULL, 10);
	if (page < TLINES || page > 40)
		page = TLINES;

	if (get_post_list_type() != type)
		set_post_list_type(type);

	xml_header(NULL);
	printf("<bbsdoc>");
	print_session();

	brc_initialize(currentuser.userid, board.name);

	post_index_record_t pir;
	post_index_record_open(&pir);
	record_t record;
	post_index_board_open(board.id, RECORD_READ, &record);

	--start;
	if (start < 0 && type == POST_LIST_NORMAL) {
		start = record_count(&record) - page;
		if (start < 0)
			start = 0;
	}

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

	if (!web_request_type(UTF8))
		board_to_gbk(&board);
	printf("<brd title='%s' desc='%s' bm='%s' total='%d' start='%d' "
			"bid='%d' page='%d' link='%s'", board.name, board.descr, board.bms,
			total, ++start, board.id, page, cgi_name);
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

enum {
	BFIND_MAX = 100,
};

typedef struct {
	bool marked;
	bool digest;
	int count;
	user_id_t uid;
	fb_time_t begin;
	post_index_record_t *pir;
	UTF8_BUFFER(t1, POST_TITLE_CCHARS);
	UTF8_BUFFER(t2, POST_TITLE_CCHARS);
	UTF8_BUFFER(t3, POST_TITLE_CCHARS);
} web_post_filter_t;

static record_callback_e web_post_filter(void *r, void *args, int offset)
{
	const post_index_board_t *pib = r;
	web_post_filter_t *wpf = args;

	if (wpf->count > BFIND_MAX)
		return RECORD_CALLBACK_BREAK;
	if ((wpf->marked && !(pib->flag & POST_FLAG_MARKED))
			|| (wpf->digest && !(pib->flag & POST_FLAG_DIGEST))
			|| (wpf->uid && pib->uid != wpf->uid))
		return RECORD_CALLBACK_CONTINUE;

	post_info_t pi;
	post_index_board_to_info(wpf->pir, pib, &pi, 1);

	if (pi.stamp < wpf->begin)
		return RECORD_CALLBACK_BREAK;
	if ((*wpf->utf8_t1 && !strcaseeq(wpf->utf8_t1, pi.utf8_title))
			|| (*wpf->utf8_t2 && !strcaseeq(wpf->utf8_t2, pi.utf8_title))
			|| (*wpf->utf8_t3 && strcaseeq(wpf->utf8_t3, pi.utf8_title)))
		return RECORD_CALLBACK_CONTINUE;

	++wpf->count;
	print_post(&pi, false);
	return RECORD_CALLBACK_MATCH;
}

int bbsbfind_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	session_set_board(board.id);

	record_t record;
	if (post_index_board_open(board.id, RECORD_READ, &record) < 0)
		return BBS_EINTNL;

	post_index_record_t pir;
	post_index_record_open(&pir);

	web_post_filter_t wpf = {
		.marked = strcaseeq(web_get_param("mark"), "on"),
		.digest = strcaseeq(web_get_param("nore"), "on"),
	};

	const char *uname = web_get_param("user");
	user_id_t uid = get_user_id(uname);
	if (uid)
		wpf.uid = uid;

	long day = strtol(web_get_param("limit"), NULL, 10);
	if (day < 0)
		day = 0;
	wpf.begin = time(NULL) - 24 * 60 * 60 * day;

	const char *names[] = { "t1", "t2", "t3" };
	char *fields[] = { wpf.utf8_t1, wpf.utf8_t2, wpf.utf8_t3 };
	int count = 0;
	for (int i = 0; i < ARRAY_SIZE(names); ++i) {
		const char *s = web_get_param(names[i]);
		if (s && *s) {
			++count;
			GBK_BUFFER(buf, POST_TITLE_CCHARS);
			strlcpy(gbk_buf, s, sizeof(gbk_buf));
			convert(env_g2u, gbk_buf, CONVERT_ALL, fields[i],
					sizeof(wpf.utf8_t1), NULL, NULL);
		}
	}

	xml_header(NULL);
	printf("<bbsbfind ");
	printf(" bid='%d'", board.id);

	if (count || uid) {
		printf(" result='1'>");
		record_reverse_foreach(&record, web_post_filter, &wpf);
	} else {
		printf(">");
	}

	post_index_record_close(&pir);
	record_close(&record);

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

static record_callback_e update_thread_stat(void *r, void *args, int offset)
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
	return RECORD_CALLBACK_MATCH;
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
			pti->tid, get_post_mark_raw(pti->stamp, pti->flag), pti->replies);
	printf(" owner='%s' potime='%s'",
			pti->owner, format_time(pti->stamp, TIME_FORMAT_XML));
	if (pti->replies > 1)
		printf(" upuser='%s' uptime='%s'", pti->last_replier,
				format_time(pti->last_stamp, TIME_FORMAT_XML));
	printf(" lastpage='%u'", pti->last_id);
	printf(">");

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(pti->utf8_title, gbk_title);
	xml_fputs(gbk_title);
	printf("</po>\n");
}

int web_forum(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return web_sector();
	session_set_board(board.id);
	board_to_gbk(&board);

	if (get_post_list_type() != POST_LIST_FORUM)
		set_post_list_type(POST_LIST_FORUM);

	brc_initialize(currentuser.userid, board.name);

	int count = TOPICS_PER_PAGE;
	int end = strtoll(web_get_param("start"), NULL, 10);

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
#if 0
static xml_node_t *create_post_node(const post_info_t *p)
{
	xml_node_t *post = xml_new_node("post", XML_NODE_ANONYMOUS_JSON);
	xml_attr_bigint(post, "id", p->id);
	xml_attr_bigint(post, "reid", p->reid);
	xml_attr_bigint(post, "tid", p->tid);
	xml_attr_string(post, "owner", p->owner, false);
	xml_attr_string(post, "stamp", format_time(p->stamp, TIME_FORMAT_XML), false);
	xml_attr_string(post, "title", p->utf8_title, true);
	char mark[2] = "\0";
	mark[0] = get_post_mark(p);
	xml_attr_string(post, "mark", mark, false);
	return post;
}

static int print_toc(xml_node_t *posts, db_res_t *res, int archive, bool asc)
{
	int rows = db_res_rows(res);
	for (int i = asc ? rows - 1 : 0;
			asc ? i >= 0 : i < rows;
			i += asc ? -1 : 1) {
		post_info_t p;
		res_to_post_info(res, i, archive, &p);
		xml_node_t *post = create_post_node(&p);
		xml_add_child(posts, post);
	}
	return rows;
}

static void print_toc_sticky(xml_node_t *root, int bid)
{
	xml_node_t *stickies = xml_new_child(root, "stickies",
			XML_NODE_CHILD_ARRAY);
	db_res_t *res = db_query("SELECT " POST_LIST_FIELDS " FROM posts.recent"
			" WHERE board = %d AND sticky ORDER BY id DESC", bid);
	print_toc(stickies, res, 0, false);
	db_clear(res);
}

enum {
	API_BOARD_TOC_LIMIT_DEFAULT = 20,
	API_BOARD_TOC_LIMIT_MAX = 50,
};

extern void board_to_node(const board_t *bp, xml_node_t *node);

int api_board_toc(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return error_msg(ERROR_BOARD_NOT_FOUND);
	session_set_board(board.id);
	brc_initialize(currentuser.userid, board.name);

	bool asc = streq(web_get_param("page"), "next");
	post_id_t start = strtoll(web_get_param("start"), NULL, 10);

	int limit = strtol(web_get_param("limit"), NULL, 10);
	if (limit > API_BOARD_TOC_LIMIT_MAX)
		limit = API_BOARD_TOC_LIMIT_MAX;
	if (limit < 0)
		return error_msg(ERROR_BAD_REQUEST);
	if (!limit)
		limit = API_BOARD_TOC_LIMIT_DEFAULT;

	int flag = *web_get_param("flag");

	xml_node_t *root = set_response_root("bbs-board-toc",
			XML_NODE_ANONYMOUS_JSON, XML_ENCODING_UTF8);
	xml_node_t *posts = xml_new_child(root, "posts", XML_NODE_CHILD_ARRAY);

	post_filter_t filter = {
		.bid = board.id, .flag = flag,
		.min = asc ? start : 0, .max = asc ? 0 : start,
	};
	query_t *q = build_post_query(&filter, asc, limit);
	db_res_t *res = query_exec(q);
	print_toc(posts, res, filter.archive, asc);
	db_clear(res);

	if (!start && !asc && !flag && board.id)
		print_toc_sticky(root, board.id);

	xml_node_t *bnode = xml_new_child(root, "board", 0);
	board_to_node(&board, bnode);
	return HTTP_OK;
}
#endif

#define BASEURL BBSHOST"/bbs"

enum {
	MAXRSS = 10, ///< max. number of posts output
};

typedef struct {
	post_index_record_t *pir;
	int bid;
	int remain;
} print_topics_t;

static record_callback_e print_topics(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	print_topics_t *pt = args;

	if (!pib->tid_delta) {
		post_info_t pi;
		post_index_board_to_info(pt->pir, pib, &pi, 1);

		printf("<item><title>");
		xml_fputs(pi.utf8_title);
		printf("</title><link>http://"BASEURL"/con?bid=%d&amp;f=%u</link>"
				"<author>%s</author><pubDate>%s</pubDate><source>%s</source>"
				"<guid>http://"BASEURL"/con?bid=%d&amp;f=%u</guid>"
				"<description><![CDATA[<pre>", pt->bid, pib->id, pi.owner,
				format_time(pi.stamp, TIME_FORMAT_RSS), pi.owner, pt->bid,
				pib->id);

		char *content = post_content_get(pi.id);
		if (content)
			xml_fputs(content);
		free(content);

		printf("<pre>]]></description></item>");
		if (--pt->remain <= 0)
			return RECORD_CALLBACK_BREAK;
	}
	return RECORD_CALLBACK_CONTINUE;
}

int bbsrss_main(void)
{
	board_t board;
	if (!get_board_by_param(&board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;
	session_set_board(board.id);

	printf("Content-type: text/xml; charset=utf-8\n\n"
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			"<rss version='2.0'><channel><title>%s</title><description>%s"
			"</description><link>"BASEURL "/doc?bid=%d</link><generator>"
			BASEURL "</generator>", board.name, board.descr, board.id);

	post_index_record_t pir;
	post_index_record_open(&pir);
	record_t record;
	post_index_board_open(board.id, RECORD_READ, &record);

	print_topics_t pt = { .pir = &pir, .bid = board.id, .remain = MAXRSS };
	record_reverse_foreach(&record, print_topics, &pt);

	record_close(&record);
	post_index_record_close(&pir);

	printf("</channel></rss>");
	return 0;
}

int api_top10(void)
{
	bool api = web_request_type(API);
	if (api && !web_request_method(GET))
		return WEB_ERROR_METHOD_NOT_ALLOWED;

	xml_node_t *node;
	if (!api) {
		xml_header(NULL);
		printf("<bbstop10>");
		print_session();
	} else {
		node = set_response_root("bbs_top10",
				XML_NODE_ANONYMOUS_JSON, XML_ENCODING_UTF8);
		node = xml_new_child(node, "topics", XML_NODE_CHILD_ARRAY);
	}

	FILE *fp = fopen("etc/posts/day_f.data", "r");
	if (fp) {
		int rank = 0;
		topic_stat_t topic;
		while (fread(&topic, sizeof(topic), 1, fp) == 1) {
			if (api) {
				xml_node_t *n = xml_new_child(node, "topic",
						XML_NODE_ANONYMOUS_JSON);
				xml_attr_integer(n, "rank", ++rank);
				xml_attr_string(n, "owner", topic.owner, false);
				xml_attr_string(n, "board", topic.bname, false);
				xml_attr_integer(n, "bid", topic.bid);
				xml_attr_integer(n, "count", topic.count);
				xml_attr_bigint(n, "tid", topic.tid);
				xml_attr_string(n, "title", topic.utf8_title, false);
			} else {
				printf("<top board='%s' owner='%s' count='%u' gid='%"PRIdPID"'>",
						topic.bname, topic.owner, topic.count, topic.tid);
				if (web_request_type(UTF8)) {
					xml_fputs(topic.utf8_title);
				} else {
					GBK_BUFFER(title, POST_TITLE_CCHARS);
					convert_u2g(topic.utf8_title, gbk_title);
					xml_fputs(gbk_title);
				}
				printf("</top>\n");
			}
		}
	}
	if (api) {
		return WEB_OK;
	} else {
		printf("</bbstop10>");
		return 0;
	}
}
