#include "libweb.h"

int bbsleft_main(void)
{
	xml_header("bbsleft");
	printf("<bbsleft>\n");
	if (loginok) {
		printf("<login>%d</login>\n<talk>%d</talk>\n"
				"<cloak>%d</cloak>\n<find>%d</find>\n",
				loginok, HAS_PERM(PERM_TALK), HAS_PERM(PERM_CLOAK),
				HAS_PERM(PERM_OBOARDS) && HAS_PERM(PERM_SPECIAL0));
		// Favorite boards
		const char *cgi = "doc";
		if (atoi(getparm("my_def_mode")) != 0)
			cgi = "tdoc";
		printf("<favurl>%s</favurl>", cgi);
		char buf[HOMELEN];
		FILE *fp;
		sethomefile(buf, currentuser.userid, ".goodbrd");
		fp = fopen(buf, "rb");
		if (fp != NULL) {
			struct goodbrdheader gbhd;
			int brdcount = 0;
			printf("<favbrd>");
			while (fread(&gbhd, sizeof(struct goodbrdheader), 1, fp)) {
				if (gbhd.flag & BOARD_CUSTOM_FLAG)
					continue;
				if (bcache[gbhd.pos].flag & BOARD_DIR_FLAG)
					printf("<dir>%s</dir>", bcache[gbhd.pos].filename);
				else
					printf("<board>%s</board>", bcache[gbhd.pos].filename);
				if (++brdcount >= GOOD_BRC_NUM)
					break;
			}
			printf("</favbrd>");
		}
	}
	printf("</bbsleft>\n");
	return 0;
}
