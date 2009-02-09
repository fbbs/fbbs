// Lists all boards.

#include "libweb.h"

int bbsall_main(void)
{
	struct boardheader *x;
	int i;
	xml_header("bbsall");
	printf("<bbsall>\n");
	for (i = 0; i < MAXBOARD; i++) {
		x = &(bcache[i]);
		if (x->filename[0] <= 0x20 || x->filename[0] > 'z')
			continue;
		if (!hasreadperm(&currentuser, x))
			continue;
		printf("<board dir='%d'>\n<title>%s</title>\n<cate>%6.6s</cate>\n"
				"<desc>%s</desc>\n<bm>%s</bm>\n</board>\n",
				x->flag & BOARD_DIR_FLAG ? 1 : 0, x->filename, x->title + 1,
				x->title + 11, x->BM); // TODO: Magic number here.
	}
	printf("</bbsall>");
	return 0;
}
