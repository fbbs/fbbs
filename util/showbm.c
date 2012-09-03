/*
 * show Bms of boards'data at common boards for firebird 2000
 *
 * For short: Ann means announce , BM means Board Manager, Brd means board
 *
 * modified by stephen 2003.03.22
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/cfg.h"
#include "fbbs/helper.h"
#include "showbm.h"

static int articles_BrdBM_AnnSum = -1;      /* the summary num of articales in announce */
static int articles_BrdBM = -1;       /* the summary num of articales of bm on board */
static int lastNarticles_BrdBM = -1;    /* the time no one post on the board */
static time_t lastNarticles_Ann = 0;   /* the last time of announce modified */
static time_t lastbmpost_Brd = 0;      /* the last post time of bm on the board */
static time_t lastpost_Brd = 0;       /* the last time of post on the board */
static int lastNarticles_Brd=-1;

/*
 * function getpath :
 * get announce path of select board from board name,if success return 0
 */
	int
getpath (char *brdname, char *pathname)
{
	FILE *fp;
	char lpathname[256];          /* local pathname */

	sprintf (lpathname, "%s/0Announce/.Search", BBSHOME);
	fp = fopen (lpathname, "r");
	if (fp == NULL)
	{
		fprintf (stderr, "Can't open .Search file.\n");
		return 1;
	}
	do
	{
		fscanf (fp, "%s", brdname);
		brdname[strlen (brdname) - 1] = '\0';
	}
	while (strcmp (brdname, pathname) && !feof (fp));
	fscanf (fp, "%s", brdname);
	fclose (fp);

	return 0;
}

/*
 * function fprintchar:
 * write several chars to file
 */
	void
fprintchar (FILE * fp0, int num, char ch)
{
	int i;
	for (i = 0; i < num; i++)
		fprintf (fp0, "%c", ch);
}

/*
 * function annarticles_BrdBMCount : 
 * get the summary num of articles_BrdBM in announce and the last modified time
 * if success return 0
 */
	int
annarticles_BrdBMCount(char *pathname)
{
	DIR *dp;
	struct dirent *link;

	dp = opendir (pathname);
	if (dp == NULL)
	{
		fprintf (stderr, "Can't open dir %s.\n", pathname);
		return 1;
	}
	while ((link = readdir (dp)) != 0)
	{
		struct stat st;
		char filename[256];

		if (*link->d_name != '.')
			articles_BrdBM_AnnSum++;
		else
			continue;
		sprintf (filename, "%s", pathname);
		strcat (filename, "/");
		strcat (filename, link->d_name);
		stat (filename, &st);
		if (st.st_mtime > lastNarticles_Ann)
			lastNarticles_Ann = st.st_mtime;

		if ((st.st_mode & S_IFMT) == S_IFDIR)
			annarticles_BrdBMCount (filename);
	}
	closedir (dp);

	return 0;
}

/*
 * function lastbmpostime : 
 * the last time of BM's post on the board 
 */
	void
lastBMPostTime (char *pathname, char *filename)
{
	char lpathname[256];
	struct stat st;

	sprintf (lpathname, "%s", pathname);
	strcat (lpathname, filename);
	stat (lpathname, &st);
	if (st.st_mtime > lastbmpost_Brd)
		lastbmpost_Brd = st.st_mtime;
	if (st.st_mtime > time (0) - 60 * 60 * 24 * DAYSN)
		lastNarticles_BrdBM++;
}

/*
 * function lastpostime :
 * get the last post's time on the board 
 * count the num of posts in last N days 
 */
	void
brdLastPostTime (char *pathname, char *filename)
{
	char lpathname[256];
	struct stat st;

	sprintf (lpathname, "%s", pathname);
	strcat (lpathname, filename);
	stat (lpathname, &st);
	if (st.st_mtime > lastpost_Brd)
		lastpost_Brd = st.st_mtime;
	/* count the last N days' post num on the board */
	if (st.st_mtime > time (0) - 60 * 60 * 24 * DAYSN)
		lastNarticles_Brd++;
}

/*
 * function bmfilecount:
 * the summary of articales' num of BM
 * if success return 0
 */
	int
bmfilecount (char *pathname, char *bm)
{
	int fd, index, total;
	char lpathname[256];
	struct fileheader *buffer;
	struct stat st;

	sprintf (lpathname, "%s", pathname);
	lpathname[strlen (lpathname) - 4] = '\0';
	fd = open (pathname, O_RDONLY);
	if (fd == -1)
	{
		fprintf (stderr, "Can not open .DIR file!\n");
		return 1;
	}
	(void) fstat (fd, &st);
	total = st.st_size / sizeof (struct fileheader);
	buffer = (struct fileheader *) calloc (total, sizeof (struct fileheader));
	if (buffer == NULL)
	{
		fprintf (stderr, "Out of memory!\n");
		exit (-1);
	}
	if (read (fd, buffer, (size_t) st.st_size) < (ssize_t) st.st_size)
	{
		free (buffer);
		fprintf (stderr, "Can not read .DIR file!\n");
		return 1;
	}
	(void) close (fd);

	for (index = 0; index < total; index++)
	{
		brdLastPostTime (lpathname, buffer[index].filename);
		if (!bm)
			continue;
		if (!strcmp (buffer[index].owner, bm))
		{
			articles_BrdBM++;
			lastBMPostTime (lpathname, buffer[index].filename);
		}
	}
	free (buffer);

	return 0;
}

/*
 * function getbmrec:
 * get record of bm from .PASSWD从.PASSWD
 * if success return 0
 */
	int
getbmrec (struct userec *bmrec, char *currbm)
{
	FILE *fp;
	char pathname[256];

	sprintf (pathname, "%s/.PASSWDS", BBSHOME);
	fp = fopen (pathname, "rb");
	if (fp == NULL)
	{
		fprintf (stderr, "Error open .PASSWDS file.\n");
		return 1;
	}

	while (1)
	{
		if (fread (bmrec, sizeof (struct userec), 1, fp) <= 0)
			break;
		if (!strcmp (bmrec->userid, currbm))
			break;
	}
	fclose (fp);

	return 0;
}

/*
 * function main:
 * do the main loop to print the info
 */
int main (int argc, char **argv)
{
	char pathname0[256];          /* target path to write data */
	char filename0[20];           /* target file to write data */
	FILE *fp0;                    /* post to announce */

	int fd;         /* init and main loop */

	struct stat st;
	struct userec bmrec;
	static int board_num, bm_num; /* for summary */

	board_num = 0;
	bm_num = 0;

	/* open the target file to write */
	sprintf (pathname0, "%s/0Announce/bmstat/", BBSHOME);
	sprintf (filename0, "M.%ld.A", time (0));
	strcat (pathname0, filename0);

	fp0 = stdout;
#ifndef PRINTOFF
	fprintf (fp0, "%s", head);
#endif

	board_t board;

	initialize_environment(INIT_CONV | INIT_DB);

	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	for (int i = 0; i < db_res_rows(res); ++i) {
		res_to_board(res, i, &board);
		board_to_gbk(&board);

		int count;                /* counter of local board */
		char pathname[256];
		char filename[80];        /* pathname = pathname + filename */
		char boardtitles[STRLEN]; /* source of board's chinese name */
		char boardtitle[STRLEN];  /* fixed chinese name of board */

		char bms[BM_LEN - 1];     /* 给strtok()用 */
		char *bm;                 /* pointer to BM */
		
		if (board.perm)
			continue;               /* limit Read/Post */
		if ((board.flag & BOARD_ANONY_FLAG))
			continue;               /* Anonymous boards */
		sprintf (pathname, "%s/boards/%s/.DIR", BBSHOME,
				board.name);
		fd = open (pathname, O_RDONLY);
		if (fd == -1)
			continue;
		(void) fstat (fd, &st);
		/* count the num on the board */
		count = st.st_size / sizeof (struct fileheader);
		(void) close (fd);

		/* chinese board's name */
		strcpy (boardtitles, board.descr);
		{
			char *p;
			boardtitle[0] = '\0';
			p = strtok (boardtitles, " ");
			p = strtok (NULL, " ");
			p = strtok (NULL, "\0");
			while (*p == ' ')
				p++;
			strcat (boardtitle, p);
		}

		if (strlen (boardtitle) >= 35)
		{
			boardtitle[32] = '.';
			boardtitle[33] = '.';
			boardtitle[34] = '.';
			boardtitle[35] = ')';
			boardtitle[36] = '\0';
		}
		else
		{
			strcat (boardtitle, ")");
			boardtitle[35] = '\0';
		}

		/* no new articles_BrdBM days */
		lastpost_Brd = 0;

		/* num of all articles_BrdBM on the board */
		sprintf (pathname, "%s/0Announce/", BBSHOME);
		if (!getpath (filename, board.name))
		{
			strcat (pathname, filename);
			articles_BrdBM_AnnSum = 0;        /* init */
			lastNarticles_Ann = 0;
			//          if (annarticles_BrdBMCount (pathname))
			//            articles_BrdBM_AnnSum = -1;     /* error */
		}
		/* else cant find Announce .Search file, error announce data. */

		sprintf (pathname, "%s/boards/", BBSHOME);
		strcat (pathname, board.name);
		strcat (pathname, "/.DIR");

		/* BM */
		sprintf (bms, "%s", board.bms);
		bm = strtok (bms, " \t\n");

		lastpost_Brd=0;
		lastNarticles_Brd = 0;
		bmfilecount (pathname, NULL);       

#ifndef PRINTOFF
		fprintf (fp0, "%-16.16s", board.name);

		fprintf (fp0, " %6d", lastNarticles_Brd);

#if 0
		fprintf (fp0, " %6d", articles_BrdBM_AnnSum);
		if ((time (0) - lastNarticles_Ann) / 60 / 60 / 24 > 999)
			fprintf (fp0, " %6d", 999);
		else if ((time (0) - lastNarticles_Ann) / 60 / 60 / 24 > NO0FILEDAYS)
			fprintf (fp0, " %s%6ld%s",REDMARK,(time (0) - lastNarticles_Ann) / 60 / 60 / 24,ENDREDMARK);
		else
			fprintf (fp0, " %6ld", (time (0) - lastNarticles_Ann) / 60 / 60 / 24);
#else
		fprintf(fp0, "      -      -");
#endif
#endif

#ifndef PRINTOFF
		if (bm == NULL)
		{
			fprintf(fp0, " %s%s%s\n", REDMARK, "诚征版主", ENDREDMARK);
			board_num++;
			continue;             /* no bm board */
		}
#endif

		while(bm){
			/* BM's articles_BrdBM on the board */
			articles_BrdBM = 0;
			lastbmpost_Brd = 0;
			lastNarticles_BrdBM = 0;
			bmfilecount (pathname, bm);       /* enable bm == NULL */
			if (getbmrec (&bmrec, bm))break;
			if (!strcmp (bm, "SYSOPs"))break;
			if (!strcmp (bm, "SYSOP"))break;
			bm_num++;

#ifndef PRINTOFF
			fprintf(fp0, " %-12.12s", bm);
			fprintf (fp0, " %6d", lastNarticles_BrdBM);
			fprintf (fp0, " %6d", articles_BrdBM);
			if ((time (0) - bmrec.lastlogin) / 60 / 60 / 24 > 999)
				fprintf (fp0, " %s%6d%s", "", 999," ");
			else if ((time (0) - bmrec.lastlogin) / 60 / 60 / 24 > BMNOLOGIN)
				fprintf (fp0, " %s%6"PRIdFBT"%s",REDMARK,(time (0) - bmrec.lastlogin) / 60 / 60 / 24,ENDREDMARK);
			else
				fprintf (fp0, " %6"PRIdFBT, (time (0) - bmrec.lastlogin) / 60 / 60 / 24);

			if ((time (0) - lastbmpost_Brd) / 60 / 60 / 24 > 999)
				fprintf (fp0, " %6d", 999);
			else if ((time (0) - lastbmpost_Brd) / 60 / 60 / 24 > BMNOFILEDAYS)
				fprintf (fp0, " %s%6ld%s",REDMARK,(time (0) - lastbmpost_Brd) / 60 / 60 / 24,ENDREDMARK);
			else
				fprintf (fp0, " %6ld", (time (0) - lastbmpost_Brd) / 60 / 60 / 24);

			bm=strtok(NULL, " \t\n");
			if(bm)fprintf(fp0, "\n%37.37s", " ");
		}	
		fprintf(fp0, "\n");		
#endif
		board_num++;
	}
	return 0;
}
