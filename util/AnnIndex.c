#include <stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include <time.h>


#define BOARD_LEN	32
#define STR_LEN		80
#define BUF_SIZE	256
#define PATH_LEN	2048
#define DOTNAME		".Names"
#define MAXDEPTH	15

static char weeknum[7][3]={"天","一","二","三","四","五","六"};

int logChange = 1,
	debug_mode = 0;

FILE *changeFile = NULL,
	*allIndex = NULL,
	*dirIndex = NULL;
int debug(char *msg, char *buf){
    printf("debug %s: %s\n", msg, buf);
}


//如果fname存在,且为目录,返回真
int dashd(char *fname)
{
	struct stat st;
	return stat(fname, &st) == 0;
}

void rtrim(char *str)
{
	unsigned char *ustr=(unsigned char *)str;
	int i=strlen(str)-1;
	if(i<0)
		return;
	while(i>=0 && ustr[i]<=32)
		i--;
	str[i+1]='\0';
}

int insertFile(char *path, char *file, char *title){
	FILE *fp;
	int got = 0, num = 0;
	char buf[BUF_SIZE];
	if ((fp = fopen(path, "a+"))!= NULL){
		while (fgets(buf, BUF_SIZE, fp)){
			rtrim(buf);
			if (!strncmp (buf, "Path=", 5)){
				num++;
				if (!strcmp(buf+7, file)){
					got = 1;
					break;
				}
			}
		}
		if (!got){
			if (strlen(title) > 38 )
			    fprintf(fp, "Name=%s\n",title);
			else
				fprintf(fp, "Name=%-38sBBS系统\n",title);
			fprintf(fp, "Path=~/%s\nNumb=%d\n#\n",file, num+1);
		}
		fclose(fp);
	}
}
										

int boardAnnIndex(char *currPath, int level, char *numstr){
	int isHide = 0, llen, num = 0;
	char dotNameFile[PATH_LEN], dotnamebuf[BUF_SIZE], title[STR_LEN], path[STR_LEN],genbuf[BUF_SIZE];
    struct stat st;
	FILE *fpNames;

	struct tm *tm;
	time_t now = time(0);
	tm = localtime(&now);
	
	sprintf(dotNameFile,"%s/.Names",currPath);
	if( (fpNames = fopen(dotNameFile, "r")) == NULL) return -1;
			
	if (level == 0){
		fprintf(allIndex,"\033[0;1;41;33m ---====== ※精华区总索引※ [更新时间："
			"%4d年%02d月%02d日%02d:%02d:%02d 星期%2s] ======--- \033[m\n",
			tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
			tm->tm_hour,tm->tm_min,tm->tm_sec,
			weeknum[tm->tm_wday]);
		fprintf(dirIndex,"\033[0;1;41;33m ---===== ※精华区目录索引※ [更新时间："
			"%4d年%02d月%02d日%02d:%02d:%02d 星期%2s] =====--- \033[m\n",
			tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
			tm->tm_hour,tm->tm_min,tm->tm_sec,
			weeknum[tm->tm_wday]);
		if (logChange){
			fprintf(changeFile,"\033[0;1;41;33m --=== ※六十日精华区整理记录※ [更新时间]"
				"%4d年%02d月%02d日%02d:%02d:%02d 星期%2s] ===-- \033[m\n",
				tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
				tm->tm_hour,tm->tm_min,tm->tm_sec,
				weeknum[tm->tm_wday]);
		}
	}
	while (fgets(dotnamebuf, BUF_SIZE, fpNames)){	
		llen = strlen(dotnamebuf);
		if (llen <= 0)
			continue;           /*      null line       */
		if ('#' == *dotnamebuf) {      /*      comment line */
			continue;
		} else if (!strncmp(dotnamebuf, "Name=", 5)) {
			strcpy(title, dotnamebuf + 5);
			rtrim(title);
			if(!strncmp(title+38,"(BM: SYSOPS)",11)
				||!strncmp(title+38,"(BM: OBOARDS)",12)  
				||!strncmp(title+38,"(BM: SECRET)",11)  
				||!strncmp(title+38,"(BM: BMS)",8)
				||!strncmp(title,"<HIDE>",6)) {
				isHide = 1;
			}
			else{
				isHide = 0;
			}
			if(strlen(title)>38)title[38] = '\0'; 
	        continue;           /*      null line       */
       } else if (!strncmp(dotnamebuf, "Path=~/", 7)) {
            strcpy(path, dotnamebuf + 7);
			rtrim(path);
            continue;
        } else if (!strncmp(dotnamebuf, "Numb=", 5)) {
			if(isHide) continue;
			num++;

        } else {
  //          if (debug_mode)
//                printf("Error in line  of file %s :\n%s\n", filename, dotnamebuf);
            continue;
		}

		if (strlen(path) < 1)
			continue;


		//生成目录/文件名
		sprintf(genbuf,"%s/%s", currPath, path);
		if (stat(genbuf, &st) == -1) {
			if (debug_mode)
				printf("Error stating file %s !\n", genbuf);
			continue;
		}
		char newpath[PATH_LEN], str[STR_LEN], tmstr[STR_LEN];
		char output[STR_LEN];
		tm = localtime(&st.st_mtime);
		sprintf(tmstr,"%02d.%02d.%02d",  tm->tm_year%100, tm->tm_mon + 1, tm->tm_mday);
		strcpy(newpath, genbuf);
		sprintf(str, "%s%2d.", numstr, num);
		if (S_ISDIR(st.st_mode)){
			sprintf(output, "%s[\033[1;32m目录\033[m]%s", str,title);	
		} else if (S_ISREG(st.st_mode)) {
			sprintf(output, "%s[\033[1;36m文件\033[m]%s", str,title); 
		} else{
			if (debug_mode)
				printf("Error: Unknown item encountered at line %d of %s\n", title);
			continue;
		}
		fprintf(allIndex, "%s\n", output);
		if (logChange && now - st.st_mtime < 3600 * 60 * 24) {
			fprintf(changeFile,"%-78s %-10s\n", output, tmstr);
		}
		if (S_ISDIR(st.st_mode)){
			fprintf(dirIndex, "%s\n", output);
			if (level < MAXDEPTH)
				boardAnnIndex(newpath, level+1, str);
		}

	}
	fclose(fpNames);

}

int IndexAnnounce(char *bname){
	FILE *fp = NULL;
	int i;
	char boardbuf[BUF_SIZE], pathname[PATH_LEN], 
		 genbuf[BUF_SIZE], boardName[BOARD_LEN] ;
	if ((fp = fopen("/home/bbs/0Announce/.Search", "r")) == NULL){
        printf("Cannot find the /home/bbs/0Announce/.Search file \n program will exit. ");
        return 1;
    }
	while(fgets(boardbuf, BUF_SIZE, fp)){
		sscanf(boardbuf, "%s %s\n", boardName, genbuf);
		for (i = 0; i < BOARD_LEN; i++)
				if (boardName[i] == ':'){
						boardName[i] = '\0';
						break;
				}
		if (bname == NULL || !strcmp(bname, boardName)){
//			if (seek_in_file("/home/bbs/etc/AnnIndexExcp",boardName)!=0 && boardmode != 1 )
//				continue;

			sprintf(pathname,"/home/bbs/0Announce/%s",genbuf);
			if (!dashd(pathname)){
				printf("CAN'T FINd %S", pathname);
				continue;
			}

			sprintf(genbuf, "%s/.annIndex", pathname);
			if (!dashd(genbuf)){
				mkdir(genbuf, 0755);
			}
			
			sprintf(genbuf, "%s/.Names", pathname);

			insertFile(genbuf,".annIndex", "【本板精华区索引】                    (BM: BMS)");
			
			sprintf(genbuf, "%s/.annIndex/.index_all", pathname);
			if ((allIndex = fopen(genbuf, "w+")) > 0){
				sprintf(genbuf, "%s/.annIndex/.Names", pathname);
				insertFile(genbuf,".index_all", "精华区总索引");
			}
			
			sprintf(genbuf, "%s/.annIndex/.index_dir", pathname);
			if ((dirIndex = fopen(genbuf, "w+")) > 0){
					sprintf(genbuf, "%s/.annIndex/.Names", pathname);
					insertFile(genbuf,".index_dir", "精华区目录索引");
			}
			
			sprintf(genbuf, "%s/.annIndex/.change_log", pathname);
			if (logChange)
				if((changeFile = fopen(genbuf, "w+")) > 0){
					sprintf(genbuf, "%s/.annIndex/.Names", pathname);
					insertFile(genbuf,".change_log", "六十日精华区整理记录");
				}
			
									
			boardAnnIndex(pathname, 0, "");
			if (logChange)
					fclose(changeFile);
			fclose(allIndex);
			fclose(dirIndex);
		}
    }
	fclose(fp);
}


int main(int argc, char *argv[])
{
	int i;
	for (i = 1; i < argc; i++){
		if(!strcmp(argv[i],"-l")){//log
			logChange = 0;
		} else if(!strcmp(argv[i],"-d"))//debug
			debug_mode = 1;
		else if(!strcmp(argv[i],"-h")){
		} else if(!strcmp(argv[i],"-b")){
			IndexAnnounce(argv[++i]);
			return 0;
		} else{
			printf("USAGE: AnnIndex [-d] [-l] [-h] [-b] [BoardName]");
			return 0;
		}
		
	}
	if (argc == 1)
		IndexAnnounce(NULL);


	return 0;
}

