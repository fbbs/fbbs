#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/web.h"

int bbsnot_main(void)
{
	board_t board;
	if (!get_board(get_param("board"), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;

	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	char fname[HOMELEN];
	snprintf(fname, sizeof(fname), "vote/%s/notes", board.name);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(fname, &m) < 0)
		return BBS_ENOFILE;
	xml_header(NULL);
	printf("<bbsnot brd='%s'>", board.name);
	xml_fputs((char *)m.ptr, stdout);
	mmap_close(&m);
	print_session();
	printf("</bbsnot>");
	return 0;
}
