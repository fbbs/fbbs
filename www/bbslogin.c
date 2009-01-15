#include "libweb.h"

int main() {
	char fname[STRLEN];//added by roly 02.05.10 
	int pid, n, t;
	char buf[256], id[20], pw[20];
	struct userec *x;
	FILE *fp;
	init_all();
	strlcpy(id, getparm("id"), 13);
        strlcpy(pw, getparm("pw"), 13);
	if(loginok && strcasecmp(id, currentuser.userid)) {
		http_fatal("系统检测到目前您的计算机上已经登录有一个帐号 %s，请先退出.(%s)", 
			currentuser.userid, "选择正常logout, 或者关闭所有浏览器窗口");
	}
	x=getuser(id);
	if(x==0) http_fatal("错误的使用者帐号");
	if(strcasecmp(id, "guest")) {
		int total;
		time_t stay;
		time_t recent;
		time_t now;
		if(!checkpasswd(x->passwd, pw)) {
			if(pw[0]!=0) sleep(2);
		/* added by roly 02.05.10 to add bad login in telnet */
			sprintf(buf, "%-12.12s %s @%s\n", id, cn_Ctime(time(0)), fromhost);
			sethomefile(fname,id,"logins.bad"); 
            		f_append(fname, buf); 
        	/* added end */                
			f_append("logins.bad", buf);
			http_fatal("密码错误");
		}
		total=check_multi(x);
		if(!user_perm(x, PERM_LOGIN))
			http_fatal("此帐号已被停机, 若有疑问, 请用其他帐号在sysop版询问.");
		if(file_has_word(".bansite", fromhost)) {
			http_fatal("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.", fromhost);
		}
		now=time(0);
		if(total>1)
		{
			recent=x->lastlogout;
			if(x->lastlogin>recent)recent=x->lastlogin;
			stay=now-recent;
			if(stay<0)stay=0;
		}
		else stay=0;
		t=x->lastlogin;
		x->lastlogin=now;
		x->stay+=stay;
		save_user_data(x);
		//add for NR autopost id:US.   eefree 06.9.8 
		if (strcasecmp(id,"US") ) {
			if(abs(t-time(0))<60) http_fatal("两次登录间隔过密!");
		}//add end
		x->numlogins++;
		strlcpy(x->lasthost, fromhost, 16);
		save_user_data(x);
		currentuser=*x;
	}
	sprintf(buf, "ENTER %s", fromhost);
	do_report("usies", buf);
	n=0;
	if(!loginok && strcasecmp(id, "guest"))	wwwlogin(x);
	redirect(FIRST_PAGE);
}

int wwwlogin(struct userec *user) {
	FILE *fp;
	char buf[80];
	int pid, n, tmp;
	struct user_info *u;
/* patch added by roly */
	if(!(currentuser.userlevel & PERM_REGISTER)) { 
                char file[256]; 
                sprintf(file, "home/%c/%s/register", 
                        toupper(currentuser.userid[0]), currentuser.userid); 
                if(file_exist(file)) { 
                        currentuser.userlevel |=PERM_DEFAULT; 
                        save_user_data(&currentuser); 
                } 
        }  
/* add end */
        fp=fopen("tmp/.UTMP.lock", "a");
	FLOCK(fileno(fp), LOCK_EX);
	for(n=0; n<MAXACTIVE; n++) {
		if(shm_utmp->uinfo[n].active == 0) {
			u=&(shm_utmp->uinfo[n]);
			u_info=u;
			pid=fork();
 			if(pid<0) http_fatal("can't fork");
 			if(pid==0) {
				wwwagent();
				exit(0);
			}
			bzero(u, sizeof(struct user_info));
			u->active=1;
			u->uid=getusernum(user->userid)+1;
			u->pid=pid;
			u->mode=10001;
/* added by roly 02.05.19 for dispaly friend char when look in telnet mode 
			getfriendstr(u);
 add end */
        		if(user_perm(&currentuser, PERM_LOGINCLOAK) &&
			(currentuser.flags[0] & CLOAK_FLAG))
                		u->invisible = YEA;
        		u->pager = 0;
        		if(currentuser.userdefine & DEF_FRIENDCALL)
				u->pager|=FRIEND_PAGER;
        		if(currentuser.flags[0] & PAGER_FLAG) {
                		u->pager|=ALL_PAGER;
                		u->pager|=FRIEND_PAGER;
			}
        		if(currentuser.userdefine & DEF_FRIENDMSG)
				u->pager|=FRIENDMSG_PAGER;
        		if(currentuser.userdefine & DEF_ALLMSG) {
                		u->pager|=ALLMSG_PAGER;
                		u->pager|=FRIENDMSG_PAGER;
        		}
				
			SpecialID(u->userid, fromhost);
			   
			strlcpy(u->from, fromhost, 24);
#ifdef SPARC
			*(int*)(u->from+30)=time(0);
#else
			*(int*)(u->from+32)=time(0);
#endif
            u->from[22] = DEFINE(DEF_NOTHIDEIP)?'S':'H';

			u->idle_time=time(0);
			//Modified by IAMFAT 2002.06.05
			//strsncpy(u->username, user->username, 20);
			strcpy(u->username, user->username);
			//strsncpy(u->userid, user->userid, 13);
			strcpy(u->userid, user->userid);
			tmp=rand()%100000000;
			u->utmpkey=tmp;
			sprintf(buf, "%d", n+1);
			setcookie("utmpnum", buf);
			sprintf(buf, "%d", tmp);
			setcookie("utmpkey", buf);
			setcookie("utmpuserid", currentuser.userid);
			set_my_cookie();
			FLOCK(fileno(fp), LOCK_UN);
			fclose(fp);
			shm_ucache->status[u->uid-1]++;
			return 0;
		}
	}
	FLOCK(fileno(fp), LOCK_UN);
	fclose(fp);
	http_fatal("抱歉，目前在线用户数已达上限，无法登录。请稍后再来。");
}

void add_msg() {
        int i;
        FILE *fp;
        char buf[129], file[256], *id=currentuser.userid;
        sprintf(file, "touch home/%c/%s/wwwmsg.flush", toupper(id[0]), id);
        system(file);
        sprintf(file, "home/%c/%s/msgfile", toupper(id[0]), id);
        i=file_size(file)/129;
        if(get_record(&buf, 129, i-1, file)<=0) return;
        sprintf(file, "home/%c/%s/wwwmsg", toupper(id[0]), id);
        fp=fopen(file, "a");
        fwrite(buf, 129, 1, fp);
        fclose(fp);
}

/* added by roly 02.05.17 for add friend list into uinfo //not completed
int
getfriendstr(struct user_info *uinfo)
{
	int     i;
	struct override *tmp;
	memset(uinfo->friend, 0, sizeof(uinfo->friend));
	setuserfile(genbuf, "friends");
	sprintf(genbuf,"%s/%s/%s/friends",BBSHOME,toupper(currentuser.userid[0]),currentuser.userid(0));
	uinfo->fnum = get_num_records(genbuf, sizeof(struct override));
	if (uinfo->fnum <= 0)
		return 0;
	uinfo->fnum = (uinfo->fnum >= MAXFRIENDS) ? MAXFRIENDS : uinfo->fnum;
	tmp = (struct override *) calloc(sizeof(struct override), uinfo->fnum);
	get_records(genbuf, tmp, sizeof(struct override), 1, uinfo->fnum);
	for (i = 0; i < uinfo->fnum; i++) {
		uinfo->friend[i] = searchuser(tmp[i].id);
	}
	free(tmp);
	//update_ulist(uinfo, utmpent);
}

int
searchuser(userid)
char   *userid;
{
	register int i;
	resolve_ucache();
	for (i = 0; i < uidshm->number; i++)
		if (!ci_strncmp(userid, uidshm->userid[i], IDLEN + 1))
			return i + 1;
	return 0;
}

 add end */

void abort_program() {
/* modified by roly   patch of NJU 0.9 */
        int stay=0; 
        struct userec *x; 
	int loginstart;
        if(!strcmp(u_info->userid, currentuser.userid)) { 
#ifdef SPARC
				loginstart=*(int*)(u_info->from+30); 
#else
				loginstart=*(int*)(u_info->from+32); 
#endif
				shm_ucache->status[u_info->uid-1]--;
                bzero(u_info, sizeof(struct user_info)); 
        } 
        //if(stay>7200) stay=7200; 
        x=getuser(currentuser.userid); 
        if(x) { 
		time_t now=time(0);
		time_t recent;
		recent=loginstart;
		if(x->lastlogout>recent)recent=x->lastlogout;
		if(x->lastlogin>recent)recent=x->lastlogin;
		stay=now-recent;
		//stay=now-loginstart;
		if(stay<0)stay=0;
                x->stay+=stay; 
                x->lastlogout=now; 
                save_user_data(x); 
        }
        exit(0); 
}

int wwwagent() {
	int i;
	for(i=0; i<1024; i++) close(i);
	for(i=0; i<NSIG; i++) signal(i, SIG_IGN);
	signal(SIGUSR2, add_msg);
	signal(SIGHUP, abort_program);
	signal(SIGABRT, abort_program);
	while(1) {
		sleep(60);
		if(abs(time(0) - u_info->idle_time)>600) {
			//f_append("err", "idle timeout");
			abort_program();
		}
	}
	exit(0);
}

int check_multi(struct userec *user) {
	int i, total=0;
	//if(currentuser.userlevel & PERM_SYSOP) return;
	for(i=0; i<MAXACTIVE; i++) {
		if(shm_utmp->uinfo[i].active==0) continue;
		if(!strcasecmp(shm_utmp->uinfo[i].userid, user->userid)) total++;
	}
	//add for NR autopost id:US.   eefree 06.9.8 
	if (strcasecmp(user->userid,"US") ) {
		if(!(user->userlevel&PERM_SYSOPS) && total>=2) http_fatal("您已经登录了2个窗口。为了保证他人利益，此次连线将被取消。");
	}//add end
	return total;
}
