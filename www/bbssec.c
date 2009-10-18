#include "libweb.h"

// Read board lists from file 'filename' and print line by line.
static int showbrdlist(const char *filename)
{
	struct boardheader *x;
	char board[STRLEN];
	FILE *fp;
	int showed = 0;

	fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	while (fgets(board, sizeof(board) - 1, fp)) {
		strtok(board,"\t\r\n ");
		if (board[0] == '\0')
			continue;
		x = getbcache(board);
		if (!x)
			return showed;
		// TODO: Magic number.
		printf("<brd name='%s' desc='%s' />", board, x->title + 10);
		showed++;
	}
	fclose(fp);
	return showed;
}

int bbssec_main(void)
{
	char path[HOMELEN];
	xml_header("bbssec");
	printf("<bbssec %s>", get_session_str());
	for(int i = 0; i < SECNUM; i++) {
		printf("<sec id='%X' desc='%s %s'>", i, secname[i][0], secname[i][1]);
		sprintf(path, "%s/info/egroup%d/recommend", BBSHOME, i);
		if(dashf(path))
			showbrdlist(path);
		printf("</sec>");
	}
	printf("</bbssec>");
	return 0;
}
