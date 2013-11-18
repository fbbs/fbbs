//% #define VOTE_FLAG    	0x1			//投票模式
#define VOTE_FLAG    	0x1			//\xcd\xb6\xc6\xb1\xc4\xa3\xca\xbd
#define NOZAP_FLAG   	0x2			//
#define OUT_FLAG     	0x4			//
//% #define ANONY_FLAG   	0x8			//匿名模式
#define ANONY_FLAG   	0x8			//\xc4\xe4\xc3\xfb\xc4\xa3\xca\xbd
#define NOREPLY_FLAG 	0x10 		/* No reply board */
#define BOARDJUNK_FLAG	0x20		//

//% #define BOARD_FLAG_VOTE         0x1             //投票模式
#define BOARD_FLAG_VOTE         0x1             //\xcd\xb6\xc6\xb1\xc4\xa3\xca\xbd
#define BOARD_FLAG_NOZAP        0x2             //no zap
//% #define BOARD_FLAG_OUT          0x4             //转信
#define BOARD_FLAG_OUT          0x4             //\xd7\xaa\xd0\xc5
//% #define BOARD_FLAG_ANONY        0x8             //匿名模式
#define BOARD_FLAG_ANONY        0x8             //\xc4\xe4\xc3\xfb\xc4\xa3\xca\xbd
#define BOARD_FLAG_NOREPLY      0x10            //No reply board
//% #define BOARD_FLAG_JUNK         0x20            //不记文章数
#define BOARD_FLAG_JUNK         0x20            //\xb2\xbb\xbc\xc7\xce\xc4\xd5\xc2\xca\xfd
//% #define BOARD_FLAG_CLUB         0x40            //俱乐部版面
#define BOARD_FLAG_CLUB         0x40            //\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6\xc3\xe6
//% #define BOARD_FLAG_READ         0x80            //隐藏版面
#define BOARD_FLAG_READ         0x80            //\xd2\xfe\xb2\xd8\xb0\xe6\xc3\xe6
#define BOARD_FLAG_POST         0x100           //postmask

#define PERM_POSTMASK  0100000     
#define PERM_NOZAP     02000000

#define STRLEN 80
#define BM_LEN 60

#include <stdio.h>
#include <time.h>

struct ob {		     /* This structure is used to hold data in */
	char filename[STRLEN];   /* the BOARDS files */
	char owner[STRLEN - BM_LEN];
	char BM[ BM_LEN - 1];
	char flag;
	char title[STRLEN ];
	unsigned level;
	unsigned char accessed[ 12 ];
} boardold;

struct  nb{		     /* This structure is used to hold data in */
	char filename[STRLEN];   /* the BOARDS files */
	char owner[STRLEN - BM_LEN];
	char BM[ BM_LEN - 4];
	unsigned int flag;
	char title[STRLEN ];
	unsigned level;
	unsigned char accessed[ 12 ];
} boardnew;


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */



main(argc, argv)
	char *argv[];
{
	char *oldfile = "/home/bbs/.BOARDS";
	char *newfile = "/home/bbs/.BOARDS.new";

	FILE *fin, *fout;
	char buf[40], *p;
	int i;

	if ((fin = fopen(oldfile, "rb")) == NULL)
	{
		printf("Can't open .BOARD file.\n");
		return 1;
	}
	if ((fout = fopen(newfile, "w+")) == NULL)
	{
		printf("Can't open .BOARD.new file.\n");
		return 1;
	}

	for (i=0; ; i++) {
		if (fread(&boardold, sizeof(boardold), 1, fin ) <= 0) break;
		memset(&boardnew, 0, sizeof(boardnew));
		strncpy(boardnew.filename ,boardold.filename, 80);
		printf("%d\nfname:%s\n",i, boardnew.filename);
		
		strncpy(boardnew.owner, boardold.owner,20);
		printf("owner:%s\n", boardnew.owner);
		
		strncpy(boardnew.BM ,boardold.BM, 56);
		printf("BM:%s\n", boardnew.BM);
		
		strncpy(boardnew.title ,boardold.title, 80);
		printf("title:%s\n", boardnew.title);
		
		boardnew.level = boardold.level & (~PERM_NOZAP) & (~PERM_POSTMASK);
		printf("level:%0x\t%0x\n",  boardnew.level, boardnew.level);
		
		if (boardold.flag & VOTE_FLAG)
			boardnew.flag |= BOARD_FLAG_VOTE;
		if (boardold.flag & NOZAP_FLAG)
			boardnew.flag |= BOARD_FLAG_NOZAP;
		if (boardold.flag & OUT_FLAG)
			boardnew.flag |= BOARD_FLAG_OUT;
		if (boardold.flag & ANONY_FLAG)
			boardnew.flag |= BOARD_FLAG_ANONY;
		if (boardold.flag & NOREPLY_FLAG)
			boardnew.flag |= BOARD_FLAG_NOREPLY;
		if (boardold.flag & BOARDJUNK_FLAG)
			boardnew.flag |= BOARD_FLAG_JUNK;
		if (boardold.level & PERM_POSTMASK)
			boardnew.flag |= BOARD_FLAG_POST;
		if (boardold.level & PERM_NOZAP)
			boardnew.flag |= BOARD_FLAG_NOZAP;
		printf("flag:%x\t%x\n\n\n",  boardnew.flag, boardnew.flag);
  		fwrite(&boardnew,sizeof(boardnew),1,fout);
  
	}
	fclose(fout);
	fclose(fin);

}

