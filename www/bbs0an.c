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
	strlcpy(path, getparm("path"), sizeof(path));
	if (strstr(path, "..") || strstr(path, "SYSHome"))
		http_fatal("此目录不存在");
	char *board = getbfroma(path);
	struct boardheader *bp = NULL;
	if (*board != '\0') {
		bp = getbcache(board);
		if (!hasreadperm(&currentuser, bp))
			http_fatal("目录不存在或无权访问");
	}
	char names[512];
	snprintf(names, sizeof(names), "0Announce%s/.Names", path);
	FILE *fp = fopen(names, "r");
	if (fp == NULL)
		http_fatal("目录不存在或无权访问"); // not indicating hidden directories.
	char buf[512], *title;
	// check directory permission.
	while (true) {
		if (fgets(buf, sizeof(buf), fp) == NULL)
			http_fatal("精华区索引错误");
		if(!strncmp(buf, "# Title=", 8)) {
			title = buf + 8;
			if (!hasannperm(title, &currentuser, bp))
				http_fatal("目录不存在或无权访问");
			break;
		}
	}

	xml_header("bbs0an");
	printf("<bbs0an><path>%s</path><visit>%d</visit>", path, get_count(path));
	if (bp != NULL)
		printf("<board>%s</board>", bp->filename);
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
				puts("<entry><title>");
				xml_fputs(name, stdout);
				printf("</title><path>%s</path>", trim(buf + 6));
				snprintf(fpath, sizeof(fpath), "0Announce%s%s", path, buf + 6);
				if (stat(fpath, &st) != 0 || (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))) {
					puts("<type>err</type>");
				} else if (S_ISREG(st.st_mode)) {
					puts("<type>file</type>");
				} else {
					puts("<type>dir</type>");
				}
				if (id != NULL) {
					puts("<id>");
					xml_fputs(id, stdout);
					puts("</id>");
				}
				printf("<time>%s</time></entry>", getdatestring(st.st_mtime, DATE_XML));
			}
		}
	}
	fclose(fp);
	puts("</bbs0an>");
	return 0;
}
