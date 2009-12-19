// Handle unread mark

#include "bbs.h"

#define BRC_MAXSIZE     50000
#define BRC_MAXNUM      60
#define BRC_STRLEN      20
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))

static char brc_buf[BRC_MAXSIZE];
static int brc_size, brc_changed = 0;
static char brc_name[BRC_STRLEN];
static int brc_list[BRC_MAXNUM], brc_num;

static char *brc_getrecord(char *ptr, char *name, int *pnum, int *list)
{
	int num;
	char *tmp;
	strlcpy(name, ptr, BRC_STRLEN);
	ptr += BRC_STRLEN;
	num = (*ptr++) & 0xff;
	tmp = ptr + num * sizeof (int);
	if (num > BRC_MAXNUM)
		num = BRC_MAXNUM;
	*pnum = num;
	memcpy (list, ptr, num * sizeof (int));
	return tmp;
}

static char *brc_putrecord(char *ptr, char *name, int num, int *list)
{
	if (num> 0) {
		if (num> BRC_MAXNUM)
			num = BRC_MAXNUM;
		strlcpy (ptr, name, BRC_STRLEN);
		ptr += BRC_STRLEN;
		*ptr++ = num;
		memcpy (ptr, list, num * sizeof (int));
		ptr += num * sizeof (int);
	}
	return ptr;
}

void brc_update(const char *userid, const char *board)
{
	char dirfile[STRLEN], *ptr;
	char tmp_buf[BRC_MAXSIZE], *tmp;
	char tmp_name[BRC_STRLEN];
	int tmp_list[BRC_MAXNUM], tmp_num;
	int fd, tmp_size;
	if (brc_changed == 0) {
		return;
	}
	ptr = brc_buf;
	if (brc_num > 0) {
		ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
	}
	if (1) {
		sethomefile(dirfile, userid, ".boardrc");
		if ((fd = open(dirfile, O_RDONLY)) != -1) {
			tmp_size = read(fd, tmp_buf, sizeof (tmp_buf));
			if (tmp_size > sizeof(tmp_buf) - BRC_ITEMSIZE)
				tmp_size = sizeof(tmp_buf) - BRC_ITEMSIZE*2 + 1;
			close(fd);
		} else {
			tmp_size = 0;
		}
	}
	tmp = tmp_buf;
	while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z')) {
		tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
		if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
			ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
		}
	}
	brc_size = (int) (ptr - brc_buf);

	if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
		ftruncate(fd, 0);
		write(fd, brc_buf, brc_size);
		close(fd);
	}
	brc_changed = 0;
}

int brc_initial(const char *userid, const char *board)
{
	char dirfile[STRLEN], *ptr;
	int fd;
	brc_update (userid, board);
	brc_changed = 0;
	if (brc_buf[0] == '\0') {
		sethomefile(dirfile, userid, ".boardrc");
		if ((fd = open (dirfile, O_RDONLY)) != -1) {
			brc_size = read (fd, brc_buf, sizeof (brc_buf));
			close (fd);
		} else {
			brc_size = 0;
		}
	}
	ptr = brc_buf;
	while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z')) {
		ptr = brc_getrecord (ptr, brc_name, &brc_num, brc_list);
		if (strncmp (brc_name, board, BRC_STRLEN) == 0) {
			return brc_num;
		}
	}
	strlcpy (brc_name, board, BRC_STRLEN);
	brc_list[0] = 1;
	brc_num = 1;
	return 0;
}

void brc_addlist(const char *filename)
{
	int ftime, n, i;
	if (!strcmp(currentuser.userid, "guest"))
		return;
	ftime = atoi(&filename[2]);
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.') {
		return;
	}
	if (brc_num <= 0) {
		brc_list[brc_num++] = ftime;
		brc_changed = 1;
		return;
	}
	for (n = 0; n < brc_num; n++) {
		if (ftime == brc_list[n]) {
			return;
		} else if (ftime > brc_list[n]) {
			if (brc_num < BRC_MAXNUM)
				brc_num++;
			for (i = brc_num - 1; i > n; i--) {
				brc_list[i] = brc_list[i - 1];
			}
			brc_list[n] = ftime;
			brc_changed = 1;
			return;
		}
	}
	if (brc_num < BRC_MAXNUM) {
		brc_list[brc_num++] = ftime;
		brc_changed = 1;
	}
}

int brc_unread(const char *filename)
{
	int ftime, n;
	ftime = atoi (&filename[2]);
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.') {
		return 0;
	}
	if (brc_num <= 0)
	return 1;
	for (n = 0; n < brc_num; n++) {
		if (ftime> brc_list[n]) {
			return 1;
		} else if (ftime == brc_list[n]) {
			return 0;
		}
	}
	return 0;
}

int brc_unread1(int ftime) {
	int n;
	if (brc_num <= 0)
		return 1;
	for (n = 0; n < brc_num; n++) {
		if (ftime > brc_list[n]) {
			return 1;
		} else if (ftime == brc_list[n]) {
			return 0;
		}
	}
	return 0;
}

int brc_clear(int ent, const char *direct, int clearall)
{
	int i, fd, posttime, size;
	char filename[20];
	struct fileheader f_info;

	if (clearall) {
		posttime = time(0) - BRC_MAXNUM + 1;
	}
	else {
		if ((fd = open(direct, O_RDONLY, 0)) == -1)
			return DONOTHING;
		size = sizeof(struct fileheader);
		lseek(fd, (off_t) ((ent - 1) * size), SEEK_SET);
		read(fd, &f_info, size);
		close(fd);
		posttime = atoi(&(f_info.filename[2])) - BRC_MAXNUM + 1;
	}
	for (i = 0; i < BRC_MAXNUM; i++) {
		sprintf(filename, "M.%d.A", posttime++);
		brc_addlist(filename);
	}
	return PARTUPDATE;
}

void brc_zapbuf(int *zbuf)
{
	if (*zbuf > 0 && brc_num > 0)
		*zbuf = brc_list[0];
}

int brc_fcgi_init(const char *user, const char *board)
{
	brc_buf[0] = '\0';
	return brc_initial(user, board);
}
