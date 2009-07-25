#include "libweb.h"

int bbsdelmail_main(void)
{
	if (!loginok)
		http_fatal("请先登录");
	char file[40];
	strlcpy(file, getparm("f"), sizeof(file));
	if (!valid_mailname(file))
		http_fatal("错误的参数");
	char buf[HOMELEN];
	void *ptr;
	size_t size;
	int fd;
	setmdir(buf, currentuser.userid);
	if ((fd = mmap_open(buf, MMAP_RDWR, &ptr, &size)) < 0)
		http_fatal("索引打开失败");
	struct fileheader *fh = bbsmail_search(ptr, size, file);
	if (fh == NULL) {
		mmap_close(ptr, size, fd);
		http_fatal("信件不存在，可能已被删除");
	}
	struct fileheader *end = (struct fileheader *)ptr + size / sizeof(*fh);
	if (fh < end) {
		memmove(fh, fh + 1, (end - fh) * sizeof(*fh));
		msync(ptr, size, MS_SYNC);
		ftruncate(fd, size - sizeof(*fh));
	}
	mmap_close(ptr, size, fd);
	if (file[0] != 's') {// not shared mail
		setmfile(buf, currentuser.userid, file);
		unlink(buf);
	}
	xml_header("bbsdelmail");
	printf("<bbsdelmail></bbsdelmail>\n");
	return 0;
}

