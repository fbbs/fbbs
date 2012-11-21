#include "libweb.h"
#include "fbbs/dbi.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

int bbstop10_main(void)
{
	xml_header(NULL);
	printf("<bbstop10>");
	print_session();

	query_t *q = query_new(0);
	query_select(q, "tid, score, uname, bname, title");
	query_from(q, "posts.hot");
	query_orderby(q, "score", false);
	query_limit(q, 10);

	db_res_t *res = query_exec(q);
	int rows = db_res_rows(res);
	for (int i = 0; i < rows; ++i) {
		post_id_t tid = db_get_post_id(res, i, 0);
		int score = db_get_integer(res, i, 1);
		const char *uname = db_get_value(res, i, 2);
		const char *bname = db_get_value(res, i, 3);
		const char *title = db_get_value(res, i, 4);

		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(title, gbk_title);

		printf("<top board='%s' owner='%s' count='%d' gid='%"PRIdPID"'>",
				bname, uname, score, tid);
		xml_fputs2(gbk_title, check_gbk(gbk_title) - gbk_title, stdout);
		printf("</top>\n");
	}
	printf("</bbstop10>");
	return 0;
}
