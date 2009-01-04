#include "libweb.h"

//char mybrd[22][80];
struct goodbrdheader gdbrd; //modified by cometcaptor 2007-07-13

int mybrdnum=0;
struct boardheader x;

/*
int ismybrd(char *board) {
	int n;
	for(n=0; n<mybrdnum; n++) 
	{
		if(!strcasecmp(mybrd[n], board)) 
			return n;
	}
	return -1;
}
*/

int main() {
	FILE *fp;
	char file[200], board[200];
	int i=0;
    int bpos = 0;
    int alreadyexists = 0; //added by cometcaptor 2007-07-13
	init_all();
	printf("<b>收藏夹 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	strsncpy(board, getparm("board"), 32);
    bpos = getbnum(board, &currentuser) - 1;
	if(!loginok) http_fatal("超时或未登录，请重新login");
	sprintf(file, "home/%c/%s/.goodbrd", toupper(currentuser.userid[0]), currentuser.userid);
	fp=fopen(file, "r");
        mybrdnum=0;
        if(fp!=NULL)
        {
                //while(fscanf(fp, "%s\n", mybrd[mybrdnum])!= EOF)
                while (fread(&gdbrd,sizeof(struct goodbrdheader),1,fp))
                {
                        mybrdnum++;
                        //if(mybrdnum>19) //modified by cometcaptor 2007-07-13
                        if (mybrdnum >= GOOD_BRC_NUM)
                                break;
                        if ((!(gdbrd.flag & BOARD_CUSTOM_FLAG))&&(bpos == gdbrd.pos))
                            alreadyexists = 1;
                }
                fclose(fp);
        }
	if(mybrdnum>=GOOD_BRC_NUM) http_fatal("您预定讨论区数目已达上限，不能增加预定");
	//if(ismybrd(board)>=0) http_fatal("您已经预定了这个讨论区");
    if (alreadyexists) http_fatal("您已经预定了这个讨论区");//modified by cometcaptor 2007-07-13
	if(!has_read_perm(&currentuser, board)) http_fatal("此讨论区不存在");
	//strcpy(mybrd[mybrdnum], board);
	//mybrdnum++;
    /*
	fp=fopen(file, "w");
        if(fp!=NULL)
        {
        	for(i=0;i<mybrdnum;i++)
                {
			fprintf(fp,"%s\n",mybrd[i]);
		}
                fclose(fp);
        }
        */
    gdbrd.id++;
    gdbrd.pid = 0;
    gdbrd.pos = bpos;
    strcpy(gdbrd.title,bcache[bpos].title);
    strcpy(gdbrd.filename,bcache[bpos].filename);
    gdbrd.flag = bcache[bpos].flag;
    fp = fopen(file,"ab");
    if (fp != NULL)
    {
        fwrite(&gdbrd,sizeof(struct goodbrdheader),1,fp);
        fclose(fp);
    }
	printf("<script>top.f2.location='bbsleft'</script>\n");
	printf("预定讨论区成功<br><a href='javascript:history.go(-1)'>快速返回</a>");
	http_quit();
}
