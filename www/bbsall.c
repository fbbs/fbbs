// Lists all boards.

#include "libweb.h"

int bbsall_main(void)
{
	struct boardheader *x;
	int i;
	xml_header("bbs");
	printf("<bbsall ");
	print_session();
	printf(">");
	for (i = 0; i < MAXBOARD; i++) {
		x = &(bcache[i]);
		if (x->filename[0] <= 0x20 || x->filename[0] > 'z')
			continue;
		if (!hasreadperm(&currentuser, x))
			continue;
		printf("<brd dir='%d' title='%s' cate='%6.6s' desc='%s' bm='%s' />",
				x->flag & BOARD_DIR_FLAG ? 1 : 0, x->filename, x->title + 1,
				get_board_desc(x), x->BM);
	}
	printf("</bbsall>");
	return 0;
}
