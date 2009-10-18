#define _GNU_SOURCE
#include "libweb.h"

int bbssel_main(void)
{
	xml_header("bbssel");
	printf("<bbssel %s>", get_session_str());
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
				// TODO: Magic number here.
				printf("<brd dir='%d' title='%s' desc='%s' />",
						is_board_dir(bp), bp->filename, bp->title + 11);
				found++;
			}
		}
		if (!found)
			printf("<notfound />");
	}
	printf("</bbssel>");
	return 0;
}
