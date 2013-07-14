#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/web.h"

static void get_post_body(const char **begin, const char **end)
{
	const char *ptr = *begin, *e = *end;
	// skip header.
	int n = 3;
	while (ptr != e && n >= 0) {
		if (*ptr == '\n')
			--n;
		++ptr;
	}
	*begin = ptr;

	ptr = e - 2; // skip last '\n'
	while (ptr >= *begin && *ptr != '\n')
		--ptr;
	if (ptr < *begin)
		return;
	//% if (!strncmp(ptr + 1, "\033[m\033[1;36m※ 修改", 17)) {
	if (!strncmp(ptr + 1, "\033[m\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4", 17)) {
		--ptr;
		while (ptr >= *begin && *ptr != '\n')
			--ptr;
		*end = (ptr >= *begin) ? ptr : *begin;
	} else {
		*end = ptr;
	}
}

extern int search_pid(int bid, post_id_t pid, post_info_t *pi);

static int do_bbspst(bool isedit)
{
	if (!session.id)
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_post_perm(&currentuser, &board))
		return BBS_EPST;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = 0;
	post_info_t pi;

	const char *f = get_param("f");
	bool reply = !(*f == '\0');

	if (isedit && !reply)
		return BBS_EINVAL;

	if (reply) {
		pid = strtol(f, NULL, 10);
		if (!pid || !search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;
		if (!isedit && (pi.flag & POST_FLAG_LOCKED))
			return BBS_EPST;
		if (isedit && !am_bm(&board) && session.uid != pi.uid)
			return BBS_EACCES;
	}
	
	xml_header(NULL);
	char path[HOMELEN];
	snprintf(path, sizeof(path), BBSHOME"/upload/%s", board.name);
	bool anony = board.flag & BOARD_ANONY_FLAG;
	printf("<bbspst brd='%s' bid='%d' edit='%d' att='%d' anony='%d'>",
			board.name, board.id, isedit, dashd(path), anony);
	print_session();

	if (reply) {
		printf("<t>");

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(pi.utf8_title, gbk_title);

		ansi_filter(gbk_title, gbk_title);
		xml_fputs2(gbk_title, 0, stdout);

		printf("</t><po f='%lu'>", pid);

		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));
		size_t len = strlen(utf8_content);

		char *gbk_content = malloc(len + 1);
		convert(env_u2g, utf8_content, len, gbk_content, len, NULL, NULL);

		if (isedit) {
			const char *begin = gbk_content;
			const char *end = begin + strlen(gbk_content);
			get_post_body(&begin, &end);
			if (end > begin)
				xml_fputs2(begin, end - begin, stdout);
		} else {
			quote_string(gbk_content, strlen(gbk_content), NULL, QUOTE_AUTO,
					false, xml_fputs3);
		}

		free(gbk_content);
		if (utf8_content != buffer)
			free(utf8_content);

		fputs("</po>", stdout);
	}
	printf("</bbspst>");
	return 0;
}

int bbspst_main(void)
{
	return do_bbspst(false);
}

int bbsedit_main(void)
{
	return do_bbspst(true);
}

/** UTF-8 "[转载]" */
#define CP_MARK_STRING  "[\xe8\xbd\xac\xe8\xbd\xbd]"

int bbsccc_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	parse_post_data();

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	post_id_t pid = strtol(get_param("f"), NULL, 10);

	post_info_t pi;
	if (!search_pid(board.id, pid, &pi))
		return BBS_ENOFILE;

	const char *target = get_param("t");
	if (*target != '\0') {
		board_t to;
		if (!get_board(target, &to))
			return BBS_ENOBRD;
		if ((to.flag & BOARD_DIR_FLAG) || to.id == board.id)
			return BBS_EINVAL;
		if (!has_post_perm(&currentuser, &to))
			return BBS_EPST;

		GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);
		if (strneq(pi.utf8_title, CP_MARK_STRING,
					sizeof(CP_MARK_STRING) - 1)) {
			strlcpy(utf8_title, pi.utf8_title, sizeof(utf8_title));
		} else {
			snprintf(utf8_title, sizeof(utf8_title), CP_MARK_STRING"%s",
					pi.utf8_title);
		}
		convert_u2g(utf8_title, gbk_title);

		char buffer[4096];
		char *content = post_content_get(pi.id, buffer, sizeof(buffer));

		post_request_t pr = {
			.autopost = false, .crosspost = true, .uname = NULL, .nick = NULL,
			.user = &currentuser, .board = &to, .title = gbk_title,
			.content = content, .sig = 0, .ip = mask_host(fromhost),
			.reid = 0, .tid = 0, .locked = false, .marked = false,
			.anony = false,
		};
		int ret = publish_post(&pr);

		if (content != buffer)
			free(content);
		if (!ret)
			return BBS_EINTNL;

		xml_header(NULL);
		printf("<bbsccc t='%ld' b='%ld' f='%u'>",
				to.id, board.id, ret);
		print_session();
		printf("</bbsccc>");
	} else {
		xml_header(NULL);
		printf("<bbsccc owner='%s' brd='%s' bid='%ld' fid='%u'>",
				pi.owner, board.name, board.id, pid);

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(pi.utf8_title, gbk_title);
		xml_fputs(gbk_title, stdout);

		print_session();
		printf("</bbsccc>");
	}
	return 0;
}

/**
 * Forward post.
 * @return 0 on success, bbserrno on error.
 */
// fwd?bid=[bid]&f=[fid]&u=[recipient]
int bbsfwd_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;
	parse_post_data();
	const char *reci = get_param("u");
	if (*reci == '\0') {
		xml_header(NULL);
		printf("<bbsfwd bid='%s' f='%s'>",
				get_param("bid"), get_param("f"));
		print_session();
		printf("</bbsfwd>");
	} else {
		if (!HAS_PERM(PERM_MAIL))
			return BBS_EACCES;

		board_t board;
		if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
				|| !has_read_perm(&currentuser, &board))
			return BBS_ENOBRD;
		if (board.flag & BOARD_DIR_FLAG)
			return BBS_EINVAL;

		post_id_t pid = strtoul(get_param("f"), NULL, 10);
		post_info_t pi;
		if (!search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		GBK_BUFFER(title2, POST_TITLE_CCHARS);
		convert_u2g(pi.utf8_title, gbk_title);
		//% snprintf(gbk_title2, sizeof(gbk_title2), "[转寄]%s", gbk_title);
		snprintf(gbk_title2, sizeof(gbk_title2), "[\xd7\xaa\xbc\xc4]%s", gbk_title);

		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));

		char file[HOMELEN];
		int ret = dump_content_to_gbk_file(utf8_content, CONVERT_ALL, file,
				sizeof(file));

		if (utf8_content != buffer)
			free(utf8_content);

		if (ret == 0) {
			ret = mail_file(file, reci, gbk_title2);
			unlink(file);

			if (ret)
				return ret;
			http_header();
			//% printf("</head><body><p>文章转寄成功</p>"
			printf("</head><body><p>\xce\xc4\xd5\xc2\xd7\xaa\xbc\xc4\xb3\xc9\xb9\xa6</p>"
					//% "<a href='javascript:history.go(-2)'>返回</a>"
					"<a href='javascript:history.go(-2)'>\xb7\xb5\xbb\xd8</a>"
					"</body></html>");
		}
	}
	return 0;
}
