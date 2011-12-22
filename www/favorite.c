#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

int web_fav(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	xml_header(NULL);
	printf("<bbsfav>");
	print_session();

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) == 0) {
		struct goodbrdheader *iter, *end;
		int num = m.size / sizeof(struct goodbrdheader);
		if (num > GOOD_BRC_NUM)
			num = GOOD_BRC_NUM;
		end = (struct goodbrdheader *)m.ptr + num;
		for (iter = m.ptr; iter != end; ++iter) {
			if (!gbrd_is_custom_dir(iter)) {
				struct boardheader *bp = bcache + iter->pos;
				printf("<brd bid='%d' brd='%s'>", iter->pos + 1, bp->filename);
				xml_fputs(get_board_desc(bp), stdout);
				printf("</brd>");
			}
		}
		mmap_close(&m);
	}
	printf("</bbsfav>");
	return 0;
}

int web_brdadd(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	board_to_gbk(&board);

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	FILE *fp = fopen(file, "a+");
	if (fp == NULL)
		return BBS_EINTNL;
	fb_flock(fileno(fp), LOCK_EX);

	struct goodbrdheader gbrd;
	bool found = false;
	while (fread(&gbrd, sizeof(gbrd), 1, fp) == 1) {
		if (gbrd.pos == board.id - 1) {
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
			gbrd.pos = board.id - 1;
			memcpy(gbrd.title, board.descr, sizeof(gbrd.title));
			memcpy(gbrd.filename, board.name, sizeof(gbrd.filename));
			if (fwrite(&gbrd, sizeof(gbrd), 1, fp) != 1)
				ret = BBS_EINTNL;
		} else {
			ret = BBS_EBRDQE;
		}
	}
	fb_flock(fileno(fp), LOCK_UN);
	fclose(fp);

	if (ret)
		return ret;
	xml_header(NULL);
	printf("<bbsbrdadd>");
	print_session();
	printf("<brd>%s</brd><bid>%d</bid></bbsbrdadd>", board.name, board.id);
	return 0;
}

// TODO: Handle user-defined directories.
static int read_submit(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (parse_post_data(ctx.r) < 0)
		return BBS_EINVAL;

	// Read parameters.
	bool boards[MAXBOARD] = {0};
	int num = 0;
	for (int i = 0; i < ctx.r->count; i++) {
		if (!strcasecmp(ctx.r->params[i].val, "on")) {
			int bid = strtol(ctx.r->params[i].key, NULL, 10);
			if (bid > 0 && bid <= MAXBOARD
					&& hasreadperm(&currentuser, bcache + bid - 1)) {
				boards[bid - 1] = true;
				++num;
			}
		}
	}
	if (num > GOOD_BRC_NUM)
		return BBS_EBRDQE;
	if (num <= 0)
		return BBS_EINVAL;

	// Read '.goodbrd'.
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDWR | O_CREAT;
	if (mmap_open(file, &m) < 0)
		return BBS_ENOFILE; // TODO: empty?
	if (mmap_truncate(&m, num * sizeof(struct goodbrdheader)) < 0) {
		return BBS_EINTNL;
	}
	struct goodbrdheader *iter, *end;
	end = (struct goodbrdheader *)m.ptr + num;

	// Remove deselected boards.
	struct goodbrdheader *dst = m.ptr;
	int id = 0;
	for (iter = m.ptr; iter != end; ++iter) {
		if (boards[iter->pos] == true) {
			boards[iter->pos] = false;
			id++;
			if (iter != dst) {
				iter->id = id;
				iter->pid = 0;
				*dst = *iter;
			}
			++dst;
		}
	}

	// Write out newly selected boards.
	for (int i = 0; i < MAXBOARD; ++i) {
		if (boards[i] == true) {
			id++;
			if (id > GOOD_BRC_NUM)
				break;
			dst->id = num;
			dst->pid = 0;
			dst->pos = i;
			dst->flag = bcache[i].flag;
			strlcpy(dst->filename, bcache[i].filename, sizeof(dst->filename));
			strlcpy(dst->title, bcache[i].title, sizeof(dst->title));
			++dst;
		}
	}
	mmap_close(&m);
	xml_header(NULL);
	printf("<bbsmybrd limit='%d' selected='%d'>", GOOD_BRC_NUM, num);
	print_session();
	printf("</bbsmybrd>");
	return 0;
}

int web_mybrd(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int type = strtol(get_param("type"), NULL, 10);
	if (type != 0)
		return read_submit();

	// Print 'bid's of favorite boards.
	xml_header(NULL);
	printf("<bbsmybrd ");
	printf(" limit='%d'>", GOOD_BRC_NUM);

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) == 0) {
		struct goodbrdheader *iter, *end;
		int num = m.size / sizeof(struct goodbrdheader);
		if (num > GOOD_BRC_NUM)
			num = GOOD_BRC_NUM;
		end = (struct goodbrdheader *)m.ptr + num;
		for (iter = m.ptr; iter != end; iter++) {
			if (!gbrd_is_custom_dir(iter))
				printf("<my bid='%d'/>", iter->pos + 1);
		}
		mmap_close(&m);
	}

	// Print all boards available.
	struct boardheader *b;
	for (int i = 0; i < MAXBOARD; i++) {
		b = bcache + i;
		if (b->filename[0] <= 0x20 || b->filename[0] > 'z')
			continue;
		if (!hasreadperm(&currentuser, b))
			continue;
		printf("<mbrd bid='%d' desc='%s' name='%s' %s/>", i + 1,
				b->title + 11, b->filename,
				is_board_dir(b) ? "dir='1'" : "");
	}
	print_session();
	printf("</bbsmybrd>");
	return 0;	
}
