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
	if (argc == 3){
		rebuild(argv[1],argv[2]);
		exit(0);
	}else{
		printf("Usage: %s <BoardName> <uploadID>\n",argv[0]);
		exit(1);
	}

}

int rebuild(char* board,char *id){
	int all=0,i ;
	char pathname[128],buf[128];
	int fd;
	DIR *dp;
	
	sprintf(pathname, "%s/upload/%s",BBSHOME,board);
	printf("1. 进入目录 %s\n",pathname);
	if( (dp = opendir(pathname))==NULL){
		printf("OpenDir error for %s\n",pathname);
		return;
	}
	
	printf("2. 生成 .DIR\n");
	all = build_dir( pathname,id);
	printf("共得到%d篇附件\n",all);
    sprintf(buf,"chown bbs:bbs -R %s",pathname);
    system(buf);
}

int
build_dir (char *pathname,char *id)
{
	unsigned int count=0;
	char lpathname[256], buf[128],dir[256];
	struct stat st;
	struct fileheader *buffer;
	FILE	*fd,*pic;
	sprintf (lpathname, "%s", pathname);
	sprintf(dir, "%s/.DIR", lpathname);
	count = 0;
	fd = fopen (dir, "r");
	if (fd == NULL) {
			printf("Open Dir error for %s\n",dir);
			return -1;
	} 
	sprintf(buf,"%s.bak",dir);
	rename(dir,buf);
	fd = fopen (dir, "w");
	sprintf(buf,"%s/.file",lpathname);
	pic = fopen(buf,"r");
	if ( pic == NULL )
	{
			printf("Open pic list error for %s\n",lpathname);
			return -1;
	}
	char picfile[100],*ptr;
	chdir(lpathname);
	while ( fgets(picfile,100,pic)!=NULL )
	{
		if( (ptr=strchr(picfile,'\n'))!=NULL)
			*ptr='\0';
		printf("Processing file: %s\n",picfile);
		strncpy(buffer->filename,picfile,100);
		strncpy(buffer->title,buffer->filename,100);
		buffer->id=count++;
		buffer->reid=1;
		buffer->timeDeleted=time(0);
		strcpy(buffer->owner,id);
		fwrite(buffer,sizeof(struct fileheader),1,fd);
	}
	fclose(pic);				
	fclose(fd);
	return count;
	}


