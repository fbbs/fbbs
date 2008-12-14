#include "libweb.h"

int main() {
	int stay, pid;
	//added by iamfat 2002.10.05
	char buf[255];
	init_all();
	if(!loginok) { 
		redirect(FIRST_PAGE);  //added by roly
		return 0;
		http_fatal("ÄúÃ»ÓÐµÇÂ¼");		
	}
	//added by iamfat 2002.10.05 for TRACE
	sprintf(buf, "EXIT @%s", fromhost);
	do_report("usies", buf);
	//added end
	pid=u_info->pid;
	if(pid>0) kill(pid, SIGABRT);
//	if(pid>0) kill(pid, SIGHUP);
	setcookie("utmpkey", "");
	setcookie("utmpnum", "");
	setcookie("utmpuserid", "");
	setcookie("my_t_lines", "");
	setcookie("my_link_mode", "");
	setcookie("my_def_mode", "");
	setcookie("my_style","");
	redirect(FIRST_PAGE); 
}
