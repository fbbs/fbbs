#include <sys/shm.h>
#include "bbs.h"
#include "fbbs/terminal.h"

#define FILE_BUFSIZE        200    /* max. length of a file in SHM*/
#define FILE_MAXLINE         25    /* max. line of a file in SHM */
#define MAX_WELCOME          15    /* 欢迎画面数 */
#define MAX_GOODBYE          15    /* 离站画面数 */
#define MAX_ISSUE            15    /* 最大进站画面数 */

struct FILESHM { //内存映射文件结构
	char line[FILE_MAXLINE][FILE_BUFSIZE]; //[25][200]
	int fileline;
	int max;
	time_t update; //更新时间
};

struct STATSHM {
	char line[FILE_MAXLINE][FILE_BUFSIZE];
	time_t update;
};

struct FILESHM * goodbyeshm;
struct FILESHM * issueshm;
struct STATSHM * statshm;

// 将文件fname映射到内存,键值为shmkey,用mode来区别不同的文件
int fill_shmfile(int mode, char* fname, char * shmkey) {
	FILE * fffd;
	char * ptr;
	char buf[FILE_BUFSIZE];
	struct stat st;
	time_t ftime, now;
	int lines = 0, nowfn = 0, maxnum = MAX_GOODBYE;
	struct FILESHM * tmp;
	now = time(0);
	if (stat(fname, &st) < 0) {
		return 0;
	}
	ftime = st.st_mtime;
	tmp = attach_shm(shmkey, 5000 + mode * 10,
		sizeof(struct FILESHM) * maxnum);
	if (tmp == NULL)
		exit(1);
	switch (mode) {
		case 1:
			issueshm = tmp;
			break;
		case 2:
			goodbyeshm = tmp;
			break;
		default:
			break;
	}

	if (labs(now - tmp[0].update) < 86400 && ftime < tmp[0].update) {
		return 1;
	}
	if ((fffd = fopen(fname, "r")) == NULL) {
		return 0;
	}
	while ((fgets(buf, FILE_BUFSIZE, fffd) != NULL) && nowfn < maxnum) {
		if (lines > FILE_MAXLINE)
			continue;
		if (strstr(buf, "@logout@") || strstr(buf, "@login@")) {
			tmp[nowfn].fileline = lines;
			tmp[nowfn].update = now;
			nowfn++;
			lines = 0;
			continue;
		}
		ptr = tmp[nowfn].line[lines];
		memcpy(ptr, buf, sizeof(buf));
		lines++;
	}
	fclose(fffd);
	tmp[nowfn].fileline = lines;
	tmp[nowfn].update = now;
	nowfn++;
	tmp[0].max = nowfn;
	return 1;
}
// 将fname文件映射到内存,结构为struct STATSHM
int fill_statshmfile(char* fname, int mode) {
	FILE * fp;
	time_t ftime;
	char * ptr;
	char buf[FILE_BUFSIZE];
	struct stat st;
	time_t now;
	int lines = 0;
	if (stat(fname, &st) < 0) {
		return 0;
	}
	ftime = st.st_mtime;
	now = time(0);

	if (mode == 0 || statshm == NULL) {
		statshm = attach_shm("STAT_SHMKEY", 5100, sizeof(struct STATSHM) * 2);
		if (statshm == NULL)
			exit(1);
	}
	if (labs(now - statshm[mode].update) < 86400 && ftime
			< statshm[mode].update) {
		return 1;
	}
	if ((fp = fopen(fname, "r")) == NULL) {
		return 0;
	}
	memset(&statshm[mode], 0, sizeof(struct STATSHM));
	while ((fgets(buf, FILE_BUFSIZE, fp) != NULL) && lines < FILE_MAXLINE) {
		ptr = statshm[mode].line[lines];
		memcpy(ptr, buf, sizeof(buf));
		lines++;
	}
	fclose(fp);
	statshm[mode].update = now;
	return 1;
}

// 显示一个映射在内存的文件内容
void show_shmfile(struct FILESHM *fh) {
	int i;
	char buf[FILE_BUFSIZE];
	for (i = 0; i < fh->fileline; i++) {
		strcpy(buf, fh->line[i]);
		showstuff(buf/*, 0*/);
	}
}
// 显示映射在内存中的STATSHM结构的文件内容
int show_statshm(char* fh, int mode)
{
	int i;
	char buf[FILE_BUFSIZE];
	if (fill_statshmfile(fh, mode)) {
		if ((mode == 0 && DEFINE(DEF_GRAPH)) || (mode == 1
				&& DEFINE(DEF_TOP10))) {
			screen_clear();

			for (i = 0; i <= 24; i++) {
				if (statshm[mode].line[i] == NULL)
					break;
				strcpy(buf, statshm[mode].line[i]);
				outs(buf);
			}
		}
		if (mode == 1)
			shmdt(statshm);
		return 1;
	}
	return 0;
}
// 显示离版画面
void show_goodbyeshm(void)
{
	int logouts;
	logouts = goodbyeshm[0].max;
	screen_clear();
	show_shmfile(&goodbyeshm[(currentuser.numlogins % ((logouts <= 1) ? 1 : logouts))]);
	shmdt(goodbyeshm);
}

void show_issue(void)
{
	int issues = issueshm[0].max;
	show_shmfile(&issueshm[(issues <= 1) ? 0 :((time(0) / 86400) % (issues))]);
	shmdt(issueshm);
}
