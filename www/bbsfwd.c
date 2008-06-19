#include "BBSLIB.inc"

int main() {
	struct fileheader *x;
	char board[80], file[80], target[80], filename[80];
	struct userec *user;
		
	init_all();
	strsncpy(board, getparm("board"), 30);
	strsncpy(file, getparm("file"), 32);
	strsncpy(target, getparm("target"), 30);
	if(!loginok) 
	{
		printf("<b>转寄/推荐给好友 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("匆匆过客不能进行本项操作");
	}
	/* Added by Amigo 2002.06.19. For mail right check. */
	if (!HAS_PERM(PERM_MAIL)) 
	{
		printf("<b>转寄/推荐给好友 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("您尚未完成注册，或者发送信件的权限被封禁");
	}
	/* Add end. */
	/* added by roly for mail check */
	if (!mailnum_under_limit(currentuser.userid) || !mailsize_under_limit(currentuser.userid)) 
	{
		printf("<b>转寄/推荐给好友 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("您的信件容量超标，无法发信");
	}
	/* add end */
	if(!has_read_perm(&currentuser, board)) 
	{
		printf("<b>转寄/推荐给好友 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	x=get_file_ent(board, file);
	if(x==0) 
	{
		printf("<b>转寄/推荐给好友 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的文件名");
	}
	printf("<b>转寄/推荐给好友 · %s [使用者: %s]</b>\n", BBSNAME, currentuser.userid);
	printpretable_lite();
	if(target[0]) {
		if(!strstr(target, "@")) {
			if(!getuser(target)) http_fatal("错误的使用者帐号");
			//add by Danielfree 06.2.5
			if (!( (getuser(target) )-> userlevel & PERM_READMAIL))
				  http_fatal("对方无法收信");
			strcpy(target, getuser(target)->userid);
			if (!mailsize_under_limit(target)|| !mailnum_under_limit(target))
				http_fatal("收信人信件容量超标，无法收信");
			//add end
		}
		user=getuser(target);
		
		sprintf(filename, "home/%c/%s/rejects", toupper(target[0]), user->userid);
		if(file_has_word(filename, currentuser.userid))
		    http_fatal("对方不想收到您的信件");
			
		return do_fwd(x, board, target);
	}
	printf("<table><tr><td>\n");
	printf("文章标题: %s<br>\n", nohtml(x->title));
	printf("文章作者: %s<br>\n", x->owner);
	printf("原讨论区: %s<br>\n", board);
	printf("<form action=bbsfwd method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	/*
	printf("把文章转寄给 <input name=target size=30 maxlength=30 value=%s> (请输入对方的id或email地址). <br>\n",
		currentuser.email);
	*/
	// modified by roly to deny internet mail
	printf("把文章转寄给 <input name=target size=12 maxlength=12 value=%s> (请输入对方的id). <br>\n",
		currentuser.userid);
	//modified end
	printf("<input type=submit value=确定转寄></form>");
}

int do_fwd(struct fileheader *x, char *board, char *target) {
	FILE *fp, *fp2;
	char title[512], buf[512], path[200], i;
	sprintf(path, "boards/%s/%s", board, x->filename);
	if(!file_exist(path)) http_fatal("文件内容已丢失, 无法转寄");
	sprintf(title, "[转寄] %s", x->title);
	title[60]=0;
	post_mail(target, title, path, currentuser.userid, currentuser.username, fromhost, -1);
	printf("文章已转寄给'%s'<br>\n", nohtml(target));
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
}
