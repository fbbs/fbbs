#include "libweb.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

static int cmp_fid(const void *key, const void *buf)
{
	unsigned int keyid = ((struct fileheader *)key)->id;
	unsigned int bufid = ((struct fileheader *)buf)->id;
	if (keyid < bufid)
		return -1;
	if (keyid > bufid)
		return 1;
	return 0;
}

int bbsdel_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int bid = strtol(get_param("bid"), NULL, 10);
	unsigned int fid = strtoul(get_param("f"), NULL, 10);
	if (fid == 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_bid(bid, &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	char file[HOMELEN];
	setwbdir(file, board.name);
	record_t r;
	record_open(file, O_RDWR, &r);
	struct fileheader fh;
	fh.id = fid;
	struct fileheader *ptr =
			record_search(&r, &fh, sizeof(fh), bsearch, cmp_fid);
	if (ptr == NULL) {
		record_close(&r);
		return BBS_ENOFILE;
	}
	bool self = !strcmp(ptr->owner, currentuser.userid);
	if (!self && !am_bm(&board)) {
		record_close(&r);
		return BBS_EACCES;
	}
	memcpy(&fh, ptr, sizeof(fh));
	record_delete(&r, ptr, sizeof(*ptr));
	record_close(&r);

	if (!(board.flag & BOARD_JUNK_FLAG)) {
		struct userec user;
		getuserec(fh.owner, &user);
		if (user.numposts > 0)
			user.numposts--;
		save_user_data(&user);
	}

	char buf[STRLEN];
	sprintf(buf, "deleted[www] '%u' on %s\n", fid, board.name);
	report(buf, currentuser.userid);
	strlcpy(fh.szEraser, currentuser.userid, sizeof(fh.szEraser));
	fh.timeDeleted = time(NULL);
	const char *trash = JUNK_DIR;
	if (!self && !HAS_PERM(PERM_OBOARDS)) {
		trash = TRASH_DIR;
	}
	fh.accessed[1] |= FILE_SUBDEL;
	setbfile(file, board.name, trash);
	append_record(file, &fh, sizeof(fh));
	updatelastpost(board.name);

	printf("Location: doc?bid=%d\n\n", bid);
	return 0;
}
