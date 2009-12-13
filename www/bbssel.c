#define _GNU_SOURCE
#include "libweb.h"

int bbssel_main(void)
{
	xml_header("bbssel");
	printf("<bbssel ");
	print_session();
	printf(">");
	char *brd = getparm("brd");
	if (*brd != '\0') {
		struct boardheader *bp;
		int found = 0;
		for (int i = 0; i < MAXBOARD; i++) {
			bp = bcache + i;
			if (!hasreadperm(&currentuser, bp))
				continue;
			if (strcasestr(bp->filename, brd)
					|| strcasestr(bp->title, brd)) {
				printf("<brd dir='%d' title='%s' desc='%s' />",
						is_board_dir(bp), bp->filename, get_board_desc(bp));
				found++;
			}
		}
		if (!found)
			printf("<notfound />");
	}
	printf("</bbssel>");
	return 0;
}
