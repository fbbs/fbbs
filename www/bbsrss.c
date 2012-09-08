#include <time.h>
#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/web.h"

#define BASEURL BBSHOST"/bbs"

enum {
	MAXRSS = 10, ///< max. number of posts output
};

int bbsrss_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	board_to_gbk(&board);

	mmap_t m;
	m.oflag = O_RDONLY;
	char file[HOMELEN];
	setbfile(file, board.name, DOT_DIR);
	if (mmap_open(file, &m) < 0)
		return BBS_EINTNL;

	struct fileheader index[MAXRSS];
	struct fileheader *fp = m.ptr, *begin = m.ptr;
	int count = m.size / sizeof(*fp);
	fp += count - 1;
	int sum;
	for (sum = 0; sum < MAXRSS; ) {
		if (fp >= begin) {
			if (fp->id == fp->gid)
				index[sum++] = *fp;
		} else {
			break;
		}
		--fp;
	}
	mmap_close(&m);

	xml_header("bbsrss");
	printf("<rss version='2.0'><channel><title>%s</title><description>%s"
			"</description><link>"BASEURL "/doc?bid=%d</link><generator>"
			BASEURL "</generator>", board.name, board.descr, board.id);

	while (--sum >= 0) {
		fp = index + sum;
		setbfile(file, board.name, fp->filename);
		printf("<item><title>");
		xml_fputs(fp->title, stdout);
		printf("</title><link>http://"BASEURL"/con?bid=%d&amp;f=%u</link>"
				"<author>%s</author><pubDate>%s</pubDate><source>%s</source>"
				"<guid>http://"BASEURL"/con?bid=%d&amp;f=%u</guid>"
				"<description><![CDATA[<pre>", board.id, fp->id, fp->owner,
				getdatestring(getfiletime(fp), DATE_RSS), fp->owner, board.id,
				fp->id);
		xml_printfile(file);
		printf("<pre>]]></description></item>");
	}
	printf("</channel></rss>");
	return 0;
}
