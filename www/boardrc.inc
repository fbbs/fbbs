#define BRC_MAXSIZE     50000
#define BRC_MAXNUM      60
#define BRC_STRLEN      20
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))
char    brc_buf[BRC_MAXSIZE];
int     brc_size;
char    brc_name[BRC_STRLEN];
int     brc_list[BRC_MAXNUM], brc_num;

char *brc_getrecord(char *ptr, char *name, int *pnum, int *list) {
        int     num;
        char   *tmp;
        strncpy(name, ptr, BRC_STRLEN);
        ptr += BRC_STRLEN;
        num = (*ptr++) & 0xff;
        tmp = ptr + num * sizeof(int);
        if (num > BRC_MAXNUM) num = BRC_MAXNUM;
        *pnum = num;
        memcpy(list, ptr, num * sizeof(int));
        return tmp;
}

char *brc_putrecord(char *ptr, char *name, int num, int *list) {
        if (num > 0 ) {
                if (num > BRC_MAXNUM) {
                        num = BRC_MAXNUM;
                }
                strncpy(ptr, name, BRC_STRLEN);
                ptr += BRC_STRLEN;
                *ptr++ = num;
                memcpy(ptr, list, num * sizeof(int));
                ptr += num * sizeof(int);
        }
        return ptr;
}

void brc_update(char *userid, char *board) {
        char    dirfile[STRLEN], *ptr;
        char    tmp_buf[BRC_MAXSIZE - BRC_ITEMSIZE], *tmp;
        char    tmp_name[BRC_STRLEN];
        int     tmp_list[BRC_MAXNUM], tmp_num;
        int     fd, tmp_size;
        ptr = brc_buf;
        if (brc_num > 0) {
                ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
        }
        if (1) {
                sethomefile(dirfile, userid, ".boardrc");
                if ((fd = open(dirfile, O_RDONLY)) != -1) {
                        tmp_size = read(fd, tmp_buf, sizeof(tmp_buf));
                        close(fd);
                } else {
                        tmp_size = 0;
                }
        }
        tmp = tmp_buf;
        while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z')) {
                tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
                if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
                        ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
                }
        }
        brc_size = (int) (ptr - brc_buf);
        if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
                ftruncate(fd, 0);
                write(fd, brc_buf, brc_size);
                close(fd);
        }
}

int brc_init(char *userid, char *boardname) {
	FILE *fp;
        char dirfile[STRLEN], *ptr;
	int tmpno=0;
        sethomefile(dirfile, userid, ".boardrc");
        if(fp=fopen(dirfile, "r")) {
                brc_size=fread(brc_buf, 1, BRC_MAXSIZE, fp);
                fclose(fp);
        } else {
                brc_size = 0;
        }
        ptr=brc_buf;
        while(ptr< &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z')) {
                ptr=brc_getrecord(ptr, brc_name, &brc_num, brc_list);
                if (!strncasecmp(brc_name, boardname, BRC_STRLEN)) return brc_num;
        }
        strncpy(brc_name, boardname, BRC_STRLEN);
        brc_list[0] = 1;
        brc_num=1;
        return 0;
}

int brc_add_read(char *filename) {
        int     ftime, n, i;
        if(!loginok) return;
        ftime=atoi(&filename[2]);
        if(filename[0]!='M' && filename[0]!='G')  return;
        if(brc_num<=0) {
                brc_list[brc_num++] = ftime;
                return;
        }
        for (n = 0; n < brc_num; n++) {
                if (ftime == brc_list[n]) {
                        return;
                } else if (ftime > brc_list[n]) {
                        if (brc_num < BRC_MAXNUM)
                                brc_num++;
                        for (i = brc_num - 1; i > n; i--) {
                                brc_list[i] = brc_list[i - 1];
                        }
                        brc_list[n] = ftime;
                        return;
                }
        }
        if(brc_num<BRC_MAXNUM) brc_list[brc_num++] = ftime;
}

int brc_un_read(char *file) {
        int ftime, n;
        if(file[0]!='M' && file[0]!='G') return 0;
	ftime=atoi(file+2);
        if (brc_num <= 0)
                return 1;
        for (n=0; n<brc_num; n++) {
                if(ftime>brc_list[n]) return 1;
                if(ftime==brc_list[n]) return 0;
        }
        return 0;
}

int brc_clear() {
	int i;
	char filename[20];
	if(!loginok) return;
	for(i=0; i<60; i++) {
		sprintf(filename, "M.%d.A", time(0)-i);
		brc_add_read(filename);
	}
}

int brc_has_read(char *file) {
	return !brc_un_read(file);
}
