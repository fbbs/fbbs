#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <dirent.h>
#include        <limits.h>
#include        "../../include/bbs.h"
#define BBSHOME "/home/bbs"
#define TRUE  1
#define FALSE 0

main(int argc,char *argv[])
{
	if (argc == 2){
		rebuild(argv[1]);
		exit(0);
	}else{
		printf("Usage: %s <BoardName>\n",argv[0]);
		exit(1);
	}

}

int rebuild(char* board){
	int all=0,i ;
	char pathname[128];
	int fd;
	DIR *dp;
	
	sprintf(pathname, "%s/boards/%s",BBSHOME,board);
	printf("1. 进入目录 %s\n",pathname);
	if( (dp = opendir(pathname))==NULL){
		printf("OpenDir error for %s\n",pathname);
		return;
	}
	
	printf("2. 生成 .DIR\n");
	all = build_dir( pathname);
}

int
build_dir (char *pathname)
{
	int fd, index, total[5], i, j, k, same, count, a,b;
	char lpathname[256], buf[128];
	char dir[5][128];
	struct stat st;
	struct fileheader *buffer[5];
	sprintf (lpathname, "%s", pathname);
	sprintf(dir[0], "%s/.NOTICE", lpathname);
	sprintf(dir[1], "%s/.DIR", lpathname);
	sprintf(dir[2], "%s/.TRASH", lpathname);
	sprintf(dir[3], "%s/.JUNK", lpathname);
	sprintf(dir[4], "%s/.DIGEST", lpathname);
	count = 0;
	for (i = 0; i < 5 ; i++ ){
		fd = open (dir[i], O_RDONLY);
		if (fd == -1) {
			printf("Open Dir error for %s\n",dir[i]);
			total[i] = 0;
		} else {
			(void) fstat (fd, &st);
			total[i] = st.st_size / sizeof (struct fileheader);
			buffer[i] = (struct fileheader *) calloc (total[i], sizeof (struct fileheader));
			if (buffer[i] == NULL){
				fprintf (stderr, "Out of memory!\n");
				return -1;
			}
			if (read (fd, buffer[i], (size_t) st.st_size) < (ssize_t) st.st_size)
			{
				free (buffer[i]);
				fprintf (stderr, "Can not read .DIR file!\n");
				return -1;
			}
			(void) close (fd);
		}
	}
	for (i = 0; i < 5 ; i++ ){
		if (total[i] == 0) continue;
		for (index = 0; index < total[i] ; index++){
			buffer[i][index].id = index + count + 1;
			same = 0;
			for (k = 0; k <= i && !same; k++) {
				if (total[k] == 0) continue;
				for (j = 0; (k<i && j< total[k]) || (k==i && j < index) && !same; j++) {
					if (strcmp (buffer[k][j].title, buffer[i][index].title) == 0){
						same = 1;
						a = k;
						b = j;
						continue;
					}
					if (!strncmp ("Re:", buffer[i][index].title, 3) || !strncmp ("RE:", buffer[i][index].title, 3))  {
						if (strcmp (buffer[k][j].title, buffer[i][index].title + 4) == 0){
							same = 1;
							a = k;
							b = j;
							continue;
								
						}
					}
					if (!strncmp ("Re:", buffer[k][j].title, 3) || !strncmp ("RE:", buffer[k][j].title, 3))  {
						if (strcmp (buffer[k][j].title + 4, buffer[i][index].title) == 0){
							same = 1;
							a = k;
							b = j;
							continue;						
						}
					}
				}
			}
			if (same){
				buffer[i][index].gid = buffer[a][b].gid;
			}
			else
				buffer[i][index].gid = buffer[i][index].id;
			buffer[i][index].reid = buffer[i][index].gid;
		}
		sprintf(buf,"%s.bak",dir[i]);
		rename(dir[i],buf);
		fd = open (dir[i], O_RDWR|O_CREAT, 0640);
		write(fd, buffer[i], total[i] * sizeof(struct fileheader));
		close(fd);
		chown(dir[i],9999,9999);
		if (i != 4)
			count += index;
		
	}
	for (i = 0; i < 5; i++){
		if (total[i])
			free (buffer[i]);
	}
	return count;
}


