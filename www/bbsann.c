#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

enum {
	ANN_TITLE_LENGTH = 39
};

static bool hasannperm(const char *title, const struct userec *user,
		const board_t *board)
{
	if ((strstr(title, "BM: SYSOPS") && !HAS_PERM2(PERM_SYSOPS, user))
			|| (strstr(title, "BM: OBOARDS") && !HAS_PERM2(PERM_OBOARDS, user))
			|| (strstr(title, "BM: BMS") && !is_bm(user, board)))
		return false;
	return true;		
}

static int get_count(const char *path)
{
	FILE *fp;
	char buf[512];
	int counts = 0;
	snprintf(buf, sizeof(buf), "0Announce%s/.counts", path);
	if (!dashf(buf))
		fp = fopen(buf, "w+");
	else
		fp = fopen(buf, "r+");
	file_lock_all(fileno(fp), FILE_WRLCK);
	if (fscanf(FCGI_ToFILE(fp), "%d", &counts) > 0) {
		counts++;
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "%d\n", counts);
	}
	file_lock_all(fileno(fp), FILE_UNLCK);
	fclose(fp);
	return counts;
}

// TODO: better rewrite
static char *getbfroma(const char *path)
{
	FILE *fp;
	static char buf1[256], buf2[256];
	memset(buf1, '\0', sizeof(buf1));
	memset(buf2, '\0', sizeof(buf2));
	if (path == NULL || *path == '\0')
		return "";
	++path;
	fp = fopen("0Announce/.Search", "r");
	if (fp == NULL)
		return "";
	while (true) {
		if(fscanf(FCGI_ToFILE(fp), "%s %s", buf1, buf2) <= 0)
			break;
		if (*buf1 != '\0')
			buf1[strlen(buf1) - 1] = '\0';
		if (*buf1 == '*')
			continue;
		size_t len = strlen(buf2);
		if(!strncmp(buf2, path, len)
				&& (path[len] == '/' || path[len] == '\0')) {
			fclose(fp);
			return buf1;
		}
	}
	fclose(fp);
	return "";
}

int bbs0an_main(void)
{
	char path[512];
	board_t board;
	int bid = strtol(get_param("bid"), NULL, 10);
	if (bid <= 0) {
		strlcpy(path, get_param("path"), sizeof(path));
		if (strstr(path, "..") || strstr(path, "SYSHome"))
			return BBS_EINVAL;
		char *bname = getbfroma(path);
		if (*bname != '\0') {
			if (!get_board(bname, &board) || !has_read_perm(&board))
				return BBS_ENODIR;
		}
	} else {
		if (!get_board_by_bid(bid, &board) || !has_read_perm(&board))
			return BBS_ENOBRD;
		if (board.flag & BOARD_DIR_FLAG)
			return BBS_EINVAL;

		path[0] = '\0';
		FILE *fp = fopen("0Announce/.Search", "r");
		if (fp == NULL)
			return BBS_EINTNL;
		char tmp[256];
		int len = strlen(board.name);
		while (fgets(tmp, sizeof(tmp), fp) != NULL) {
			if (!strncmp(tmp, board.name, len) && tmp[len] == ':'
					&& tmp[len + 1] == ' ') {
				tmp[len + 1] = '/';
				strlcpy(path, tmp + len + 1, sizeof(path));
				path[strlen(path) - 1] = '\0';
				break;
			}
		}
		fclose(fp);
		if (path[0] == '\0')
			return BBS_ENODIR;
	}
	char names[512];
	snprintf(names, sizeof(names), "0Announce%s/.Names", path);
	FILE *fp = fopen(names, "r");
	if (fp == NULL)
		return BBS_ENODIR; // not indicating hidden directories.
	char buf[512], *title;
	// check directory permission.
	while (true) {
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			fclose(fp);
			return BBS_ENODIR;
		}
		if(!strncmp(buf, "# Title=", 8)) {
			title = buf + 8;
			if (!hasannperm(title, &currentuser, &board)) {
				fclose(fp);
				return BBS_ENODIR;
			}
			break;
		}
	}

	xml_header(NULL);
	printf("<bbs0an path='%s' v='%d' ", path, get_count(path));
	if (board.id)
		printf(" brd='%s'", board.name);
	printf(">");
	print_session();
	
	char name[STRLEN], fpath[1024], *id = NULL, *ptr;
	struct stat st;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (!strncmp(buf, "Name=", 5)) {
			strlcpy(name, trim(buf + 5), sizeof(name));
			if (strlen(name) > ANN_TITLE_LENGTH) {
				id = name + ANN_TITLE_LENGTH;
				if (!hasannperm(name + ANN_TITLE_LENGTH, &currentuser, &board))
					continue;
				name[ANN_TITLE_LENGTH -  1] = '\0';
				if (!strncmp(id, "BM: ", 4))
					id += 4;
				if ((ptr = strchr(id, ')')) != NULL)
					*ptr = '\0';
			} else {
				id = NULL;
			}
			if (fgets(buf, sizeof(buf), fp) == NULL || strncmp(buf, "Path=~", 6)) {
				break;
			} else {
				printf("<ent path='%s' t='", trim(buf + 6));
				snprintf(fpath, sizeof(fpath), "0Announce%s%s", path, buf + 6);
				if (stat(fpath, &st) != 0 || (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))) {
					printf("e'");
				} else if (S_ISREG(st.st_mode)) {
					printf("f'");
				} else {
					printf("d'");
				}
				if (id != NULL)
					printf(" id='%s'", id);
				printf(" time='%s'>", format_time(st.st_mtime, TIME_FORMAT_XML));
				xml_fputs(trim(name), stdout);
				printf("</ent>");
			}
		}
	}
	fclose(fp);
	puts("</bbs0an>");
	return 0;
}

int bbsanc_main(void)
{
	const char *path = get_param("path");
	if (strstr(path, "bbslist") || strstr(path, ".Search")
			|| strstr(path, ".Names") || strstr(path, "..")
			|| strstr(path, "SYSHome"))
		return BBS_EINVAL;

	char *bname = getbfroma(path);	
	board_t board;
	if (*bname) {
		if (!get_board(bname, &board) || !has_read_perm(&board))
			return BBS_ENOFILE;
	}

	char fname[512];
	sprintf(fname, "0Announce%s", path);
	xml_header(NULL);
	printf("<bbsanc ");
	if (board.id)
		printf(" brd='%s'", board.name);
	printf(">");
	print_session();
	printf("<po>");
	xml_printfile(fname);
	printf("</po></bbsanc>");
	return 0;
}
