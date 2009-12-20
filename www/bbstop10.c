#include "libweb.h"

enum {
    BOARD_LEN = 18, TITLE_LEN = 62, OWNER_LEN = 16,
};

typedef struct top_t {
    unsigned int gid;
    char board[BOARD_LEN];
    char title[TITLE_LEN];
    char owner[OWNER_LEN];
    int count;
    time_t last;
} top_t;

int bbstop10_main(void)
{
	xml_header("bbstop10");
	printf("<bbstop10 ");
	print_session();
	printf(">");
	top_t top;
	FILE *fp = fopen(BBSHOME"/etc/posts/day.0", "rb");
	if (fp != NULL) {
		for (int i = 0; i < 10; ++i) {
			if (fread(&top, sizeof(top), 1, fp) != 1)
				break;
			printf("<top board='%s' owner='%s' count='%d' gid='%u'>",
					top.board, top.owner, top.count, top.gid);
			xml_fputs(top.title, stdout);
			printf("</top>\n");
		}
		fclose(fp);
	}
	printf("</bbstop10>");
	return 0;
}
