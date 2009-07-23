#include "libweb.h"

bool allow_reply(struct fileheader *fh)
{
	if (fh == NULL || fh->accessed[0] & FILE_NOREPLY)
		return false;
	return true;
}

int get_post_mark(struct fileheader *fh, bool has_read)
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

// Read 'count' files starting from 'start'(origin 1) in index 'dir' and 
// prints file information in XML format.
// Returns files in total on success, -1 on error.
int bbsdoc(const char *dir, int *start, int count)
{
	void *ptr;
	size_t size;
	int fd, total, mark;
	if (safe_mmapfile(dir, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, &fd)) {
		total = size / sizeof(struct fileheader);
		if (*start <= 0 || *start > total - count)
			*start = total - count + 1;
		if (*start < 1) {
			*start = 1;
			count = total;
		}
		struct fileheader *fh = (struct fileheader *)ptr + *start - 1;
		struct fileheader *end = fh + count;
		for (; fh < end; ++fh) {
			printf("<post>\n<author>%s</author>\n<time>%s</time>\n"
					"<id>%u</id>\n<title>",
					fh->owner, getdatestring(getfiletime(fh), DATE_XML), fh->id);
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
		end_mmapfile(ptr, size, fd);
		return total;
	}
	return -1;
}

int bbsdoc_main(void)
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
	setbfile(dir, board, DOT_DIR);
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
	int total = bbsdoc(dir, &start, my_t_lines);
	// TODO: magic number.
	printf("<title>%s</title>\n<bm>%s</bm>\n<desc>%s</desc>\n"
			"<total>%d</total>\n<start>%d</start>\n<bid>%d</bid>\n"
			"<page>%d</page></bbsdoc>", bp->filename, bp->BM, bp->title + 11,
			total, start, bid, my_t_lines);
	// TODO: marquee, recommend, spin
	return 0;
}
