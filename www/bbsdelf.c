#include "BBSLIB.inc"

void recount_size(char *board)
{
      char path[256], cmd[256];
      sprintf(path, "%s/upload/%s", BBSHOME, board);
      if(dashd(path))
      {
          FILE *fp;
          sprintf(cmd , "du %s|cut -f1>%s/.size", path, path);
          system(cmd);
      }
}
												

int main() 
{
	FILE *fpdir,*fptrash;
	struct dir x;
	struct boardheader *brd;
	char file[80], board[80], filename[80];//,filename2[80];
	int start;
	int num=0;
	init_all();
	if(!loginok) http_fatal("匆匆过客无法删除文件, 请先登录");
	strsncpy(file, getparm("file"), 40);
	start=atoi(getparm("start"));
	strsncpy(board, getparm("board"),80);
	sprintf(filename,"%s/upload/%s/.DIR",BBSHOME,board);
	fpdir=fopen(filename, "r");
	if(fpdir==0) 
		http_fatal("空目录");
	while(1) 
	{
		if(fread(&x, sizeof(x), 1, fpdir)<=0) 
			break;
		if(!strncmp(x.filename, file, 36)) 
		{
			brd=getbcache(board);
			if(brd==0) 
				http_fatal("内部错误10002");
			if(strcasecmp(x.owner,currentuser.userid) && !has_BM_perm(&currentuser, board) && !(currentuser.userlevel & PERM_OBOARDS)) 
			{
				http_fatal("没有权限删除文件");
			}

			sprintf(filename,"%s/upload/%s/%s",BBSHOME,board,x.filename);
			if(unlink(filename))
			{
				http_fatal("删除文件出错");
			}
			/*
			sprintf(filename2,"%s_TRASH",filename);
			if(rename(filename,filename2))
			{
				http_fatal("删除文件出错");
			}
			strcat(x.showname,"_TRASH");			
			sprintf(filename,"%s/upload/%s/.TRASH", BBSHOME,x.board);
			fptrash=fopen(filename,"a");
			if(fptrash==NULL)
			{
				http_fatal("内部错误:写文件错误");
			}
			fwrite(&x,sizeof(struct dir),1,fptrash);
			fclose(fptrash);	
			*/
			sprintf(filename,"%s/upload/%s/.DIR",BBSHOME, board);
			del_record(filename, sizeof(struct dir), num);
			{
				char buf[256],log[100];
				sprintf(buf, "DEL [%s] %s %dB %s %s FILE:%s\n",cn_Ctime(time(0)), currentuser.userid, x.id, fromhost, board, file);
				sprintf(log,"%s/upload.log",BBSHOME);
				f_append(log, buf);
			}
			recount_size(board);
			sprintf(filename, "bbsfdoc?board=%s&start=%d", board,start);
			redirect(filename);
			//printf("<script>history.go(-1);</script>");
			http_quit();
		}
		num++;
	}
	fclose(fpdir);
	http_fatal("错误的文件名");
}

