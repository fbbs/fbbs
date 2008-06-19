#include <stdio.h>
//#include <search.h>

#define FIFOFILE "/home/bbs/anonfifo"

int main(int argc, char *argv[])
{
	char line[256];
	FILE *fp;
	mkfifo(FIFOFILE, 0);
	daemon(1,1);
	fp=fopen(FIFOFILE, "r");
	if(!fp)
	{
		printf("cannot open fifo file!\n");
		exit(1);
	}

	while(fgets(line, 256, fp))
	{
		
	
	}

	fclose(fp);

	unlink(FIFOFILE);
	
	return 0;
}
