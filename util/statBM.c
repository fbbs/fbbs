#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/cfg.h"
#include "fbbs/helper.h"
#include "fbbs/log.h"
#include "fbbs/string.h"

int type=-1, flag=0, timed=0, sorttype=0;
char groupid=0;

char suffix[4][10]={"", "week", "month", "year"};
struct libtruct {
	char boardname[STRLEN];
	char id[STRLEN];
	int data[LOG_BM_LEN];
} *lib;
int total=0;

int order[10000];

int show(int k,int i)
{
	//% printf("序号: %-4d ", k+1);
	printf("\xd0\xf2\xba\xc5: %-4d ", k+1);
	if(lib[i].data[0]>=3600)
		//% printf("版名: %-15s 版主: %-13s 停留: %d 小时 %d 分 %d秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]/3600, lib[i].data[0]/60%60, lib[i].data[0]%60);
		printf("\xb0\xe6\xc3\xfb: %-15s \xb0\xe6\xd6\xf7: %-13s \xcd\xa3\xc1\xf4: %d \xd0\xa1\xca\xb1 %d \xb7\xd6 %d\xc3\xeb\n", lib[i].boardname, lib[i].id, lib[i].data[0]/3600, lib[i].data[0]/60%60, lib[i].data[0]%60);
	else if(lib[i].data[0]>=60)
		//% printf("版名: %-15s 版主: %-13s 停留: %d 分 %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]/60, lib[i].data[0]%60);
		printf("\xb0\xe6\xc3\xfb: %-15s \xb0\xe6\xd6\xf7: %-13s \xcd\xa3\xc1\xf4: %d \xb7\xd6 %d \xc3\xeb\n", lib[i].boardname, lib[i].id, lib[i].data[0]/60, lib[i].data[0]%60);
	else
		//% printf("版名: %-15s 版主: %-13s 停留: %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]);
		printf("\xb0\xe6\xc3\xfb: %-15s \xb0\xe6\xd6\xf7: %-13s \xcd\xa3\xc1\xf4: %d \xc3\xeb\n", lib[i].boardname, lib[i].id, lib[i].data[0]);
	//% printf("    进版次数: %-4d    版内发文: %-4d    删除文章: %-4d    恢复删除: %-4d\n", 
	printf("    \xbd\xf8\xb0\xe6\xb4\xce\xca\xfd: %-4d    \xb0\xe6\xc4\xda\xb7\xa2\xce\xc4: %-4d    \xc9\xbe\xb3\xfd\xce\xc4\xd5\xc2: %-4d    \xbb\xd6\xb8\xb4\xc9\xbe\xb3\xfd: %-4d\n",
			lib[i].data[LOG_BM_INBOARD], lib[i].data[LOG_BM_POST], lib[i].data[LOG_BM_DELETE], lib[i].data[LOG_BM_UNDELETE]);
	//% printf("    收入文摘: %-4d    去掉文摘: %-4d    标记 m文: %-4d    去掉 m文: %-4d\n",
	printf("    \xca\xd5\xc8\xeb\xce\xc4\xd5\xaa: %-4d    \xc8\xa5\xb5\xf4\xce\xc4\xd5\xaa: %-4d    \xb1\xea\xbc\xc7 m\xce\xc4: %-4d    \xc8\xa5\xb5\xf4 m\xce\xc4: %-4d\n",
			lib[i].data[LOG_BM_DIGIST], lib[i].data[LOG_BM_UNDIGIST],	lib[i].data[LOG_BM_MARK], lib[i].data[LOG_BM_UNMARK]);
	//% printf("    标记水文: %-4d    去掉水文: %-4d    标记 x文: %-4d    去掉 x文: %-4d\n",
	printf("    \xb1\xea\xbc\xc7\xcb\xae\xce\xc4: %-4d    \xc8\xa5\xb5\xf4\xcb\xae\xce\xc4: %-4d    \xb1\xea\xbc\xc7 x\xce\xc4: %-4d    \xc8\xa5\xb5\xf4 x\xce\xc4: %-4d\n",
			lib[i].data[LOG_BM_WATER], lib[i].data[LOG_BM_UNWATER], lib[i].data[LOG_BM_CANNOTRE], lib[i].data[LOG_BM_UNCANNOTRE]);
	//% printf("    封禁人数: %-4d    解封人数: %-4d    加入Club: %-4d    取消Club: %-4d\n", 
	printf("    \xb7\xe2\xbd\xfb\xc8\xcb\xca\xfd: %-4d    \xbd\xe2\xb7\xe2\xc8\xcb\xca\xfd: %-4d    \xbc\xd3\xc8\xeb""Club: %-4d    \xc8\xa1\xcf\xfb""Club: %-4d\n",
			lib[i].data[LOG_BM_DENYPOST], lib[i].data[LOG_BM_UNDENY], lib[i].data[LOG_BM_ADDCLUB], lib[i].data[LOG_BM_DELCLUB]);
	//% printf("    收入精华: %-4d    整理精华: %-4d    合集文章: %-4d\n", 
	printf("    \xca\xd5\xc8\xeb\xbe\xab\xbb\xaa: %-4d    \xd5\xfb\xc0\xed\xbe\xab\xbb\xaa: %-4d    \xba\xcf\xbc\xaf\xce\xc4\xd5\xc2: %-4d\n",
			lib[i].data[LOG_BM_ANNOUNCE], lib[i].data[LOG_BM_DOANN], lib[i].data[LOG_BM_COMBINE]);
	//% printf("    区段收精: %-4d    区段删除: %-4d    区段其他: %-4d\n", lib[i].data[LOG_BM_RANGEANN],  lib[i].data[LOG_BM_RANGEDEL], lib[i].data[LOG_BM_RANGEOTHER]);
	printf("    \xc7\xf8\xb6\xce\xca\xd5\xbe\xab: %-4d    \xc7\xf8\xb6\xce\xc9\xbe\xb3\xfd: %-4d    \xc7\xf8\xb6\xce\xc6\xe4\xcb\xfb: %-4d\n", lib[i].data[LOG_BM_RANGEANN],  lib[i].data[LOG_BM_RANGEDEL], lib[i].data[LOG_BM_RANGEOTHER]);
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
					if(lib[order[i]].data[LOG_BM_INBOARD]<lib[order[j]].data[LOG_BM_INBOARD]){
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
			//% printf("%04d-%02d-%02d 日版主工作情况一览\n", 
			printf("%04d-%02d-%02d \xc8\xd5\xb0\xe6\xd6\xf7\xb9\xa4\xd7\xf7\xc7\xe9\xbf\xf6\xd2\xbb\xc0\xc0\n", 
					1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
			break;
		case 1:
			//% printf("%04d-%02d-%02d 本周版主工作情况一览.\n",
			printf("%04d-%02d-%02d \xb1\xbe\xd6\xdc\xb0\xe6\xd6\xf7\xb9\xa4\xd7\xf7\xc7\xe9\xbf\xf6\xd2\xbb\xc0\xc0.\n",
					1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
			break;
		case 2:
			//% printf("%04d-%02d-%02d 到 %04d-%02d-%02d 本月版主工作情况一览\n",
			printf("%04d-%02d-%02d \xb5\xbd %04d-%02d-%02d \xb1\xbe\xd4\xc2\xb0\xe6\xd6\xf7\xb9\xa4\xd7\xf7\xc7\xe9\xbf\xf6\xd2\xbb\xc0\xc0\n",
					1900+today->tm_year, today->tm_mon + 1,1,
					1900+today->tm_year, today->tm_mon + 1, today->tm_mday);

			break;
		case 3:
			//% printf("%04d-%02d-%02d 到 %04d-%02d-%02d 年度版主工作情况一览\n",
			printf("%04d-%02d-%02d \xb5\xbd %04d-%02d-%02d \xc4\xea\xb6\xc8\xb0\xe6\xd6\xf7\xb9\xa4\xd7\xf7\xc7\xe9\xbf\xf6\xd2\xbb\xc0\xc0\n",
					1900+today->tm_year, 1, 1,
					1900+today->tm_year, today->tm_mon + 1, today->tm_mday);
	}
	printf("--------------------------------------------------------------------------------\n\n");
	for(i=0;i<total;i++)
		show(i, order[i]);
}

int check_BM(const char* board, char* bmname,void* arg)
{
	int i, fd, data[LOG_BM_LEN];
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
	if(buf.st_size<LOG_BM_LEN*sizeof(int)){
		memset(data, 0, sizeof(int)*LOG_BM_LEN);
	}
	else
		read(fd, data, sizeof(int)*LOG_BM_LEN);
	if (flag==1){
		for(i=0;i<LOG_BM_LEN;i++)
			data[i]+=lib[total].data[i];
		total++;
		lseek(fd, 0, SEEK_SET);
		write(fd, data, sizeof(int)*LOG_BM_LEN);
	}
	else if (flag==3) {
		for(i=0;i<LOG_BM_LEN;i++)
			data[i]=lib[total].data[i];
		total++;
		lseek(fd, 0, SEEK_SET);
		write(fd, data, sizeof(int)*LOG_BM_LEN);
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
		memcpy(lib[total].data, data, sizeof(int)*LOG_BM_LEN);
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
	int j, k, data[LOG_BM_LEN], datatmp[LOG_BM_LEN];
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
							for(k=0;k<LOG_BM_LEN;k++){
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

	initialize_environment(INIT_CONV | INIT_DB);

	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
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
