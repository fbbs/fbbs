

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BBSDISKPATH "/export/home/bbs/0Announce/groups/"
#define TARWORKPATH "/bbsbackup/Anntar/"
#define TRASHPATH "/bbsbackup/anntrash/"
#define DOTNAMES ".Names"
#define DOTINDEX "index.htm"
#define MAXQUEUEDEEP 4096
#define MAXDIRFILES 1024
#define DIRPATHLEN 1024
#define STRLEN 255

char strBoard[STRLEN];
char strPwd[STRLEN];
char strAnnpath[STRLEN];

char arrDirs[MAXQUEUEDEEP][DIRPATHLEN];
int inttop;
int intbottom;
char arrFiles[MAXDIRFILES][STRLEN];
int intfcount;

int
dashd( fname )
char    *fname;
{
    struct stat st;

    return ( stat( fname, &st ) == 0 && S_ISDIR( st.st_mode ) );
}

void deltrashfile(char *sourcepath)
{
  char Buf[STRLEN];
  char Cmd[STRLEN];
  FILE *fl;
  int bolexist;
  int ii;

  sprintf(Buf, "ls %s > %slist_txt", sourcepath, TRASHPATH); 
  system(Buf); 

  sprintf(Buf,"%slist_txt",TRASHPATH);
  if (fl=fopen(Buf,"rt")) {
      while (!feof(fl)){
          fgets(Buf,STRLEN,fl);
          if (feof(fl)) break;
          Buf[strlen(Buf)-1]='\0';
          bolexist=0;
          for (ii=0;ii<intfcount;ii++)
              if (!strcmp(Buf,arrFiles[ii])) {
                  bolexist=1;
                  break;
              }
          printf("File:%s\n",Buf);
          if (!bolexist) {
              sprintf(Cmd,"mv %s/%s %s%s"
                  ,sourcepath,Buf,TRASHPATH,strBoard);
              system(Cmd);
              printf("Cmd:%s\n",Cmd);
          }
      }
  }
  fclose(fl);
}

void convertfile(char *sourcepath,char *targetpath)
{
}

void do_dir()
{
    char strSourcedir[DIRPATHLEN];
    char strTargetdir[DIRPATHLEN];
    char strFindex[DIRPATHLEN];
    char strFnames[DIRPATHLEN];
    char strCon[STRLEN];
    char fn[STRLEN];
    char * ptr;
    FILE * findex;
    FILE * fnames;
    int boltrash;
    int ii;

    //sprintf(strSourcedir,"%s%s%s",BBSDISKPATH,strAnnpath,arrDirs[intbottom]);
    sprintf(strSourcedir,arrDirs[intbottom]);
    sprintf(strTargetdir,"%s%sd%7",TARWORKPATH,strBoard,intbottom);
    // 对文件所在源路径和目的路径赋值

    // &&& sprintf(strFindex,"%s/index.htm",strTargetdir);
    // &&& if ( findex=fopen(strFindex,"w") ) {
        // &&& 写入 index.htm 的起始的一些信息

        //sprintf(strFnames,"%s%s%s/%s",BBSDISKPATH,strAnnpath,strBoard,DOTNAMES);
        sprintf(strFnames,"%s/%s",strSourcedir,DOTNAMES);
        if (fnames=fopen(strFnames,"rt")) {
            // &&& 读头信息
            intfcount=0;
            while (!feof(fnames)) {
                fgets(strCon,STRLEN,fnames);
                if (feof(fnames)) break;
                else strCon[strlen(strCon)-1]=0;
                if (ptr=strstr(strCon,"Path=~/")){
                    //ptr[strlen(ptr)-1]=0;
                    strcpy(arrFiles[intfcount],ptr+7);
                    boltrash=0;
                    for (ii=0;ii<intfcount;ii++)
                        if (!strcmp(arrFiles[intfcount],arrFiles[ii])) {
                            boltrash=1;
                            break;
                        }
                    if (boltrash) continue;
                    sprintf(fn,"%s/%s",strSourcedir,arrFiles[intfcount]);
                    if (dashd(fn)) {
                        strcpy(arrDirs[inttop],fn);
                        inttop++;
                    }
                    intfcount++;
                }
                // &&& 将PATH字段的值写入arrFiles[intfcount];

                
                // 检查是否存在重复的链接，若有，则跳过此文件。
                
                
            }
        }
        // 读 .Names 文件
        fclose(fnames);

        deltrashfile(strSourcedir);
        printf("Do Dir:%s.\n",strSourcedir);
        // &&& 写入 index.htm 结束的一些信息
    // &&& }
}

int
main(int argc, char **argv)
{
    char strCmd[DIRPATHLEN];
    char strFname[DIRPATHLEN];
    FILE * fw;

    strcpy(strBoard,"emprise");  // &&& 需要替换
    // copy from 原来的程序
    printf("Initializing...\n"); 
    if (argc < 2) { 
        printf("Syntax: %s XDIR\n", argv[0]); 
        exit(-1); 
    }
    strcpy(strBoard, argv[1]); 
    getcwd(strPwd, STRLEN); 
    
    //if (chdir(strBoard)){ 
    //    printf("XDIR \"%s\" not found\n", strBoard); 
    //    exit(-1); 
    //}
    strcpy(strAnnpath,"literal.faq/"); // &&& 需要替换
    if (!dashd(strBoard)){
        printf("XDIR \"%s\" not found\n", strBoard); 
    	exit(-1);
    }
    // end of copy
    
    // 得到版面名称，存入strBoard

    intbottom=0;
    inttop=1;
    sprintf(arrDirs[intbottom],"%s",strBoard);
    // 初始化存放目录的队列
    
    // &&& sprintf(strCmd,"mkdir %s%s",TARWORKPATH,strBoard);
    // &&& system(strCmd);
    sprintf(strCmd,"mkdir %s%s",TRASHPATH,strBoard);
    system(strCmd);
    // 建立相应的目录

    while (intbottom<inttop) {
        do_dir();
        intbottom++;
    }
    // 处理该版精华区下所有的目录
    
    /* // &&&
    sprintf(strFname,"%s%s/index.htm",TARWORKPATH,strBoard);
    if ( fw=fopen(strFname,"w") ) {
        // &&& 根据页面，写入index.htm
    }
    // 写入index.htm

    sprintf(strCmd,"tar cf - %s%s| gzip - >%s%s%s.tar.gz"
        ,TARWORKPATH,strBoard,BBSDISKPATH,strAnnpath,strBoard);
    system(strCmd);
    */
    // 将生成的内容打包
    
    printf("END.\n");
}


