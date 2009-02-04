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
		printf("<board>\n"
				"<url><![CDATA[%s]]></url>\n"
				"<title><![CDATA[%s]]></title>\n"
				"</board>\n",
				board, x->title + 10);
		showed++;
	}
	fclose(fp);
	return showed;
}

int bbssec_main(void)
{
	int i, style;
	char path[HOMELEN];

	xml_header("bbssec");
	
	printf("<bbssec>\n");
	style = loginok ? strtol(getparm("my_style"), NULL, 10) : 0;
	printf("<style>%d</style>\n", style);

	for(i = 0; i < SECNUM; i++) {
		printf("<sector>\n"
				"<name>%X</name>\n"
				"<title><![CDATA[%s %s]]></title>\n", i, secname[i][0], secname[i][1]);
		sprintf(path, "%s/info/egroup%d/recommend", BBSHOME, i);
		if(dashf(path))
		{
			showbrdlist(path);
		}
		printf("</sector>\n");
	}
	printf("</bbssec>\n");
	return 0;
}
