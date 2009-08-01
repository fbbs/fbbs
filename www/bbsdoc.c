#include "libweb.h"

bool allow_reply(const struct fileheader *fh)
{
	if (fh == NULL || fh->accessed[0] & FILE_NOREPLY)
		return false;
	return true;
}

int get_post_mark(const struct fileheader *fh, bool has_read)
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
	if (!has_read) {
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
		printf("<post>\n<author>%s</author>\n<time>%s</time>\n",
				fh->owner, getdatestring(getfiletime(fh), DATE_XML));
		if (mode != MODE_DIGEST)
			printf("<id>%u</id><title>", fh->id);
		else
			printf("<id>%s</id><title>", fh->filename);
		xml_fputs(fh->title, stdout);
		printf("</title>\n");
		if (!allow_reply(fh))
			printf("<noreply />\n");
		mark = get_post_mark(fh, brc_unread(fh->filename));
		if (mark != ' ')
			printf("<mark>%c</mark></post>\n", mark);
		else
			printf("</post>\n");
   	}
	return count;
}

// Read 'count' files starting from 'start'(origin 1) in index 'dir' and 
// prints file information in XML format.
// Returns files in total on success, -1 on error.
static int get_bbsdoc(const char *dir, int *start, int count, int mode)
{
	void *ptr;
	size_t size;
	int fd, total = -1;
	struct fileheader **array = malloc(sizeof(struct fileheader *) * count);
	if (array == NULL)
		return -1;
	memset(array, 0, sizeof(*array) * count);
	struct fileheader *begin = malloc(sizeof(struct fileheader) * count);
	if (begin == NULL)
		return -1;
	if ((fd = mmap_open(dir, MMAP_RDONLY, &ptr, &size)) > 0) {
		total = size / sizeof(struct fileheader);
		struct fileheader *fh = (struct fileheader *)ptr;
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
			fh = (struct fileheader *)ptr + *start - 1;
			memcpy(begin, fh, sizeof(struct fileheader) * count);
		}
		mmap_close(ptr, size, fd);
		print_bbsdoc(begin, count, mode);
		return total;
	}
	free(begin);
	free(array);
	return total;
}

static int bbsdoc(int mode)
{
	char board[STRLEN];
	char *bidstr = getparm("bid");
	struct boardheader *bp;
	if (*bidstr == '\0') {
		strlcpy(board, getparm("board"), sizeof(board));
		bp = getbcache(board);
	} else {
		bp = getbcache2(strtol(bidstr, NULL, 10));
		if (bp != NULL)
			strlcpy(board, bp->filename, sizeof(board));
	}
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal("您选择的是一个目录");

	char dir[HOMELEN];
	switch (mode) {
		case MODE_DIGEST:
			setbfile(dir, board, DIGEST_DIR);
			break;
		default:
			setbfile(dir, board, DOT_DIR);
			break;
	}
	int start = strtol(getparm("start"), NULL, 10);
	int my_t_lines = strtol(getparm("my_t_lines"), NULL, 10);
	int bid = getbnum2(bp);
	if (my_t_lines < 10 || my_t_lines > 40)
		my_t_lines = TLINES;
	brc_initial(currentuser.userid, board);

	xml_header("bbsdoc");
	printf("<bbsdoc>\n");
	char path[HOMELEN];
	sprintf(path, "%s/info/boards/%s/icon.jpg", BBSHOME, board);
	if(dashf(path))
		printf("<icon>%s</icon>\n", path);
	sprintf(path, "%s/info/boards/%s/banner.jpg", BBSHOME, board);
	if(dashf(path))	
		printf("<banner>%s</banner>\n", path);
	int total = get_bbsdoc(dir, &start, my_t_lines, mode);
	char *cgi_name = "";
	switch (mode) {
		case MODE_DIGEST:
			cgi_name = "g";
			break;
		case MODE_THREAD:
			cgi_name = "t";
			break;
	}
	// TODO: magic number.
	printf("<title>%s</title>\n<bm>%s</bm>\n<desc>%s</desc>\n"
			"<total>%d</total>\n<start>%d</start>\n<bid>%d</bid>\n"
			"<page>%d</page><link>%s</link></bbsdoc>", bp->filename, bp->BM,
			bp->title + 11, total, start, bid, my_t_lines, cgi_name);
	// TODO: marquee, recommend, spin
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
