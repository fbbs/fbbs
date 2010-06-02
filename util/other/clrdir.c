#include "bbs.h"

#define DOTWORK ".wrk"
#define DOTBAK ".bak"
#define BAKDIR "/bbsbackup/bbs/"

char curdir[500];
char workdir[500];
char buf[200];

void 
runcmd(s)
char *s;
{
    //printf("%s\n",s);
    system(s);
}

void
report(s)
char *s ;
{
    static int disable = NA ;
    int fd ;

    if(disable)
        return ;
    if((fd = open("clrlog",O_WRONLY|O_CREAT,0644)) != -1 ) {
        char buf[512] ;
        char timestr[10], *thetime;
        time_t dtime;
        time(&dtime);
        thetime = ctime(&dtime);
        strncpy(timestr, &(thetime[11]), 8);
        timestr[8] = '\0';
        fb_flock(fd,LOCK_EX) ;
        lseek(fd,0,SEEK_END) ;
        sprintf(buf,"%s %s\n", timestr, s) ;
        write(fd,buf,strlen(buf)) ;
        fb_flock(fd,LOCK_UN) ;
        close(fd) ;
        return ;
    }
    disable = YEA ;
    return ;
}

void clrdir(DotFile)
char * DotFile;
{
	struct fileheader fhdr;
	char cmd[500];
	int fid;
	
	if((fid = open(DotFile,O_RDONLY,0)) == -1) return;
	while(read(fid,&fhdr,sizeof fhdr) == sizeof fhdr) {
		sprintf(cmd,"mv %s/%s %s",curdir,fhdr.filename,workdir);
		runcmd(cmd);
	}
	
	close(fid);
}

void
main(argc, argv)
int argc;
char **argv;
{
	char dirfile[500];
	char backupdir[500];

	char cmdbuf[500];

	if (argc!=3) {
		printf("Usage: clrdir <ParentPath> <DirName>.\nExample: clrdir boards sysop.\n");
		exit(-1);
	}
	
	
	sprintf(workdir,"%s/%s%s",argv[1],argv[2],DOTWORK);
	sprintf(backupdir,"%s%s",BAKDIR,argv[1]);
	sprintf(curdir,"%s/%s",argv[1],argv[2]);
	
	sprintf(cmdbuf,"mkdir %s",workdir);
	runcmd(cmdbuf);
	
	sprintf(dirfile,"%s/%s/%s",argv[1],argv[2],DOT_DIR);
	clrdir(dirfile);
	sprintf(dirfile,"%s/%s/%s",argv[1],argv[2],DIGEST_DIR);
	clrdir(dirfile);
	
	sprintf(cmdbuf,"mv %s/.??* %s",curdir,workdir);
	runcmd(cmdbuf);
	sprintf(cmdbuf,"mv %s/deny_users %s",curdir,workdir);
	runcmd(cmdbuf);
	sprintf(cmdbuf,"mv %s %s",curdir,backupdir);
	runcmd(cmdbuf);
	sprintf(cmdbuf,"mv %s %s",workdir,curdir);
	runcmd(cmdbuf);

 printf("complete %s.\n",argv[2]);

}
 
