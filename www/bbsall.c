// Lists all boards.

#include "libweb.h"

int bbsall_main(void)
{
	struct boardheader *x;
	int i;
	xml_header("bbsall");
	printf("<bbsall p='%s'>", get_permission());
	for (i = 0; i < MAXBOARD; i++) {
		x = &(bcache[i]);
		if (x->filename[0] <= 0x20 || x->filename[0] > 'z')
			continue;
		if (!hasreadperm(&currentuser, x))
			continue;
		printf("<brd dir='%d' title='%s' cate='%6.6s' desc='%s' bm='%s' />",
				x->flag & BOARD_DIR_FLAG ? 1 : 0, x->filename, x->title + 1,
				x->title + 11, x->BM); // TODO: Magic number here.
	}
	printf("</bbsall>");
	return 0;
}
