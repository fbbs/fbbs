#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/web.h"

static int show_sector(int sid, db_res_t *res, int last)
{
	for (int i = last + 1; i < db_res_rows(res); ++i) {
		int sector = db_get_integer(res, i, 2);
		if (sector == sid) {
			last = i;

			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			convert_u2g(db_get_value(res, i, 1), gbk_descr);
			printf("<brd name='%s' desc='%s'/>",
					db_get_value(res, i, 0), gbk_descr);
		}
	}
	return last;
}

int bbssec_main(void)
{
	xml_header(NULL);
	printf("<bbssec>");
	print_session();

	db_res_t *r1 = db_exec_query(env.d, true,
			"SELECT id, name, descr, short_descr"
			" FROM board_sectors ORDER BY name ASC");
	if (!r1)
		return BBS_EINVAL;

	db_res_t *res = db_query("SELECT b.name, b.descr, b.sector"
			" FROM boards b JOIN board_sectors s ON b.section = s.id"
			" WHERE b.flag & %d <> 0", BOARD_RECOMMEND_FLAG);
	if (!res) {
		db_clear(r1);
		return BBS_EINVAL;
	}

	int last = -1;
	for (int i = 0; i < db_res_rows(r1); ++i) {
		printf("<sec id='%s' desc='%s [%s]'", db_get_value(r1, i, 1),
				db_get_value(r1, i, 2), db_get_value(r1, i, 3));
		int sid = db_get_integer(r1, i, 0);
		last = show_sector(sid, res, last);
		printf("</sec>");
	}

	db_clear(res);
	db_clear(r1);

	printf("</bbssec>");
	return 0;
}
