#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fbbs.h"
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

int get_post_mark(bool marked, bool digest, bool water, bool unread)
{
	int mark = ' ';

	if (digest) {
		if (marked)
			mark = 'b';
		else
			mark = 'g';
	} else if (marked) {
		mark = 'm';
	}

	if (water && mark == ' ')
		mark = 'w';

	if (unread) {
		if (mark == ' ')
			mark = '+';
		else
			mark = toupper(mark);
	}

	return mark;
}

static void print_bbsdoc(const post_info_t *p)
{
	int mark = get_post_mark(p->flag & POST_FLAG_MARKED,
			p->flag & POST_FLAG_DIGEST, false, brc_unread(p->id));

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
	post_list_filter_t filter = { .bid = bid, .type = type };

	int count = _load_sticky_posts(&filter, &sticky_posts);
	for (int i = 0; i < count; ++i) {
		print_bbsdoc(sticky_posts + i);
	}

	free(sticky_posts);
}

static void print_posts(post_list_type_e type, int bid, post_id_t pid,
		int limit)
{
	char query[256];
	build_post_query(query, sizeof(query), type, false, limit + 1);

	if (!pid)
		pid = POST_ID_MAX;
	db_res_t *r = db_query(query, bid, pid);
	if (r) {
		int rows = db_res_rows(r);
		if (rows > limit)
			rows = limit;

		for (int i = rows; i >= 0; --i) {
			post_info_t info;
			res_to_post_info(r, i, &info);
			print_bbsdoc(&info);
		}
		db_clear(r);
	}
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

	int page = strtol(get_param("my_t_lines"), NULL, 10);
	if (page < TLINES || page > 40)
		page = TLINES;

	if (get_doc_mode() != type)
		set_doc_mode(type);

	xml_header(NULL);
	printf("<bbsdoc>");
	print_session();

	brc_fcgi_init(currentuser.userid, board.name);

	print_posts(type, board.id, start, page);
	print_sticky_posts(board.id, type);

	char *cgi_name = "";
	if (type == POST_LIST_DIGEST)
		cgi_name = "g";
	else if (type == POST_LIST_THREAD)
		cgi_name = "t";

	printf("<brd title='%s' desc='%s' bm='%s' start='%d' "
			"bid='%d' page='%d' link='%s'", board.name, board.descr, board.bms,
			start, board.id, page, cgi_name);
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
	return bbsdoc(POST_LIST_THREAD);
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

	char query[512] = "";
	size_t size = sizeof(query);
	char *q = query;

	strappend(&q, &size,
			"SELECT "POST_LIST_FIELDS" FROM posts WHERE board = %%d");

	bool marked = strcaseeq(get_param("mark"), "on");
	if (marked)
		strappend(&q, &size, " AND marked");

	bool locked = strcaseeq(get_param("nore"), "on");
	if (locked)
		strappend(&q, &size, " AND locked");

	long day = strtol(get_param("limit"), NULL, 10);
	if (day < 0)
		day = 0;
	fb_time_t begin = time(NULL) - 24 * 60 * 60 * day;
	strappend(&q, &size, " AND stamp > %%t");

	const char *uname = get_param("user");
	user_id_t uid = get_user_id(uname);
	if (uid) {
		char s[16];
		snprintf(s, sizeof(s), "%"PRIdUID, uid);
		strappend(&q, &size, " AND owner = ");
		strappend(&q, &size, s);
	}

	const char *names[] = { "t1", "t2", "t3" };
	const char *k[] = { NULL, NULL, NULL };
	int count = 0;
	for (int i = 0; i < 3; ++i) {
		const char *s = get_param(names[i]);
		if (*s) {
			k[count++] = s;
			strappend(&q, &size, " AND title ILIKE %%s");
		}
	}

	strappend(&q, &size, " ORDER BY id DESC LIMIT "BFIND_MAX);

	xml_header(NULL);
	printf("<bbsbfind ");
	printf(" bid='%d'", bid);

	if (count || uid) {
		printf(" result='1'>");
		db_res_t *res;
		switch (count) {
			case 0:
				res = db_query(query, bid, begin);
				break;
			case 1:
				res = db_query(query, bid, begin, k[0]);
				break;
			case 2:
				res = db_query(query, bid, begin, k[0], k[1]);
				break;
			default:
				res = db_query(query, bid, begin, k[0], k[1], k[2]);
				break;
		}

		for (int i = 0; i < db_res_rows(res); ++i) {
			post_info_t info;
			res_to_post_info(res, i, &info);
			print_bbsdoc(&info);
		}
	} else {
		printf(">");
	}

	print_session();
	printf("</bbsbfind>");
	return 0;
}

#if 0
typedef struct topic_t {
	unsigned int gid;
	unsigned int lastpage;
	int posts;
	fb_time_t potime;
	fb_time_t uptime;
	char mark;
	char owner[IDLEN + 1];
	char uper[IDLEN + 1];
	char title[STRLEN - IDLEN - 1];
} topic_t;

static void _init_topic(topic_t *tp, const struct fileheader *fp)
{
	strlcpy(tp->owner, fp->owner, sizeof(tp->owner));
	strlcpy(tp->title, fp->title, sizeof(tp->title));
	tp->mark = get_post_mark(fp, brc_unread_legacy(fp->filename));
	tp->potime = getfiletime(fp);
}

static int _find_topic(topic_t *t, int posts, const struct fileheader *fp)
{
	int i, found = -1;
	for (i = 0; i < posts && found < 0; ++i) {
		if (fp->gid == t[i].gid) {
			found = i;
			++t[i].posts;
			if (fp->id == t[i].gid) {
				_init_topic(t + i, fp);
			} else if (t[i].posts == POSTS_PER_PAGE)
				t[i].lastpage = fp->id;
		}
	}
	return found;
}

static void _push_topic(topic_t *t, int posts, const struct fileheader *fp)
{
	topic_t *tp = t + posts;
	tp->gid = fp->gid;
	tp->lastpage = 0;
	tp->posts = 1;
	tp->uptime = getfiletime(fp);
	strlcpy(tp->uper, fp->owner, sizeof(tp->uper));
	tp->owner[0] = '\0';
	if (fp->id == fp->gid) {
		_init_topic(tp, fp);
	} else {
		if (strncmp(fp->title, "Re: ", 4) == 0)
			strlcpy(tp->title, fp->title + 4, sizeof(tp->title));
		else
			strlcpy(tp->title, fp->title, sizeof(tp->title));
	}
}

static topic_t *_get_topics(const char *dir, int *count, unsigned int start,
		unsigned int *next)
{
	topic_t *t = malloc(sizeof(topic_t) * *count);
	if (!t) {
		*count = 0;
		return NULL;
	}

	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(dir, &m) != 0) {
		free(t);
		*count = 0;
		return NULL;
	}

	int posts = 0, finished = 0;
	const struct fileheader *begin = m.ptr, *end = begin + (m.size / sizeof(*end)) - 1;
	if (start)
		end = dir_bsearch(begin, end, start);

	const struct fileheader *fp;
	for (fp = end; fp >= begin && finished < *count; --fp) {
		int i = _find_topic(t, posts, fp);
		if (i >= 0) {
			if (t[i].gid == fp->id)
				++finished;
		} else if (posts < *count) {
			_push_topic(t, posts++, fp);
			if (posts == *count)
				*next = fp->id;
		}
	}

	mmap_close(&m);
	*count = posts;
	return t;
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

	char dir[HOMELEN];
	setbfile(dir, board.name, DOT_DIR);
	int count = TOPICS_PER_PAGE;
	unsigned int next = 0;
	topic_t *t = _get_topics(dir, &count,
			strtoul(get_param("start"), NULL, 10), &next);

	xml_header(NULL);
	printf("<forum title='%s' desc='%s' bm='%s' bid='%d' next='%u'>", 
			board.name, board.descr, board.bms, board.id, next);
	print_board_logo(board.name);
	print_session();

	for (int i = 0; i < count; ++i) {
		printf("<po gid='%u' m='%c' posts='%d'",
				t[i].gid, t[i].mark, t[i].posts);
		if (t[i].owner[0] != '\0') {
			printf(" owner='%s' potime='%s'",
					t[i].owner, getdatestring(t[i].potime, DATE_XML));
		}
		if (t[i].posts > 1) {
			printf(" upuser='%s' uptime='%s'",
				t[i].uper, getdatestring(t[i].uptime, DATE_XML));
		}
		if (t[i].lastpage)
			printf(" lastpage='%u'", t[i].lastpage);
		printf(">");
		xml_fputs2(t[i].title, check_gbk(t[i].title) - t[i].title, stdout);
		printf("</po>\n");
	}
	free(t);
	printf("</forum>");

	return 0;
}
#endif
int web_forum(void)
{
	return 0;
}
