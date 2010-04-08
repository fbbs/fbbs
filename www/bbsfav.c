#include "libweb.h"

int bbsfav_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	// Read '.goodbrd'
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return BBS_ENOFILE;
	struct goodbrdheader *iter, *end;
	int num = m.size / sizeof(struct goodbrdheader);
	if (num > GOOD_BRC_NUM)
		num = GOOD_BRC_NUM;
	end = (struct goodbrdheader *)m.ptr + num;

	// Print all favorite boards.
	xml_header("bbs");
	printf("<bbsfav ");
	print_session();
	printf(">");
	for (iter = m.ptr; iter != end; ++iter) {
		if (!gbrd_is_custom_dir(iter)) {
			struct boardheader *bp = bcache + iter->pos;
			printf("<brd bid='%d' brd='%s'>", iter->pos + 1, bp->filename);
			xml_fputs(get_board_desc(bp), stdout);
			printf("</brd>");
		}
	}
	mmap_close(&m);
	printf("</bbsfav>");
	return 0;
}

int bbsbrdadd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	FILE *fp = fopen(file, "a+");
	if (fp == NULL)
		return BBS_EINTNL;
	flock(fileno(fp), LOCK_EX);

	struct goodbrdheader gbrd;
	bool found = false;
	while (fread(&gbrd, sizeof(gbrd), 1, fp) == 1) {
		if (gbrd.pos == bid - 1) {
			found = true;
			break;
		}
	}

	int ret = 0;
	if (!found) {
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp) / sizeof(gbrd);
		if (size < GOOD_BRC_NUM) {
			gbrd.id = size + 1;
			gbrd.pid = 0;
			gbrd.pos = bid - 1;
			memcpy(gbrd.title, bp->title, sizeof(gbrd.title));
			memcpy(gbrd.filename, bp->filename, sizeof(gbrd.filename));
			if (fwrite(&gbrd, sizeof(gbrd), 1, fp) != 1)
				ret = BBS_EINTNL;
		} else {
			ret = BBS_EBRDQE;
		}
	}
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	if (ret)
		return ret;
	xml_header("bbs");
	printf("<bbsbrdadd ");
	print_session();
	printf("><brd>%s</brd><bid>%d</bid></bbsbrdadd>", bp->filename, bid);
	return 0;
}
