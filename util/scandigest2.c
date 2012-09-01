#include "bbs.h"
void main(int argc,char *argv[])
{
	char file[80],filepath[200],cmd[200];
	int i=0,count=0;
	struct fileheader x;
	FILE * fp,*newfp;
	if (argc <=1 )
		{
		printf("usage:%s boardname\n",argv[0]);
		return;
		}
	sprintf(file,"/home/bbs/boards/%s/.DIGEST",argv[1]);
	if ( (fp=fopen(file, "r")) ==NULL)
	{
			printf("can't open file %s\n",file);
			return;
	}
	printf("Entering..............%s\n",argv[1]);
	while (   fread(&x,sizeof(x),1,fp) ) {
	sprintf(filepath,"/home/bbs/boards/%s/%s",argv[1],x.filename);
	if ( ) {
			count++;
			continue;
	}
	 i=x.id;
	}
	printf("%d\n",i);
	printf("fixed %d posts\n",count);
	if ( count > 0 ) 
	{
		printf("replace .digest now...\n");
		sprintf(cmd,"mv %s %s",newfile,file);
		system(cmd);
		sprintf(cmd,"chown bbs:bbs %s",file);
		system(cmd);
		printf("done!\n");
	}
	fclose(fp);
	fclose(newfp);
}
