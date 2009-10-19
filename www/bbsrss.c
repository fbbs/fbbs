#include "libweb.h"
#include <time.h>

#define BASEURL BBSHOST"/bbs"

enum {
	MAXRSS = 10, ///< max. number of posts output
};

int bbsrss_main(void)
{
	struct boardheader *bp = getbcache2(strtol(getparm("bid"), NULL, 10));
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (is_board_dir(bp))
		return BBS_EINVAL;

	mmap_t m;
	m.oflag = O_RDONLY;
	char file[HOMELEN];
	setbfile(file, bp->filename, DOT_DIR);
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

	int bid = bp - bcache + 1;
	xml_header("bbsrss");
	printf("<rss version='2.0'><channel><title>%s</title><description>%s"
			"</description><link>"BASEURL"/doc?bid=%d</link><generator>"
			BASEURL"</generator>", bp->filename, bp->title + 11, bid);

	while (--sum >= 0) {
		fp = index + sum;
		setbfile(file, bp->filename, fp->filename);
		printf("<item><title>");
		xml_fputs(fp->title, stdout);
		printf("</title><link>http://"BASEURL"/con?bid=%d&amp;f=%u</link>"
				"<author>%s</author><pubDate>%s</pubDate><source>%s</source>"
				"<guid>http://"BASEURL"/con?bid=%d&amp;f=%u</guid>"
				"<description>", bid, fp->id, fp->owner,
				getdatestring(getfiletime(fp), DATE_RSS), fp->owner, bid,
				fp->id);
		xml_printfile(file, stdout);
		printf("</description></item>");
	}
	printf("</channel></rss>");
	return 0;
}
