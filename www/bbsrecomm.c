
void showrecomm(char *board) {
	FILE *fp;
	int i, index=0, total=0;
	char *ptr, path[512], names[512], name[1024][80], file[1024][80], buf[512], title[256]=" ";
	char *board;
	init_all();
	
	strsncpy(path,anno_path_of(board),511);
	if(strstr(path, "..") || strstr(path, "SYSHome")) http_fatal("此目录不存在");
	sprintf(names, "0Announce%s/.recommend/.Names", path);
	fp=fopen(names, "r");
	board=getbfroma(path);
	if(board[0] && !has_read_perm(&currentuser, board))return;
	if(fp==0) return;
	while(1) {
		if(fgets(buf, 511, fp)==0) break;
		if(!strncmp(buf, "# Title=", 8)) strcpy(title, buf+8);
		if(!strncmp(buf, "Name=", 5) && total<1023) {
			strcpy(name[total], trim(buf+5));
			strcpy(file[total], "");
			total++;
		}
		if(!strncmp(buf, "Path=~", 6) && total>0) {
			sprintf(file[total-1], "%s", trim(buf+6));
		}
	}
	if(strstr(title, "BM: SYSOPS") && !(currentuser.userlevel & PERM_SYSOP)) http_fatal("错误的目录");
	if(strstr(title, "BM: OBOARDS") && !(currentuser.userlevel & PERM_OBOARDS)) http_fatal("错误的目录");
	if(strstr(title, "BM: BMS") && !has_BM_perm(&currentuser, board)) http_fatal("错误的目录");
	
	buf[0]=0;
	if(total<=0)return;
	printpretable();	
	for(i=0; i<total; i++) {
		char *id;
		/* if(strstr(name[i], "SYSOPS")) continue; */
		//以上代码由roly注释掉，因为站长可以访问该目录  2002.01.03
		
		index++;
		if(strlen(name[i])<=39) {
			id="";
		} else {
			name[i][38]=0;
			id=name[i]+39;
			if(!strncmp(id, "BM: ", 4)) id+=4;
			ptr=strchr(id, ')');
			if(ptr) ptr[0]=0;
		}
		
		/*  add by roly 2002.01.03
		 *  根据目录的作者以及用户权限判断是否显示 
		 */		
		if (!strncmp(id,"SYSOPS",6) && !(currentuser.userlevel & PERM_SYSOP)) continue;
		//modified by iamfat 2002.10.18 保证BMS目录本版版主可见
		if (!strncmp(id,"BMS",3) && !has_BM_perm(&currentuser, board))continue;
		//if(!strncmp(id,"BMS",3) && !(currentuser.userlevel & PERM_BOARDS)) continue;
		if (!strncmp(id,"OBOARDS",7) && !(currentuser.userlevel & PERM_OBOARDS)) continue;
		/*  add end     */ 
		
		printf("<tr class=%s ><td>%d", ((cc++)%2)?"pt9dc":"pt9lc", index);
		sprintf(buf, "0Announce%s%s", path, file[i]);
		if(!file_exist(buf))  continue;
		} else if(file_isdir(buf)) {
			printf("[目录] <a href=bbs0an?path=%s%s>%s</a><br>", path, file[i], nohtml(name[i]));
		} else {
			printf("[文件] <td><a href=bbsanc?path=%s%s>%s</a>", path, file[i], nohtml(name[i]));
		}
	}
	printposttable();
	printf("</center>\n");
	http_quit();
}

