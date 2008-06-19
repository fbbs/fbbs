/* 
   Added by roly 2002.05.03 
	自动检查.BOARDS文件,寻找担任斑竹的ID,给这些ID添加斑竹权限


编译本程序前，请把 #define BBSHOME "/home/bbs" 修改成你正确的路径。

编译方法：
 gcc -O2 -L/usr/ucblib -lucb -lsocket -lnsl -I../../include -o BMPerm BMPerm.c ../../src/record.c -L../../lib -lBBS
使用方法： ./reBMlist 

*/
#define BM_LEN	60
#define IDLEN 12
#define STRLEN 80
#define BBSHOME "/export/home/bbs"
#include "bbs.h" 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


struct boardheader fh;
char uid[IDLEN+2];
int usernum;

int
getusernum(uentp)
struct userec *uentp ;
{

    if (strcmp(uentp->userid,uid)) usernum++;
    else return QUIT;
    return 0 ;
}


void report(char *str)
{
}


int
main(void)
{
   FILE *fp;
   int bid = 1,bmnum,i,j;
   char usernames[3][80];
   char BOARDS2[80];
	struct userec currentuser;
   int icount = 0 ;

   sprintf(BOARDS2,"%s/.BOARDS",BBSHOME);

   while(1) {
	//	printf("bid=%d",bid);
       	if (get_record(BOARDS2, &fh, sizeof(fh), bid) == -1)break; 
        bmnum = 0;
        for (i =0,j =0 ;fh.BM[i] != '\0';i++) {
                if( fh.BM[i] == ' ' ) {
                        usernames[bmnum][j] = '\0';
                        bmnum ++;
                        j = 0;
                }
                else usernames[bmnum][j++] = fh.BM[i];
        }
	 usernames[bmnum++][j] = '\0'; 
   
    if(fh.filename[0] == '\0') 
		bid++;//printf("No.%2d invalidation, ship...\n",bid ++);
	else if(fh.BM[0] == '\0')
		bid++;//printf("No.%2d %16s   : [no boardmanager] ship...\n",bid ++,fh.filename );
	else {
		icount++;
		printf("%3d.\t%-20s\t",icount,fh.filename);
		bid ++;
		for( i = 0 ; i < bmnum ; i ++ ) {
			strcpy(uid,usernames[i]);
			usernum=0;
			apply_record(PASSFILE, getusernum, sizeof(struct userec));
		    	usernum++;
			if(get_record(PASSFILE,&currentuser,sizeof(currentuser),usernum)==-1){
    				printf("Error reading PASSFILE!\n");
    			}
    			//currentuser.userlevel |= PERM_BOARDS;
    			//substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
    			printf("%-12s\t",uid);
			printf("%-20s\t\n",currentuser.realname);
			break;
    		}
    	}
   }
   printf("Total boards %d .\n",bid-1);
}

