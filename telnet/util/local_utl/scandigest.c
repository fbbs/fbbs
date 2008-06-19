#include "bbs.h"
void main(int argc,char *argv[])
{
	char file[80],newfile[80],cmd[200];
	int i=0,count=0;
	struct fileheader x;
	FILE * fp,*newfp;
	if (argc <=1 )
		{
		printf("usage:%s boardname\n",argv[0]);
		return;
		}
	sprintf(newfile,"/home/bbs/boards/%s/.DIGEST.new",argv[1]);
	sprintf(file,"/home/bbs/boards/%s/.DIGEST",argv[1]);
	if ( (fp=fopen(file, "r")) ==NULL)
	{
			printf("can't open file %s\n",file);
			return;
	}
	if ( (newfp=fopen(newfile,"w"))==NULL)
	{
			printf("can't write file %s\n",newfile);
			return;
	}
	printf("Entering..............%s\n",argv[1]);
	while (   fread(&x,sizeof(x),1,fp) ) {
	if (x.id ==i ) {
			count++;
			continue;
	}
	 fwrite(&x, sizeof(x), 1, newfp);
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
