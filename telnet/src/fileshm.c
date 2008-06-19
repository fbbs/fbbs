/*
$Id: fileshm.c 366 2007-05-12 16:35:51Z danielfree $
*/

#include "bbs.h"

struct FILESHM {	//内存映射文件结构
    char    line[FILE_MAXLINE][FILE_BUFSIZE];	//[25][200]
    int     fileline;
    int     max;
    time_t  update;								//更新时间
};

struct STATSHM {
    char    line[FILE_MAXLINE][FILE_BUFSIZE];
    time_t  update;
};

struct FILESHM	*	welcomeshm;
struct FILESHM	*	goodbyeshm;
struct FILESHM	*	issueshm;
struct STATSHM	*	statshm;

// 将文件fname映射到内存,键值为shmkey,用mode来区别不同的文件
int	fill_shmfile(int  mode,char* fname,char * shmkey) {
    FILE   *	fffd;
    char   *	ptr;
    char    	buf[FILE_BUFSIZE];
    struct stat st;
    time_t  	ftime, now;
    int     	lines = 0, nowfn = 0, maxnum;
    struct FILESHM *	tmp;
    switch (mode) {
    case 1:
        maxnum = MAX_ISSUE;
        break;
    case 2:
        maxnum = MAX_GOODBYE;
        break;
    case 3:
        maxnum = MAX_WELCOME;
        break;
    default:
        break;
    }
    now = time(0);
    if (stat(fname, &st) < 0) {
        return 0;
    }
    ftime	=	st.st_mtime;
    tmp = (void *) attach_shm(shmkey, 5000 + mode * 10, sizeof(struct FILESHM) * maxnum);
    switch (mode) {
    case 1:
        issueshm = tmp;
        break;
    case 2:
        goodbyeshm = tmp;
        break;
    case 3:
        welcomeshm = tmp;
        break;
    default:
        break;
    }

    if (abs(now - tmp[0].update) < 86400 && ftime < tmp[0].update) {
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
int	fill_statshmfile(char*	fname,int	mode) {
    FILE   *	fp;
    time_t  	ftime;
    char   *	ptr;
    char    	buf[FILE_BUFSIZE];
    struct stat st;
    time_t  	now;
    int     	lines = 0;
    if (stat(fname, &st) < 0) {
        return 0;
    }
    ftime = st.st_mtime;
    now = time(0);

    if (mode == 0 || statshm == NULL) {
        statshm = (void *) attach_shm("STAT_SHMKEY", 5100, sizeof(struct STATSHM) * 2);
    }
    if (abs(now - statshm[mode].update) < 86400 && ftime < statshm[mode].update) {
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
void	show_shmfile(struct FILESHM *fh) {
    int     i;
    char    buf[FILE_BUFSIZE];
    for (i = 0; i < fh->fileline; i++) {
        strcpy(buf, fh->line[i]);
        showstuff(buf/*, 0*/);
    }
}
// 显示映射在内存中的STATSHM结构的文件内容
int	show_statshm(char* fh,int mode) {
    int     i;
    char    buf[FILE_BUFSIZE];
    if (fill_statshmfile(fh, mode)) {
        if ((mode == 0 && DEFINE(DEF_GRAPH)) || (mode == 1 && DEFINE(DEF_TOP10))) {
            clear();

            for (i = 0; i <= 24; i++) {
                if (statshm[mode].line[i] == NULL)
                    break;
                strcpy(buf, statshm[mode].line[i]);
                prints(buf);
            }
        }
        if (mode == 1 )
            shmdt(statshm);
        return 1;
    }
    return 0;
}
// 显示离版画面
void	show_goodbyeshm() {
    int     logouts;
    logouts = goodbyeshm[0].max;
    clear();
    show_shmfile(&goodbyeshm[(currentuser.numlogins % ((logouts <= 1) ? 1 : logouts))]);
    shmdt(goodbyeshm);
}
// 显示进版画面
void	show_welcomeshm() {
    int     welcomes;
    welcomes = welcomeshm[0].max;
    clear();
    show_shmfile(&welcomeshm[(currentuser.numlogins % ((welcomes <= 1) ? 1 : welcomes))]);
    if (DEFINE(DEF_TOP10))
        pressanykey();
    shmdt(welcomeshm);
}

//显示issueshm中的内容?
void	show_issue() {
    int	issues = issueshm[0].max;
    show_shmfile(&issueshm[(issues <= 1) ? 0 :((time(0) / 86400) % (issues))]);
    shmdt(issueshm);
}
