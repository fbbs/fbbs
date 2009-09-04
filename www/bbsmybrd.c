#include "libweb.h"

static bool is_dir(const struct boardheader *bp)
{
	return (bp->flag & BOARD_DIR_FLAG);
}

// TODO: Handle user-defined directories.
static int read_submit(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	parse_post_data();

	// Read parameters.
	bool boards[MAXBOARD] = {0};
	int num = 0;
	for (int i = 0; i < parm_num; i++) {
		if (!strcasecmp(parm_val[i], "on")) {
			int bid = strtol(parm_name[i], NULL, 10);
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
	m.oflag = O_RDWR;
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
	xml_header("bbsmybrd");
	printf("<bbsmybrd><limit>%d</limit><selected>%d</selected></bbsmybrd>",
			GOOD_BRC_NUM, num);
	return 0;
}

int bbsmybrd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int type = strtol(getparm("type"), NULL, 10);
	if (type != 0)
		return read_submit();

	// Read '.goodbrd'.
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

	// Print 'bid's of favorite boards.
	xml_header("bbsmybrd");
	printf("<bbsmybrd p='%s' limit='%d'>", get_permission(), GOOD_BRC_NUM);
	for (iter = m.ptr; iter != end; iter++) {
		if (!gbrd_is_custom_dir(iter))
			printf("<my bid='%d'/>", iter->pos + 1);
	}
	mmap_close(&m);

	// Print all boards available.
	struct boardheader *b;
	for (int i = 0; i < MAXBOARD; i++) {
		b = bcache + i;
		if (b->filename[0] <= 0x20 || b->filename[0] > 'z')
			continue;
		if (!hasreadperm(&currentuser, b))
			continue;
		printf("<b bid='%d' desc='%s' %s/>", i + 1, b->title + 11,
				is_dir(b) ? "dir='1'" : "");
	}
	printf("</bbsmybrd>");
	return 0;	
}
