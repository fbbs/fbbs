#define STRLEN 80
#define BM_LEN 60
#define IDLEN                12    /* Length of userids */

#include <stdio.h>
#include <time.h>
struct dir {
	char board[20];
	char userid[14];
	char showname[40];
	char exp[80];
	char type[30];
	int filename;
	int date;
	int level;
	int size;
	int live;
	int click;
	int active;
	int accessed;
} old;

struct fileheader {	/* This structure is used to hold data in */
	char	filename[STRLEN-8];     /* the DIR files */
	unsigned int	id;
	unsigned int  	gid;
	char 	owner[STRLEN];
	char 	title[STRLEN-IDLEN-1];
	char 	szEraser[IDLEN+1];
	unsigned 	level;
	unsigned char 	accessed[4];   /* struct size = 256 bytes */
	unsigned int reid;
	time_t 	timeDeleted;
} new;



/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */



main(argc, argv)
	char *argv[];
{
	char oldfile[STRLEN], newfile[STRLEN], buf[255];

	FILE *fin, *fout;
	char *p;
	int i;

	sprintf(oldfile, "/home/bbs/upload/%s/.DIR.old", argv[1]);
	sprintf(newfile, "/home/bbs/upload/%s/.DIR", argv[1]);
	sprintf(buf, "mv -f %s %s", newfile, oldfile);
	system(buf);
	
	if ((fin = fopen(oldfile, "rb")) == NULL)
	{
		printf("Can't open %s .DIR file.\n", argv[1]);
		return 1;
	}
	if ((fout = fopen(newfile, "w+")) == NULL)
	{
		printf("Can't open .DIR.new file.\n", argv[1]);
		return 1;
	}

	for (i=0; ; i++) {
		if (fread(&old, sizeof(old), 1, fin ) <= 0) break;
		memset(&new, 0, sizeof(new));
		strncpy(new.filename ,old.showname, 80);
		strncpy(new.owner, old.userid,20);
		new.id = old.size;
		new.timeDeleted = old.date;
		new.reid = old.active;
		fwrite(&new,sizeof(new),1,fout);
			
	}
	fclose(fout);
	fclose(fin);
	sprintf(buf, "chown bbs:bbs %s", newfile);
	system(buf);

}

