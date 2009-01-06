#include "bbs.h"

extern int flush_ucache();
extern int resolve_ucache();

extern int load_ucache();
extern void resolve_boards();
//退出时执行的函数
void do_exit() {
	flush_ucache();
	flush_bcache();
}

int main(int argc, char *argv[]) {
	chdir(BBSHOME); //进入BBS用户主目录
	setuid(BBSUID); //将进程的 用户ID
	setgid(BBSGID); //组ID设置成BBS
	setreuid(BBSUID, BBSUID); //设置有效用户ID	
	setregid(BBSGID, BBSGID); //有效组ID为BBS

	if (argc <= 1) {
		printf("usage: daemon | flushed | reload\n");
		exit(0);
	}
	if ( !strcasecmp(argv[1], "daemon") ) { // miscd daemon
		switch (fork()) { //后台程序:需要创建一个子进程,由子进程杀死父进程
			case -1: //
				printf("cannot fork\n");
				exit(0);
				break;
			case 0: // 子进程
				break;
			default:
				exit(0); //父进程
				break;
		}

		if (load_ucache(0) != 0) { //将用户的数据映射到内存
			printf("load ucache error\n");
			exit(-1);
		}

		resolve_boards();
		atexit(do_exit); //注册退出前运行的函数.正常退出前须执行此函数

		while (1) { //循环
			refresh_utmp(); //刷新用户临时数据
			b_closepolls(); //关闭投票
			flush_ucache(); //将用户在内存中的数据写回.PASSWDS
			flush_bcache();
			sleep(60 * 15); //睡眠十分钟,即每十五分钟同步一次.        
		}
	} else if ( !strcasecmp(argv[1], "flushed") ) { //miscd flushed
		resolve_ucache();
		flush_ucache();
		flush_bcache();
	} else if ( !strcasecmp(argv[1], "reload") ) { //miscd reload
		if (load_ucache(1) != 0) {
			printf("load ucache error\n");
			exit(-1);
		}
	} else {
		printf("usage: daemon | flushed | reload\n");
		exit(0);
	}

	return 0;
}
