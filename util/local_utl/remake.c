#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <dirent.h>
#include        <limits.h>
#include        "../../include/bbs.h"
#define BBSHOME "/home/bbs"
#define TRUE  1
#define FALSE 0

int  fileflag=1;
int  totalfound=0,totalfile=0;
char    control[80];

struct postnode
{
char filename[20];
int num;
};

report()
{}

main(argc,argv)
int argc;
char *argv[];
{
        char dir[80];

        if(argc<2)
        {
                printf("Usage: %s <BoardName>\n",argv[0]);
                exit(1);
        }
        if (argv[2]=='\0')
           sprintf(dir,"%s/boards/%s",BBSHOME,argv[1]);
        else
           sprintf(dir,"%s/mail/%c/%s",BBSHOME,toupper(argv[1][0]),argv[1]);   
        myftw(dir);
}  

int
do_remake(path,file)
char *path,*file;
{
        FILE *fp;
        char *ptr,*ptr2;
        char filename[80];
        char buf[256];
        struct fileheader fh;
        int step=0;

        sprintf(filename,"%s/%s",path,file);
        if( (fp=fopen(filename,"r")) == NULL)
        {
                printf("Open error.. \n");
                return;
        }
        strncpy(fh.filename,file,sizeof(fh.filename));
        fh.level=0;
        memset(&fh.accessed,0,sizeof(fh.accessed));
        while( fgets(buf,256,fp)!=NULL)
        {
                if(strstr(buf,"^[[1;33m发信人: ")||strstr(buf,"发信人: ")||strstr(buf,"作  者: ")||strstr(buf,"寄信人: "))
                {		
						if (strstr(buf,"^[[1;33m发信人: "))
							ptr=&buf[16];
						else
							ptr=&buf[8];
                        ptr2=strchr(ptr,' ');
                        if(ptr2!=NULL)
                        {
                                *ptr2='\0';
                        }
                        ptr2=strchr(ptr,'@');
                        if(ptr2!=NULL)
                        {
                                *ptr2='\0';
                        }
                        ptr2=strchr(ptr,'.');
                        if(ptr2!=NULL)
                        {
                                *(ptr2+1)='\0';
                        }
                        strncpy(fh.owner,ptr,sizeof(fh.owner));
                        step=1;
                }
                if(strstr(buf,"标  题: ")||strstr(buf,"题  目: "))
                {
                        ptr=&buf[8];
                        ptr[strlen(ptr)-1]=0;
                        strncpy(fh.title,ptr,sizeof(fh.title));
                        step=2;
                }
                if(step==2)
                        break;
        }
        fclose(fp);
        if(step==2)
        {
                fh.filename[ STRLEN - 1 ] = 'S';
                fh.filename[ STRLEN - 2 ] = 'S';
                append_record(control,&fh,sizeof(fh));
                return 1;
        }
        else
        {
                //unlink(filename);
		printf("%s\n", filename);
		return 0;
        }
}

int
cmpfname( brd, tmp )
struct postnode *brd, *tmp;
{
    return strcmp(brd->filename, tmp->filename);
}

int
do_sort(pn)
char *pn;
{
        struct postnode *allnode;
        struct fileheader post;
        char sfname[STRLEN];
        int i=0;
        int n;
	int total;
        FILE *tf;
        total=get_num_records(control,sizeof(struct fileheader));
        allnode=(struct postnode *)malloc(sizeof(struct postnode)*total);

        if((tf=fopen(control,"rb"))==NULL)
        {
                printf(".DIR cant open...");
                return ;
        }
        while(1)
        {
                if(fread(&post,sizeof(post),1,tf)<=0) break;
                allnode[i].num=i+1;
                strncpy(allnode[i].filename,post.filename,19);
                i++;
        }
        fclose(tf);
        qsort( allnode, i, sizeof( struct postnode ), cmpfname );
        sprintf(sfname,"%s/.DIR.sort",pn);
        for(n=0;n<total;n++)
        {
                get_record(control,&post,sizeof(post),allnode[n].num);
                append_record(sfname,&post,sizeof(post));
        }
        rename(sfname,control);
        free(allnode);
}

int myftw(pathname)
char *pathname;
{
  struct stat statbuf;
  DIR *dp;
  char buf[80];
  struct dirent *dirp;
  int all=0,done=0;  

  printf("1. 进入目录 %s\n",pathname);
  if( (dp = opendir(pathname))==NULL)
  {
       printf("OpenDir error for %s\n",pathname);
       return;
  }
  sprintf(control,"%s/.DIR",pathname);
  sprintf(buf,"%s.bak",control);
  rename(control,buf);
  printf("2. 整理文章，建立 .DIR\n");
  while((dirp = readdir(dp))!=NULL)
  {
       char pname[256];
       if(!strcmp(dirp->d_name,".")||!strcmp(dirp->d_name,"..")||dirp->d_name[0]!='M')
                continue;
       done+=do_remake(pathname,dirp->d_name);
       all++;
  }
  printf("3. 排序文章\n");
  do_sort(pathname);
  printf("%d 篇文章重建，%d 文章失败，已经删除\n",done,all-done);
  chown(control,9999,99);
}
int	append_record(	char   *filename,
		char   *record	,
		int     size
		)
{
	int     fd;
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) {
		report("open file error in append_record()");
		return -1;
	}
	FLOCK(fd, LOCK_EX);
	lseek(fd, 0, SEEK_END);
	if (safewrite(fd, record, size) == -1)
		report("apprec write err!");
	FLOCK(fd, LOCK_UN);
	close(fd);
	return 0;
}


int get_num_records(const char *filename, const int size)
{
	struct stat st;
	if (stat(filename, &st) == -1)
		return 0;
	return (st.st_size / size);
}

int get_record(char *filename, void *rptr, int size, int id)
{
	int fd;
	int ret;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	ret = get_record_handle(fd, rptr, size, id);
	close(fd);
	return ret;
}

int safewrite(int fd, char *const buf, int size)
{
	int		cc, sz = size, origsz = size;
	char   *bp = buf;

	do {
		cc = write(fd, bp, sz);
		if ((cc < 0) && (errno != EINTR)) {
			report("safewrite err!");
			return -1;
		}
		if (cc > 0) {
			bp += cc;
			sz -= cc;
		}
	} while (sz > 0);
	return origsz;
}

int get_record_handle(int fd,void *rptr,int size,int id)
{
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1)
		return -1;
	if (read(fd, rptr, size) != size)
		return -1;
	return 0;
}

