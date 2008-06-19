#include "../../include/bbs.h"
void main() 
{
	char dir[100],newdir[100],c;
	int i=0;
	struct boardheader x;
	FILE * fp,*newfp;
	//sprintf(newdir,"%s/.BOARDS.tmp",BBSHOME);
	sprintf(dir,"%s/.BOARDS",BBSHOME);
	fp=fopen(dir, "r");
	//newfp=fopen(newdir,"w");
	while (   fread(&x,sizeof(struct boardheader),1,fp) ) {
	printf("%s\n",x.filename);
	//printf("Save/No?");
	//c=getchar();
	//if (!( c == 'N' || c == 'n' )) {
	// fwrite(&x, sizeof(struct boardheader), 1, newfp);
	//}
	//if (i<500 ){
	//	i++;
	//	fwrite(&x, sizeof(struct boardheader), 1, newfp);
	//}
    i++;
	}
	printf("%d\n",i);
	fclose(fp);
	//fclose(newfp);
}
