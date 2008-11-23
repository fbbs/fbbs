#include <string.h>
#include "bbs.h"

char tname[STRLEN];
char fname[STRLEN];
extern char currboard[STRLEN];
extern char someoneID[31];

//	信息查看,
//		type = 	0时,看M文
//				1时,看原作
//				2时,同作者
//				3时,同作者,模糊查找
//				4时,标题关键字
int marked_all(int type) {
	struct fileheader post;
	register num=0;
	int fd;
	char dname[STRLEN], buf[STRLEN];
	char tempname1[50], tempname2[50];
	struct stat st1, st2;

	sprintf(dname, "boards/%s/%s", currboard, DOT_DIR);
	sprintf(fname, "boards/%s/%s2", currboard, DOT_DIR);
	switch (type) {
		case 0: // 生成M文索引文件
			sprintf(tname, "boards/%s/%s", currboard, MARKED_DIR);
			break;
		case 1: // 生成原作索引文件
			sprintf(tname, "boards/%s/%s", currboard, AUTHOR_DIR);
			break;
		case 2:
		case 3: // 同作者索引文件
			sprintf(tname, "boards/%s/SOMEONE.%s.DIR.%d", currboard,
					someoneID, type-2);
			break;
		case 4: // 标题关键字索引
			sprintf(tname, "boards/%s/KEY.%s.DIR", currboard,
					currentuser.userid);
			break;
		default:
			break;
	}

	if (stat(dname, &st1) == -1) {
		return 1;
	}
	if (stat(tname, &st2) != -1) {
		if (st2.st_mtime >= st1.st_mtime) {
			return 1;
		}
	}
	unlink(tname);
	sprintf(buf, "cp %s %s", dname, fname); //使用外部命令
	system(buf);

	if ((fd = open(fname, O_RDONLY, 0)) == -1) {
		return -1;
	}

	while (read(fd, &post, sizeof(struct fileheader))
			==sizeof(struct fileheader)) {
		num++;
		switch (type) {
			case 0:
				if (post.accessed[ 0 ] & FILE_MARKED) {
					append_record(tname, &post, sizeof(post));
				}
				break;
			case 1:
				if (strncmp(post.title, "Re:", 3)&& strncmp(post.title,
						"RE:", 3)) {
					append_record(tname, &post, sizeof(post));
				}
				break;
			case 2:
			case 4:
				if (type ==2) {
					strtolower(tempname1, post.owner);
				} else {
					strtolower(tempname1, post.title);
				}
				strtolower(tempname2, someoneID);

				if (strcasestr(tempname1, tempname2) ) {
					append_record(tname, &post, sizeof(post));
				}
				break;
			case 3:
				if ( !strcasecmp(post.owner, someoneID)) {
					append_record(tname, &post, sizeof(post));
				}
				break;
		}
	}
	close(fd);
	unlink(fname);
	return 0;
}
