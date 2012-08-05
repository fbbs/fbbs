#define _GNU_SOURCE
#include "libweb.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

enum {
	BFIND_MAX = 100,
	TOPICS_PER_PAGE = 13,
};

typedef struct criteria_t {
	int bid;
	bool mark;
	bool nore;
	time_t limit;
	const char *t1;
	const char *t2;
	const char *t3;
	const char *user;
	int found;
} criteria_t;

extern const struct fileheader *dir_bsearch(const struct fileheader *begin,
        const struct fileheader *end, unsigned int fid);

bool allow_reply(const struct fileheader *fh)
{
	if (fh == NULL || fh->accessed[0] & FILE_NOREPLY)
		return false;
	return true;
}

int get_post_mark(const struct fileheader *fh, bool unread)
{
	int mark = ' ';
	if (fh == NULL)
		return mark;
	if (fh->accessed[0] & FILE_DIGEST)
		mark = 'g';
	if (fh->accessed[0] & FILE_MARKED) {
		if (mark == ' ')
			mark = 'm';
		else
			mark = 'b';
	}
	if (fh->accessed[0] & FILE_DELETED && mark == ' ')
		mark = 'w';
	if (unread) {
		if (mark == ' ')
			mark = '+';
		else
			mark = toupper(mark);
	}
	return mark;
}

static bool select_bbsdoc(const struct fileheader *fh, int mode)
{
	switch (mode) {
		case MODE_THREAD:
		case MODE_TOPICS:
			return (fh->id == fh->gid);
		default:
			break;
	}
	return true;
}

static int print_bbsdoc(const struct fileheader *fh, int count, int mode)
{
	int mark;
	const struct fileheader *end = fh + count;
	for (; fh < end; ++fh) {
		mark = get_post_mark(fh, brc_unread(fh->filename));
		printf("<po %s%sm='%c' owner='%s' time= '%s' id='",
				mode == MODE_NOTICE ? "sticky='1' " : "",
				allow_reply(fh) ? "" : "nore='1' ", mark,
				fh->owner, getdatestring(getfiletime(fh), DATE_XML));
		if (mode != MODE_DIGEST)
			printf("%u'>", fh->id);
		else
			printf("%s'>", fh->filename);
		xml_fputs2(fh->title, check_gbk(fh->title) - fh->title, stdout);
		printf("</po>\n");
   	}
	return count;
}

// Read 'count' files starting from 'start'(origin 1) in index 'dir' and 
// prints file information in XML format.
// Returns files in total on success, -1 on error.
static int get_bbsdoc(const char *dir, int *start, int count, int mode)
{
	int total = -1;
	struct fileheader **array = malloc(sizeof(struct fileheader *) * count);
	if (array == NULL)
		return -1;
	memset(array, 0, sizeof(*array) * count);
	struct fileheader *begin = malloc(sizeof(struct fileheader) * count);
	if (begin == NULL) {
		free(array);
		return -1;
	}
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(dir, &m) == 0) {
		total = m.size / sizeof(struct fileheader);
		struct fileheader *fh = m.ptr;
		struct fileheader *end = fh + total;
		int all = 0;
		if (mode != MODE_NORMAL && mode != MODE_DIGEST) {
			total = 0;
			for (; fh != end; ++fh) {
				if (select_bbsdoc(fh, mode)) {
					total++;
					if (*start > 0 && total >= *start + count)
						continue;
					array[all++] = fh;
					if (all == count)
						all = 0;
				}
			}
		}
		// Check range of 'start' and 'count'.
		if (*start <= 0 || *start > total - count)
			*start = total - count + 1;
		if (*start < 1) {
			*start = 1;
			count = total;
			all = 0;
		}
		// Copy index to allocated area. For more concurrency.
		if (mode != MODE_NORMAL && mode != MODE_DIGEST) {
			for (int i = 0; i < count; i++) {
				memcpy(begin + i, array[all++], sizeof(*begin));
				if (all == count)
					all = 0;
			}
		} else {
			fh = (struct fileheader *)m.ptr + *start - 1;
			memcpy(begin, fh, sizeof(struct fileheader) * count);
		}
		mmap_close(&m);
		print_bbsdoc(begin, count, mode);
	}
	free(begin);
	free(array);
	return total;
}

extern int web_sector(void);

static void _print_board_img(const char *board)
{
	char path[HOMELEN];

	snprintf(path, sizeof(path), "%s/info/boards/%s/icon.jpg", BBSHOME, board);
	if (dashf(path))
		printf(" icon='../info/boards/%s/icon.jpg'", board);

	sprintf(path, "%s/info/boards/%s/banner.jpg", BBSHOME, board);
	if (dashf(path))
		printf(" banner='../info/boards/%s/banner.jpg'", board);
}

static int bbsdoc(int mode)
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

	char dir[HOMELEN];
	switch (mode) {
		case MODE_DIGEST:
			setbfile(dir, board.name, DIGEST_DIR);
			break;
		default:
			setbfile(dir, board.name, DOT_DIR);
			break;
	}

	int start = strtol(get_param("start"), NULL, 10);

	int my_t_lines = strtol(get_param("my_t_lines"), NULL, 10);
	if (my_t_lines < 10 || my_t_lines > 40)
		my_t_lines = TLINES;

	if (get_doc_mode() != mode)
		set_doc_mode(mode);

	xml_header(NULL);
	printf("<bbsdoc>");
	print_session();
	brc_fcgi_init(currentuser.userid, board.name);
	int total = get_bbsdoc(dir, &start, my_t_lines, mode);

	if (mode == MODE_NORMAL) {
		setbfile(dir, board.name, NOTICE_DIR);
		mmap_t m = { .oflag = O_RDONLY };
		if (mmap_open(dir, &m) == 0) {
			int count = m.size / sizeof(struct fileheader);
			print_bbsdoc(m.ptr, count, MODE_NOTICE);
			mmap_close(&m);
		}
	}

	char *cgi_name = "";
	switch (mode) {
		case MODE_DIGEST:
			cgi_name = "g";
			break;
		case MODE_THREAD:
			cgi_name = "t";
			break;
	}

	printf("<brd title='%s' desc='%s' bm='%s' total='%d' start='%d' "
			"bid='%d' page='%d' link='%s'", board.name, board.descr, board.bms,
			total, start, board.id, my_t_lines, cgi_name);
	_print_board_img(board.name);
	printf("/>\n</bbsdoc>");

	// TODO: marquee, recommend
	return 0;
}

int bbsdoc_main(void)
{
	return bbsdoc(MODE_NORMAL);
}

int bbsgdoc_main(void)
{
	return bbsdoc(MODE_DIGEST);
}

int bbstdoc_main(void)
{
	return bbsdoc(MODE_THREAD);
}

int bbsodoc_main(void)
{
	return bbsdoc(MODE_TOPICS);
}

int do_bfind(void *buf, int count, void *args)
{
	struct fileheader *fp = buf;
	criteria_t *cp = args;
	time_t date = getfiletime(fp);
	if (date < cp->limit)
		return QUIT;
	if (cp->nore && fp->id != fp->gid)
		return 0;
	if (cp->mark && !(fp->accessed[0] & FILE_DIGEST)
			&& !(fp->accessed[0] & FILE_MARKED))
		return 0;
	if (*cp->user && strcasecmp(fp->owner, cp->user))
		return 0;
	if (*cp->t1 && !strcasestr(fp->title, cp->t1))
		return 0;
	if (*cp->t2 && !strcasestr(fp->title, cp->t2))
		return 0;
	if (*cp->t3 && strcasestr(fp->title, cp->t3))
		return 0;
	print_bbsdoc(fp, 1, MODE_NORMAL);
	if (++cp->found >= BFIND_MAX)
		return QUIT;
	return 0;
}

int bbsbfind_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	criteria_t cri;
	cri.bid = strtol(get_param("bid"), NULL, 10);

	board_t board;
	if (!get_board_by_bid(cri.bid, &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;

	cri.mark = false;
	if (!strcasecmp(get_param("mark"), "on"))
		cri.mark = true;
	cri.nore = false;
	if (!strcasecmp(get_param("nore"), "on"))
		cri.nore = true;
	long day = strtol(get_param("limit"), NULL, 10);
	if (day < 0)
		day = 0;
	cri.limit = time(NULL) - 24 * 60 * 60 * day;
	cri.t1 = get_param("t1");
	cri.t2 = get_param("t2");
	cri.t3 = get_param("t3");
	cri.user = get_param("user");
	cri.found = 0;

	xml_header(NULL);
	printf("<bbsbfind ");
	printf(" bid='%d'", cri.bid);

	if (*cri.t1 || *cri.t2 || *cri.t3 || *cri.user) {
		printf(" result='1'>");
		char file[HOMELEN];
		setwbdir(file, board.name);
		apply_record(file, do_bfind, sizeof(struct fileheader), &cri, false,
				true, true);
	} else {
		printf(">");
	}
	print_session();
	printf("</bbsbfind>");
	return 0;
}

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
	tp->mark = get_post_mark(fp, brc_unread(fp->filename));
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
	_print_board_img(board.name);
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
