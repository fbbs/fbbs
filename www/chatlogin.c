#include "libweb.h"

int main() {
	int pid, n, t;
	char buf[256], id[20], pw[20];
	struct userec *x;
	FILE *fp;
	init_all();
	strsncpy(id, getparm("id"), 13);
        strsncpy(pw, getparm("pw"), 13);
	if(loginok && strcasecmp(id, currentuser.userid)) {
		http_fatal("系统检测到目前您的计算机上已经登录有一个帐号 %s，请先退出.(%s)", 
			currentuser.userid, "选择正常logout, 或者关闭所有浏览器窗口");
	}
	x=getuser(id);
	if(x==0) http_fatal("错误的使用者帐号");
	if(strcasecmp(id, "guest")) {
		if(!checkpasswd(x->passwd, pw)) {
			if(pw[0]!=0) sleep(2);
                        sethomefile(buf,id,"logins.bad");
                        do_report(buf, fromhost);
 			do_report("logins.bad", fromhost);
			http_fatal("密码错误");
		}
		if(!user_perm(x, PERM_LOGIN))
			http_fatal("此帐号已被停机, 若有疑问, 请用其他帐号在sysop版询问.");
		if(file_has_word(".bansite", fromhost)) {
			http_fatal("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.", fromhost);
		}
		t=x->lastlogin;
		x->lastlogin=time(0);
		save_user_data(x);
		if(abs(t-time(0))<5) http_fatal("两次登录间隔过密!");
		x->numlogins++;
		strsncpy(x->lasthost, fromhost, 16);
		save_user_data(x);
		currentuser=*x;
	}
	sprintf(buf, "ENTER CHAT %s", fromhost);
	do_report("usies", buf);
	n=0;
	if(!loginok && strcasecmp(id, "guest"))	wwwlogin(x);
	redirect("bbschat");
}

int wwwlogin(struct userec *user) {
	FILE *fp;
	char buf[80];
	int pid, n, tmp;
	struct user_info *u;
	fp=fopen(".UTMP.lock", "r");
	flock(fileno(fp), LOCK_EX);
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
			strsncpy(u->from, fromhost, 24);
#ifdef SPARC
			*(int*)(u->from+30)=time(0);
#else
			*(int*)(u->from+32)=time(0);
#endif
			u->idle_time=time(0);
			strsncpy(u->username, user->username, 20);
			strsncpy(u->userid, user->userid, 13);
			tmp=rand()%100000000;
			u->utmpkey=tmp;
			sprintf(buf, "%d", n+1);
			setcookie("utmpnum", buf);
			sprintf(buf, "%d", tmp);
			setcookie("utmpkey", buf);
			setcookie("utmpuserid", currentuser.userid);
			set_my_cookie();
			flock(fileno(fp), LOCK_UN);
			fclose(fp);
			return 0;
		}
	}
	flock(fileno(fp), LOCK_UN);
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

void abort_program() {
	int stay;
	struct userec *x;
	f_append("err", "ok");
	if(strcmp(u_info->userid, currentuser.userid))
		bzero(u_info, sizeof(struct user_info));
#ifdef SPARC
	stay=abs(time(0) - *(int*)(u_info->from+30));
#else
	stay=abs(time(0) - *(int*)(u_info->from+32));
#endif
        if(stay>7200) stay=7200;
	x=getuser(currentuser.userid);
 	if(x) {
		x->stay+=stay;
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
	while(1) {
		sleep(60);
		if(abs(time(0) - u_info->idle_time)>600) {
			f_append("err", "idle timeout");
			abort_program();
		}
	}
	exit(0);
}
