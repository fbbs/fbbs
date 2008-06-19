#include <stdio.h>
#include "bbs.h"
#define BM_LEN 60
//#include "../../include/config.h"
struct boardheader {		     /* This structure is used to hold data in */
	char filename[STRLEN];   /* the BOARDS files */
	char owner[STRLEN - BM_LEN];
	char BM[ BM_LEN - 1];
	char flag;
	char title[STRLEN ];
	unsigned level;
	unsigned char accessed[ 12 ];
};

#define PERM_BASIC      000001
#define PERM_CHAT       000002
#define PERM_MAIL       000004
#define PERM_POST       000010
#define PERM_LOGINOK    000020
#define PERM_DENYPOST   000040
#define PERM_CLOAK      000100
#define PERM_SEECLOAK   000200
#define PERM_XEMPT      000400
#define PERM_WELCOME    001000
#define PERM_BOARDS     002000
#define PERM_ACCOUNTS   004000
#define PERM_CHATCLOAK  010000
#define PERM_OVOTE      020000
#define PERM_SYSOP      040000
#define PERM_POSTMASK  0100000     
#define PERM_ANNOUNCE  0200000
#define PERM_OBOARDS   0400000
#define PERM_ACBOARD   01000000
#define PERM_NOZAP     02000000
#define PERM_FORCEPAGE 04000000
#define PERM_EXT_IDLE  010000000
#define PERM_MESSAGE   020000000
#define PERM_ACHATROOM 040000000
#define PERM_LARGEMAIL 0100000000
#define PERM_SPECIAL4  0200000000
#define PERM_SPECIAL5  0400000000
#define PERM_SPECIAL6  01000000000
#define PERM_SPECIAL7  02000000000
#define PERM_SPECIAL8  04000000000

#define NEW_PERM_LOGIN      0x1
#define NEW_PERM_TALK       0x2
#define NEW_PERM_MAIL       0x4
#define NEW_PERM_POST       0x8
#define NEW_PERM_REGISTER   0x10
#define NEW_PERM_BINDMAIL   0x20
#define NEW_PERM_BOARDS     0x40
#define NEW_PERM_OBOARDS    0x80
#define NEW_PERM_ACLUB      0x100
#define NEW_PERM_ANNOUNCE   0x200
#define NEW_PERM_USER       0x400
#define NEW_PERM_ACBOARD    0x800
#define NEW_PERM_ACHATROOM  0x1000
#define NEW_PERM_SYSOP      0x2000
#define NEW_PERM_CLOAK      0x4000
#define NEW_PERM_SEECLOAK   0x8000
#define NEW_PERM_XEMPT      0x10000
#define NEW_PERM_LONGLIFE   0x20000
#define NEW_PERM_LARGEMAIL  0x40000
#define NEW_PERM_ARBI       0x80000
#define NEW_PERM_SERV       0x100000
#define NEW_PERM_TECH       0x200000
#define NEW_PERM_SPECIAL0   0x400000
#define NEW_PERM_SPECIAL1   0x800000
#define NEW_PERM_SPECIAL2   0x1000000
#define NEW_PERM_SPECIAL3   0x2000000
#define NEW_PERM_SPECIAL4   0x4000000
#define NEW_PERM_SPECIAL5   0x8000000
#define NEW_PERM_SPECIAL6   0x10000000
#define NEW_PERM_SPECIAL7   0x20000000
#define NEW_PERM_SPECIAL8   0x40000000
#define NEW_PERM_SPECIAL9   0x80000000

#define NEW_HAS_PERM(u, x)     ((x)?u.level&(x):1)

 
void debug(int i, int j){
  printf("step %x\t%x\n",i, j);
}



void myuserlever()
{
    char *src, *dst;
	struct boardheader board;
	FILE *fr,*fw;
	int i,count;
	unsigned int level;
	dst = "/home/bbs/.BOARDS";
	src= "/home/bbs/.BOARDS.NEW";
	if ((fr = fopen(dst, "r"))&& (fw=fopen(src, "w")))
	{
		while(fread(&board, sizeof(struct boardheader), 1, fr)){
			level = 0;
			if (board.level != 0){
				if (NEW_HAS_PERM(board, PERM_BASIC))
					level |=  NEW_PERM_LOGIN;
				if (NEW_HAS_PERM(board, PERM_CHAT) && NEW_HAS_PERM(board, PERM_MESSAGE))
					level |=  NEW_PERM_TALK;
				if (NEW_HAS_PERM(board, PERM_MAIL))
					level |=  NEW_PERM_MAIL;
				if (NEW_HAS_PERM(board, PERM_POST))
					level |=  NEW_PERM_POST;
				if (NEW_HAS_PERM(board, PERM_LOGINOK))
					level |=  NEW_PERM_REGISTER;
				if (NEW_HAS_PERM(board, PERM_CLOAK))
					level |=  NEW_PERM_CLOAK;
				if (NEW_HAS_PERM(board, PERM_SEECLOAK))
					level |=  NEW_PERM_SEECLOAK;
				if (NEW_HAS_PERM(board, PERM_XEMPT))
					level |=  NEW_PERM_XEMPT;
				if (NEW_HAS_PERM(board, PERM_BOARDS))
					level |=  NEW_PERM_BOARDS;
				if (NEW_HAS_PERM(board, PERM_ACCOUNTS))
					level |=  NEW_PERM_USER;
				if (NEW_HAS_PERM(board, PERM_SYSOP))
					level |=  NEW_PERM_SYSOP;
				if (NEW_HAS_PERM(board, PERM_ANNOUNCE))
					level |=  NEW_PERM_ANNOUNCE;
				if (NEW_HAS_PERM(board, PERM_OBOARDS))
					level |=  NEW_PERM_OBOARDS;
				if (NEW_HAS_PERM(board, PERM_ACBOARD))
					level |=  NEW_PERM_ACBOARD;
				if (NEW_HAS_PERM(board, PERM_EXT_IDLE))
					level |=  NEW_PERM_SPECIAL0;
				if (NEW_HAS_PERM(board, PERM_ACHATROOM))
					level |=  NEW_PERM_ARBI;
				if (NEW_HAS_PERM(board, PERM_LARGEMAIL))
					level |=  NEW_PERM_LARGEMAIL;
				if (NEW_HAS_PERM(board, PERM_SPECIAL4))
					level |=  NEW_PERM_BINDMAIL;
				if (NEW_HAS_PERM(board, PERM_SPECIAL5))
					level |=  NEW_PERM_SERV;
				if (NEW_HAS_PERM(board, PERM_SPECIAL7))
					level |=  NEW_PERM_TECH;
				if (NEW_HAS_PERM(board, PERM_SPECIAL8))
					level |=  NEW_PERM_LONGLIFE;

				debug(board.level,level);
				board.level = level;
			}
			fwrite(&board, sizeof(struct boardheader), 1, fw);
		}
		fclose(fr);
		fclose(fw);
	}
}


int main(int argc,char ** argv) {
	int i;

		
    myuserlever();
	

    return 0;
}


