/*   Filename: chboard.c      Location: bbs_src/local_utl    */
/*   Compile:  gcc -O2 -o chboard chboard.c record.c         */
/*   Assume bbs is in /home/bbs                              */
#include "../bbs.h"

int
report()
{
   return;
}

main()
{
        FILE *rec;
        int i=0;
        struct boardheader board;
        rec=fopen("/home/bbs/.BOARDS","rb");
        printf("Board Records Transfering...\n");
        while(1)
        {
                if(fread(&board,sizeof(board),1,rec)<=0) break;
                i++;
                printf("%d %s\t\n",i,board.filename);
                if(!strcmp(board.filename,"anonymous")||!strcmp(board.filename,"Diary"))
                  {
                  board.flag |= ANONY_FLAG;
                  printf("%s set to anonymous.\n",board.filename);
                  substitute_record("/home/bbs/.BOARDS",&board,sizeof(board),i);
                  }
                else
                  {
                  board.flag &= ~ANONY_FLAG;
                  substitute_record("/home/bbs/.BOARDS",&board,sizeof(board),i);
                  }
        }
        printf("\n%d Board Records Transferred...\n",i);
        fclose(rec);
}

