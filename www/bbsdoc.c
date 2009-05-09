#include "libweb.h"

// Get file time according to its name 's'.
static inline time_t getfiletime(const struct fileheader *f)
{
	return (time_t)strtol(f->filename + 2, NULL, 10);
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
	int total = file_size(dir) / sizeof(struct fileheader);
	int start = strtol(getparm("start"), NULL, 10);
	int my_t_lines = strtol(getparm("my_t_lines"), NULL, 10);
	int bid = getbnum2(bp);
	if (my_t_lines < 10 || my_t_lines > 40)
		my_t_lines = TLINES;
	if (strlen(getparm("start")) == 0 || start > total - my_t_lines)
		start = total - my_t_lines + 1;
  	if (start < 1)
		start = 1;
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

	// TODO: magic number.
	printf("<title>%s</title>\n<bm>%s</bm>\n<desc>%s</desc>\n"
			"<total>%d</total>\n<start>%d</start>\n<bid>%d</bid>\n",
			bp->filename, bp->BM, bp->title + 11, total, start, bid);

	// TODO: marquee, recommend, spin
	FILE *fp = fopen(dir, "r");
	if (fp != NULL) {
		fseek(fp, (start - 1) * sizeof(struct fileheader), SEEK_SET);
		struct fileheader x;
		int type;
		for (int i = 0; i < my_t_lines; i++) {
			type = ' ';
			if (fread(&x, sizeof(x), 1, fp) <= 0)
				break;
			printf("<post>\n<author>%s</author>\n<time>%s</time>\n"
					"<id>%u</id>\n<title>",
					x.owner, getdatestring(getfiletime(&x), DATE_XML), x.id);
			xml_fputs(x.title, stdout);
			printf("</title>\n");
			if (x.accessed[0] & FILE_NOREPLY)
				printf("<noreply />\n");
			// Mark
			if (x.accessed[0] & FILE_DIGEST)
				type = 'g';
			if (x.accessed[0] & FILE_MARKED) {
				if (type == ' ')
					type = 'm';
				else
					type = 'b';
			}
			if (x.accessed[0] & FILE_DELETED && type == ' ')
				type = 'w';
			if (brc_unread(x.filename)) {
				if (type == ' ')
					type = '+';
				else
					type = toupper(type);
			}
			if (type != ' ')
				printf("<mark>%c</mark></post>\n", type);
			else
				printf("</post>\n");
     	}
		fclose(fp);
	}
	printf("</bbsdoc>");
	return 0;
}
