#include "libweb.h"

int bbsdoc_main(void)
{
	char board[STRLEN];
	strlcpy(board, getparm("board"), sizeof(board));
	struct boardheader *bp = getbcache(board);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal(HTTP_STATUS_BADREQUEST, "您选择的是一个目录");

	char dir[HOMELEN];
	setbfile(dir, board, DOT_DIR);
	int total = file_size(dir) / sizeof(struct fileheader);
	int start = strtol(getparm("start"), NULL, 10);
	int my_t_lines = strtol(getparm("my_t_lines"), NULL, 10);
	if (my_t_lines < 10 || my_t_lines > 40)
		my_t_lines = TLINES;
	if (strlen(getparm("start")) == 0 || start > total-my_t_lines)
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
			"<total>%d</total>\n<start>%d</start>",
			bp->filename, bp->BM, bp->title + 11, total, start);

	// TODO: marquee, recommend, spin
	FILE *fp = fopen(dir, "r");
	if (fp != NULL) {
		fseek(fp, (start - 1) * sizeof(struct fileheader), SEEK_SET);
		int i;
		struct fileheader x;
		for (i = 0; i < my_t_lines; i++) {
			if (fread(&x, sizeof(x), 1, fp) <= 0)
				break;
			printf("<post>\n<author>%s</author>\n"
					"<title>%s</title>\n<id>%ud</id>\n",
					x.owner, x.title, x.id);
			if (x.accessed[0] & FILE_NOREPLY)
				printf("<noreply />\n");
			printf("</post>\n");
			// TODO: time and mark
     	}
	}
	fclose(fp);
	printf("</bbsdoc>");
	return 0;
}
