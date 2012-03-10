#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/cfg.h"
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"

int type=-1, flag=0, timed=0, sorttype=0;
char groupid=0;

char suffix[4][10]={"", "week", "month", "year"};
struct libtruct {
	char boardname[STRLEN];
	char id[STRLEN];
	int data[BMLOGLEN];
} *lib;
int total=0;

int order[10000];

int show(int k,int i)
{
    printf("序号: %-4d ", k+1);
    if(lib[i].data[0]>=3600)
    	printf("版名: %-15s 版主: %-13s 停留: %d 小时 %d 分 %d秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]/3600, lib[i].data[0]/60%60, lib[i].data[0]%60);
    else if(lib[i].data[0]>=60)
    	printf("版名: %-15s 版主: %-13s 停留: %d 分 %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]/60, lib[i].data[0]%60);
    else
		printf("版名: %-15s 版主: %-13s 停留: %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]);
    printf("    进版次数: %-4d    版内发文: %-4d    删除文章: %-4d    恢复删除: %-4d\n", 
		lib[i].data[BMLOG_INBOARD], lib[i].data[BMLOG_POST], lib[i].data[BMLOG_DELETE], lib[i].data[BMLOG_UNDELETE]);
    printf("    收入文摘: %-4d    去掉文摘: %-4d    标记 m文: %-4d    去掉 m文: %-4d\n", 
	lib[i].data[BMLOG_DIGIST], lib[i].data[BMLOG_UNDIGIST],	lib[i].data[BMLOG_MARK], lib[i].data[BMLOG_UNMARK]);
    printf("    标记水文: %-4d    去掉水文: %-4d    标记 x文: %-4d    去掉 x文: %-4d\n", 
	lib[i].data[BMLOG_WATER], lib[i].data[BMLOG_UNWATER], lib[i].data[BMLOG_CANNOTRE], lib[i].data[BMLOG_UNCANNOTRE]);
    printf("    封禁人数: %-4d    解封人数: %-4d    加入Club: %-4d    取消Club: %-4d\n", 
		lib[i].data[BMLOG_DENYPOST], lib[i].data[BMLOG_UNDENY], lib[i].data[BMLOG_ADDCLUB], lib[i].data[BMLOG_DELCLUB]);
    printf("    收入精华: %-4d    整理精华: %-4d    合集文章: %-4d\n", 
		lib[i].data[BMLOG_ANNOUNCE], lib[i].data[BMLOG_DOANN], lib[i].data[BMLOG_COMBINE]);
    printf("    区段收精: %-4d    区段删除: %-4d    区段其他: %-4d\n", lib[i].data[BMLOG_RANGEANN],  lib[i].data[BMLOG_RANGEDEL], lib[i].data[BMLOG_RANGEOTHER]);
	printf("\n");
    return 0;
}

void sort()
{
	int i,j,k;
	for(i=0;i<total;i++)
		order[i]=i;
	switch(sorttype){
		case 0:
			for(i=0;i<total;i++)
				for(j=i+1;j<total;j++)
				if(lib[order[i]].data[BMLOG_INBOARD]<lib[order[j]].data[BMLOG_INBOARD]){
					k=order[i];
					order[i]=order[j];
					order[j]=k;
				}
			break;
		case 1:
			for(i=0;i<total;i++)
				for(j=i+1;j<total;j++)
				if(strcasecmp(lib[order[i]].id, lib[order[j]].id)>0){
					k=order[i];
					order[i]=order[j];
					order[j]=k;
				}
			break;
	}
}

void showall()
{
	int i;
	time_t now;
	struct tm *today;
	
	time(&now);
	today = localtime(&now);
	
	switch (timed){
		case 0:
			printf("%04d-%02d-%02d 日版主工作情况一览\n", 
				1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
			break;
		case 1:
			printf("%04d-%02d-%02d 本周版主工作情况一览.\n",
				1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
			break;
		case 2:
			printf("%04d-%02d-%02d 到 %04d-%02d-%02d 本月版主工作情况一览\n",
				1900+today->tm_year, today->tm_mon + 1,1,
				1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
				
			break;
		case 3:
			printf("%04d-%02d-%02d 到 %04d-%02d-%02d 年度版主工作情况一览\n",
				1900+today->tm_year, 1, 1,
				1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
	}
	printf("--------------------------------------------------------------------------------\n\n");
	for(i=0;i<total;i++)
		show(i, order[i]);
}

int check_BM(const char* board, char* bmname,void* arg)
{
    int i, fd, data[BMLOGLEN];
    struct flock ldata;
    struct stat buf;
    char direct[STRLEN];
    char bm[20];

    strcpy(bm,bmname); 
    sprintf(direct, "boards/%s/.bm.%s%s", board, bm, suffix[timed]);
    if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1) return 0;
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        close(fd);
        return 0;
    }
    fstat(fd, &buf);
    if(buf.st_size<BMLOGLEN*sizeof(int)){
       	memset(data, 0, sizeof(int)*BMLOGLEN);
    }
    else
       	read(fd, data, sizeof(int)*BMLOGLEN);
    if (flag==1){
    	for(i=0;i<BMLOGLEN;i++)
    		data[i]+=lib[total].data[i];
    	total++;
        lseek(fd, 0, SEEK_SET);
        write(fd, data, sizeof(int)*BMLOGLEN);
    }
    else if (flag==3) {
    	for(i=0;i<BMLOGLEN;i++)
    		data[i]=lib[total].data[i];
    	total++;
        lseek(fd, 0, SEEK_SET);
        write(fd, data, sizeof(int)*BMLOGLEN);
    }
   	ldata.l_type = F_UNLCK;
   	fcntl(fd, F_SETLKW, &ldata);
   	close(fd);
   	if (flag==-1) {
   		unlink(direct);
   	}
   	else if (flag==0)
   		total++;
   	else if (flag==1){
   	}
   	else if (flag==2){
   		strcpy(lib[total].boardname,board );
   		strcpy(lib[total].id, bm);
   		memcpy(lib[total].data, data, sizeof(int)*BMLOGLEN);
		total++;
   	}
    return 0;
}

static void query_BM(const board_t *boards, int count)
{
	char bms[BOARD_BM_LEN + 1], *bm;

	for (int i = 0; i < count; ++i) {
		const board_t *bp = boards + i;
		strlcpy(bms, bp->bms, sizeof(bms));
		bm = strtok(bms, " ");
		while (bm) {
			if (strncmp(bm, "SYSOP", 5) && strncmp(bm, "SYSOPs", 6))
				check_BM(bp->name, bm, NULL);
			bm = strtok(NULL,  " ");
		}
	}
}

static void update_BM(const board_t *boards, int count)
{
	int j, k, data[BMLOGLEN], datatmp[BMLOGLEN];
	int fdtmp;
	char *bm;
	char buf[80], bms[BOARD_BM_LEN + 1];
	time_t t;
	struct tm *res;

	time(&t);
	res = localtime(&t);

	for (int i = 0; i < count; ++i) {
		const board_t *bp = boards + count;
		strlcpy(bms, bp->bms, sizeof(bms));
		bm = strtok(bms, " ");
		while (bm) {
			if (strncmp(bm,"SYSOP", 5) && strncmp(bm, "SYSOPs", 6) ){
				sprintf(buf, "boards/%s/.bm.%s", bp->name, bm);
				if ((fdtmp = open(buf, O_RDONLY, 0644))) {
					bzero(data, sizeof(data));
					read(fdtmp, data, sizeof(data));
					close(fdtmp);
					unlink(buf);
					for (j = 1; j < 4 ; j++ )
					{
						sprintf(buf, "boards/%s/.bm.%s%s", bp->name, bm, suffix[j]);
						if ((j == 1 && res->tm_wday == 0 )
							|| (j == 2 && res->tm_mday == 1 )
							||(j == 3 && res->tm_yday == 0 ))
						{
							unlink(buf);
						} else if ((fdtmp = open(buf, O_RDWR | O_CREAT, 0644))) {
							bzero(datatmp, sizeof(datatmp));
							read(fdtmp, datatmp, sizeof(datatmp));
							for(k=0;k<BMLOGLEN;k++){
								datatmp[k] += data[k];
							}
							lseek(fdtmp, 0, SEEK_SET);
							
							write(fdtmp, datatmp, sizeof(datatmp));
							close(fdtmp);
						}
					}

				}
			}
			bm = strtok( NULL,  " ");
		}

	}
}

int main(int argc, char ** argv)
{
	if (argc<=1) {
		printf("usage: statBM day|week|month|year|update|clear\n");
		return EXIT_FAILURE;
	}
	if (!strcasecmp(argv[1],"clear")) type=0;
	if (!strcasecmp(argv[1],"day")) type=1;
	if (!strcasecmp(argv[1],"week")) type=2;
	if (!strcasecmp(argv[1],"month")) type=3;
	if (!strcasecmp(argv[1],"year")) type=4;
	if (!strcasecmp(argv[1],"update")) type=5;
	if (argc>=3&&!strcasecmp(argv[2],"id")) sorttype=1;
	if (type==-1) {
		printf("usage: statBM day|week|month|year|update|clear\n");
		return EXIT_FAILURE;
	}

	env.p = pool_create(DEFAULT_POOL_SIZE);
	env.c = config_load(env.p, DEFAULT_CFG_FILE);
	initialize_convert_env();
	initialize_db();

	db_res_t *res = db_exec_query(env.d, true, BOARD_SELECT_QUERY_BASE);
	int count = db_res_rows(res);
	if (count < 1)
		return EXIT_FAILURE;
	board_t *boards = malloc(sizeof(*boards) * count);
	for (int i = 0; i < count; ++i) {
		res_to_board(res, i, boards + i);
	}

    chdir(BBSHOME);
    switch(type) {
    	case 0:
			timed=0;
			flag=-1;
			query_BM(boards, count);
    		timed=1;
    		flag=-1;
    		query_BM(boards, count);
			timed=2;
    		flag=-1;
			query_BM(boards, count);
    		timed=3;
    		flag=-1;
    		query_BM(boards, count);
		break;
    	case 1:
    	case 2:
    	case 3:
    	case 4:
    		timed=type-1;
    		flag=0;
			query_BM(boards, count);
			lib = malloc(sizeof(struct libtruct)*total);
			total=0;
    		flag=2;
    		query_BM(boards, count);
			sort();
    		showall();
    		break;
    	case 5:
			update_BM(boards, count);
		    break;
    }
}
