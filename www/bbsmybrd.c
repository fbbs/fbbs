#include "libweb.h"
struct boardheader data[MAXBOARD];
//char mybrd[40][80];
//int mybrdnum=0;
//改用统一的goodboard结构 cometcaptor 2007-04-23
struct goodboard {
  struct goodbrdheader boards[GOOD_BRC_NUM];
  unsigned int nowpid;
  int             num;
} GoodBrd;

int cmpboard(b1, b2)
struct boardheader *b1, *b2;
{
        return strcasecmp(b1->filename, b2->filename);
}

int main() {
	int total=0, i, i1, i2, type;
	char path[200];
	char *brd;
   	FILE *fp;
	init_all();
	printf("<b>收藏夹 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	if(!loginok) 
		http_fatal("尚未登录或者超时");
	type=atoi(getparm("type"));
	if(type!=0) 
	{
		read_submit();
		http_quit();
	}
   	sprintf(path, "home/%c/%s/.goodbrd", toupper(currentuser.userid[0]), currentuser.userid);
   	fp=fopen(path, "rb");
	//mybrdnum=0;
	GoodBrd.num = 0;
	if(fp!=NULL)
	{
		/*修改goodbrd结构做出的相应改动 cometcaptor 2007-04-23
		while(fscanf(fp, "%s\n", mybrd[mybrdnum])!= EOF)
		{
			mybrdnum++;
			if(mybrdnum>39)
				break;
		}
		*/
		while (fread(&GoodBrd.boards[GoodBrd.num],sizeof(struct goodbrdheader), 1, fp))
		{
			if (GoodBrd.boards[GoodBrd.num].flag & BOARD_CUSTOM_FLAG)
				GoodBrd.num++;
			else
			{
				i = GoodBrd.boards[GoodBrd.num].pos;
				if ((bcache[i].filename[0]) &&
								 (bcache[i].flag & BOARD_POST_FLAG //p限制版面
								 || HAS_PERM(bcache[i].level) //权限足够
								 ||(bcache[i].flag & BOARD_NOZAP_FLAG))) //不可zap
					GoodBrd.num++;
			}
			if (GoodBrd.num == GOOD_BRC_NUM)
				break;
		}
		fclose(fp);
	}

   	printf("<center>\n");
	printf("<b>个人预定讨论区管理(您目前预定了%d个讨论区，最多可预定%d个)</b><br>\n", GoodBrd.num, GOOD_BRC_NUM);
	printf("<form action='bbsmybrd?type=1&confirm1=1' method=post>\n");
	printf("<input type=hidden name=confirm1 value=1>\n");
	printf("<table>\n");
	for(i=0; i<MAXBOARD; i++) 
	{
		if(has_read_perm(&currentuser, bcache[i].filename)) 
		{
				if ((bcache[i].flag & BOARD_CLUB_FLAG)&& (bcache[i].flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, bcache[i].filename)&& !isclubmember(currentuser.userid, bcache[i].filename))
						        continue;
			memcpy(&data[total], &(bcache[i]), sizeof(struct boardheader));
			total++;
		}
	}
	qsort(data, total, sizeof(struct boardheader), cmpboard);
	for(i=0; i<total; i++) 
	{
               //if ( (data[i].group == 0) && (data[i].flag &BOARD_DIR_FLAG) ) //根目录不可加入收藏夹06.3.5
               //    continue;
		if(i%3==0) 
			printf("<tr>\n");
		printf("\t<td><input type=checkbox name=%s %s>",data[i].filename, ismyboard(data[i].filename)?" checked":" ");
                if  ( data[i].flag & BOARD_DIR_FLAG )   //modify for dir Danielfree 06.3.5
		printf("<a href=bbsboa?board=%s>%s(%s)</a></td>\n",data[i].filename, data[i].filename, data[i].title+11);
               else 
		printf("<a href=bbsdoc?board=%s>%s(%s)</a></td>\n",data[i].filename, data[i].filename, data[i].title+11);
		if(i%3==2)
			printf("</tr>\n");
	}
	printf("\n</table>\n");
	printposttable_lite();
	printf("<input type=submit value=确认预定> <input type=reset value=复原>\n");
	printf("</form>\n");
	http_quit();
}

int ismyboard(char *board) //modified by cometcaptor 2007-04-23 
{
	int i;
	int bpos = getbnum(board, &currentuser) - 1;
	//for(i=0; i<mybrdnum; i++)
	for (i = 0; i<GoodBrd.num && i<GOOD_BRC_NUM; i++)
	{
		//if(strcasecmp(board, mybrd[i])==0)
		if ((!(GoodBrd.boards[i].flag & BOARD_CUSTOM_FLAG))&&(bpos == GoodBrd.boards[i].pos)) 
			return 1;
	}
	return 0;
}

int read_submit() {
        int i, i1, i2, n;
        char buf1[200];
        char *brd;
        FILE *fp;
	int mybrdnum=0;
	struct boardheader *bh;
	int bhi;
        if(!strcmp(getparm("confirm1"), "")) http_fatal("参数错误");
        for(i=0; i<parm_num; i++) 
		{
                if(!strcasecmp(parm_val[i], "on")) 
				{
                        //if(mybrdnum>=40) http_fatal("您试图预定超过40个讨论区"); modified by cometcaptor 2007-04-23 
			if(mybrdnum>=GOOD_BRC_NUM) http_fatal("您试图预定的讨论区数目超过上限");
                        if(!has_read_perm(&currentuser, parm_name[i])) 
			{
				printf("警告: 无法预定'%s'讨论区<br>\n", nohtml(parm_name[i]));
                                continue;
                        }
                        //strsncpy(mybrd[mybrdnum], parm_name[i], 80);
			bhi = getbnum(parm_name[i], &currentuser);
			if (bhi)
			{
				bhi--;
				GoodBrd.boards[mybrdnum].id = mybrdnum+1;
				GoodBrd.boards[mybrdnum].pid = 0;
				GoodBrd.boards[mybrdnum].pos = bhi;
				strcpy(GoodBrd.boards[mybrdnum].title, bcache[bhi].title);
				strcpy(GoodBrd.boards[mybrdnum].filename, bcache[bhi].filename);
				GoodBrd.boards[mybrdnum].flag = bcache[bhi].flag;
                        	mybrdnum++;
			}
                   }
		}
        sprintf(buf1, "home/%c/%s/.goodbrd", toupper(currentuser.userid[0]), currentuser.userid);
		fp=fopen(buf1, "wb");
		if(fp!=NULL)
		{
			/*
			for(i=0;i<mybrdnum;i++)
				fprintf(fp,"%s\n",mybrd[i]);
			fclose(fp);*/
			fwrite(&GoodBrd.boards[0], sizeof(struct goodbrdheader), mybrdnum, fp);
			fclose(fp);
		}
        printf("<script>top.f2.location='bbsleft'</script>修改预定讨论区成功，您现在一共预定了%d个讨论区:<hr>\n", mybrdnum);
        printf("[<a href='javascript:history.go(-2)'>返回</a>]");
			
}
