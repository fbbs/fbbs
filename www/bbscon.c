#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

enum {
	POST_FIRST = 1 << 1,
	POST_LAST = 1 << 2,
	THREAD_FIRST_POST = 1 << 3,
	THREAD_LAST_POST = 1 << 4,
	THREAD_LAST = 1 << 5,
	THREAD_FIRST = 1 << 6,
	NOT_THREAD_LAST_POST = 1 << 7,
};

typedef struct {
	char flags;
	char action;
	bool found;
	int offset;
	post_id_t pid;
	post_id_t tid;
	post_info_t *pi;
	post_index_record_t *pir;
	post_index_board_t pib;
} bbscon_search_callback_t;

static int save_result(const post_index_board_t *pib,
		bbscon_search_callback_t *bsc, int off)
{
	bsc->found = true;
	bsc->offset = off;
	bsc->tid = pib->id - pib->tid_delta;
	memcpy(&bsc->pib, pib, sizeof(bsc->pib));
	return post_index_board_to_info(bsc->pir, pib, bsc->pi, 1);
}

static record_callback_e search_pid_callback(void *ptr, void *args, int off)
{
	const post_index_board_t *pib = ptr;
	bbscon_search_callback_t *bsc = args;

	if (!bsc->action) {
		if (pib->id <= bsc->pid) {
			if (pib->id == bsc->pid)
				save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
	} else if (bsc->action == 'p') {
		if (bsc->offset >= 0) {
			save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pib->id == bsc->pid)
			bsc->offset = off;
	} else if (bsc->action == 'b') {
		if (pib->id <= bsc->tid) {
			if (pib->id == bsc->tid)
				save_result(pib, bsc, off);
			return RECORD_CALLBACK_BREAK;
		}
		if (pib->id == bsc->pid)
			bsc->tid = pib->id - pib->tid_delta;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static record_callback_e search_next(void *ptr, void *args, int offset)
{
	const post_index_board_t *pib = ptr;
	bbscon_search_callback_t *bsc = args;

	if (bsc->action == 'n') {
		if (offset > bsc->offset) {
			save_result(pib, bsc, offset);
			return RECORD_CALLBACK_BREAK;
		}
	} else if (bsc->action == 'a') {
		if (pib->id > bsc->pid && pib->id - pib->tid_delta == bsc->tid) {
			save_result(pib, bsc, offset);
			return RECORD_CALLBACK_BREAK;
		}
	} else {
		if (pib->id > bsc->pid && pib->id - pib->tid_delta == bsc->tid) {
			bsc->flags |= NOT_THREAD_LAST_POST;
			return RECORD_CALLBACK_BREAK;
		}
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int search(int bid, post_id_t pid, int action, bool extra,
		post_info_t *pi)
{
	record_t record;
	if (post_index_board_open(bid, RECORD_READ, &record) < 0)
		return -1;

	post_index_record_t pir;
	post_index_record_open(&pir);

	bbscon_search_callback_t bsc = {
		.flags = 0, .action = action, .offset = -1, .pi = pi, .pid = pid,
		.pir = &pir, .tid = 0, .found = false,
	};
	record_reverse_foreach(&record, search_pid_callback, &bsc);

	if (bsc.found) {
		if (action == 'n' || action == 'a' || extra) {
			bsc.found = false;
			record_foreach(&record, &bsc.pib, bsc.offset, search_next, &bsc);
		}
	} else {
		memset(pi, 0, sizeof(*pi));
	}

	bsc.flags |= bsc.found;
	if (bsc.found && extra) {
		if (!bsc.offset)
			bsc.flags |= POST_FIRST;
		if (bsc.offset == record_count(&record) - 1)
			bsc.flags |= POST_LAST;
		if (pi->id == pi->tid)
			bsc.flags |= THREAD_FIRST_POST;
		if (bsc.flags & NOT_THREAD_LAST_POST)
			bsc.flags |= THREAD_LAST_POST;
	}
	post_index_record_close(&pir);
	record_close(&record);
	return bsc.flags;
}

int search_pid(int bid, post_id_t pid, post_info_t *pi)
{
	return search(bid, pid, 0, false, pi);
}

int bbscon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = strtol(get_param("f"), NULL, 10);
	char action = *get_param("a");
//	int archive = strtol(get_param("archive"), NULL, 10);
	bool sticky = *get_param("s");

	int ret = 0;
	post_info_t pi;
	if (sticky) {
		record_t record;
		if (post_index_board_open_sticky(board.id, RECORD_READ, &record) < 0)
			return BBS_ENOFILE;
		// TODO
		record_close(&record);
	} else {
		ret = search(board.id, pid, action, true, &pi);
		if (ret <= 0)
			return BBS_ENOFILE;
	}

	xml_header(NULL);

	bool anony = board.flag & BOARD_ANONY_FLAG;
	int opt = get_user_flag();
	printf("<bbscon link='con' bid='%d' anony='%d' attach='%d'%s%s>",
			board.id, anony, maxlen(board.name),
			opt & PREF_NOSIG ? " nosig='1'" : "",
			opt & PREF_NOSIGIMG ? " nosigimg='1'" : "");

	print_session();

	bool isbm = am_bm(&board);
	bool self = (session.uid == pi.uid);
	printf("<po fid='%"PRIdPID"'%s%s%s%s%s%s", pi.id,
			(pi.flag & POST_FLAG_STICKY) ? " sticky='1'" : "",
			ret & POST_FIRST ? " first='1'" : "",
			ret & POST_LAST ? " last='1'" : "",
			ret & THREAD_LAST_POST ? " tlast='1'" : "",
			(pi.flag & POST_FLAG_LOCKED) ? " nore='1'" : "",
			self || isbm ? " edit='1'" : "");
	printf(" reid='%"PRIdPID"' gid='%"PRIdPID"'>", pi.reid, pi.tid);

	char buffer[4096];
	char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));
	size_t len = strlen(utf8_content);

	char *gbk_content = malloc(len + 1);
	convert(env_u2g, utf8_content, len, gbk_content, len, NULL, NULL);

	xml_print_post_wrapper(gbk_content, strlen(gbk_content));

	free(gbk_content);
	if (utf8_content != buffer)
		free(utf8_content);

	printf("</po></bbscon>");

	brc_initialize(currentuser.userid, board.name);
	brc_mark_as_read(pi.stamp);
	brc_update(currentuser.userid, board.name);

	return 0;
}

int bbsgcon_main(void)
{
	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	const char *f = get_param("f");
	if (strstr(f, "..") || strstr(f, "/") || strncmp(f, "G.", 2))
		return BBS_EINVAL;
	xml_header(NULL);
	printf("<bbscon link='gcon' bid='%d'>", board.id);
	print_session();
	printf("<po>");

	char file[HOMELEN];
	setbfile(file, board.name, f);
	xml_print_post_wrapper(file, 0);

	printf("</po></bbscon>");
	brc_initialize(currentuser.userid, board.name);
	brc_addlist_legacy(f);
	brc_update(currentuser.userid, board.name);
	return 0;
}

int xml_print_post_wrapper(const char *str, size_t size)
{
	if (request_type(REQUEST_PARSED)) {
		int opt = PARSE_NOQUOTEIMG;
		int flag = get_user_flag();
		if (flag & PREF_NOSIG)
			opt |= PARSE_NOSIG;
		if (flag & PREF_NOSIGIMG)
			opt |= PARSE_NOSIGIMG;
		return xml_print_post(str, size, opt);
	}
	if (!request_type(REQUEST_MOBILE)) {
		xml_fputs2(str, size, stdout);
		return 0;
	}
	return xml_print_post(str, size, PARSE_NOSIG);
}
