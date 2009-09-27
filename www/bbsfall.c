#include "libweb.h"

/**
 * Print override information.
 * @param buf the starting address of override struct.
 * @param count not used.
 * @param args not used.
 * @return 0.
 */
static int print_override(void *buf, int count, void *args)
{
	override_t *ov = buf;
	printf("<ov id='%s'>", ov->id);
	xml_fputs(ov->exp, stdout);
	printf("</ov>");
	return 0;
}

int bbsfall_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header("bbsfall");
	printf("<bbsfall p='%s' u='%s'>", get_permission(), currentuser.userid);
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, "friends");
	apply_record(file, print_override, sizeof(override_t), NULL, false,
			false, true);
	printf("</bbsfall>");
	return 0;
}
