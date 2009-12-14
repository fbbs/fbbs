#include "libweb.h"

enum {
	ANN_TITLE_LENGTH = 39
};

static bool hasannperm(const char *title, const struct userec *user,
		const struct boardheader *bp)
{
	if ((strstr(title, "BM: SYSOPS") && !HAS_PERM2(PERM_SYSOPS, user))
			|| (strstr(title, "BM: OBOARDS") && !HAS_PERM2(PERM_OBOARDS, user))
			|| (strstr(title, "BM: BMS") && !chkBM(bp, user)))
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
	FLOCK(fileno(fp), LOCK_EX);
	fscanf(FCGI_ToFILE(fp), "%d", &counts);
	counts++;
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "%d\n", counts);
	FLOCK(fileno(fp), LOCK_UN);
	fclose(fp);
	return counts;
}

int bbs0an_main(void)
{
	char path[512];
	struct boardheader *bp = NULL;
	int bid = strtol(getparm("bid"), NULL, 10);
	if (bid <= 0) {
		strlcpy(path, getparm("path"), sizeof(path));
		if (strstr(path, "..") || strstr(path, "SYSHome"))
			return BBS_EINVAL;
		char *board = getbfroma(path);
		if (*board != '\0') {
			bp = getbcache(board);
			if (!hasreadperm(&currentuser, bp))
				return BBS_ENODIR;
		}
	} else {
		bp = getbcache2(bid);
		if (bp == NULL || !hasreadperm(&currentuser, bp))
			return BBS_ENOBRD;
		if (bp->flag & BOARD_DIR_FLAG)
			return BBS_EINVAL;
		path[0] = '\0';
		FILE *fp = fopen("0Announce/.Search", "r");
		if (fp == NULL)
			return BBS_EINTNL;
		char tmp[256];
		int len = strlen(bp->filename);
		while (fgets(tmp, sizeof(tmp), fp) != NULL) {
			if (!strncmp(tmp, bp->filename, len) && tmp[len] == ':'
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
		if (fgets(buf, sizeof(buf), fp) == NULL)
			return BBS_ENODIR;
		if(!strncmp(buf, "# Title=", 8)) {
			title = buf + 8;
			if (!hasannperm(title, &currentuser, bp))
				return BBS_ENODIR;
			break;
		}
	}

	xml_header("bbs0an");
	printf("<bbs0an path='%s' v='%d' ", path, get_count(path));
	print_session();
	if (bp != NULL)
		printf(" brd='%s'", bp->filename);
	printf(">");
	
	char name[STRLEN], fpath[1024], *id = NULL, *ptr;
	struct stat st;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (!strncmp(buf, "Name=", 5)) {
			strlcpy(name, trim(buf + 5), sizeof(name));
			if (strlen(name) > ANN_TITLE_LENGTH) {
				id = name + ANN_TITLE_LENGTH;
				if (!hasannperm(name + ANN_TITLE_LENGTH, &currentuser, bp))
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
				printf(" time='%s'>", getdatestring(st.st_mtime, DATE_XML));
				xml_fputs(trim(name), stdout);
				printf("</ent>");
			}
		}
	}
	fclose(fp);
	puts("</bbs0an>");
	return 0;
}
