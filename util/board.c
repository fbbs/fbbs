#define VOTE_FLAG    	0x1			//投票模式
#define NOZAP_FLAG   	0x2			//
#define OUT_FLAG     	0x4			//
#define ANONY_FLAG   	0x8			//匿名模式
#define NOREPLY_FLAG 	0x10 		/* No reply board */
#define BOARDJUNK_FLAG	0x20		//

#define BOARD_VOTE_FLAG         0x1             //投票模式
#define BOARD_NOZAP_FLAG        0x2             //no zap
#define BOARD_OUT_FLAG          0x4             //转信
#define BOARD_ANONY_FLAG        0x8             //匿名模式
#define BOARD_NOREPLY_FLAG      0x10            //No reply board
#define BOARD_JUNK_FLAG         0x20            //不记文章数
#define BOARD_CLUB_FLAG         0x40            //俱乐部版面
#define BOARD_READ_FLAG         0x80            //隐藏版面
#define BOARD_POST_FLAG         0x100           //postmask

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
			boardnew.flag |= BOARD_VOTE_FLAG;
		if (boardold.flag & NOZAP_FLAG)
			boardnew.flag |= BOARD_NOZAP_FLAG;
		if (boardold.flag & OUT_FLAG)
			boardnew.flag |= BOARD_OUT_FLAG;
		if (boardold.flag & ANONY_FLAG)
			boardnew.flag |= BOARD_ANONY_FLAG;
		if (boardold.flag & NOREPLY_FLAG)
			boardnew.flag |= BOARD_NOREPLY_FLAG;
		if (boardold.flag & BOARDJUNK_FLAG)
			boardnew.flag |= BOARD_JUNK_FLAG;
		if (boardold.level & PERM_POSTMASK)
			boardnew.flag |= BOARD_POST_FLAG;
		if (boardold.level & PERM_NOZAP)
			boardnew.flag |= BOARD_NOZAP_FLAG;
		printf("flag:%x\t%x\n\n\n",  boardnew.flag, boardnew.flag);
  		fwrite(&boardnew,sizeof(boardnew),1,fout);
  
	}
	fclose(fout);
	fclose(fin);

}

