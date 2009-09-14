#include "libweb.h"

// TODO: rewrite
static bool quota_exceeded(const char *board)
{
	int all = 0, now = 0;
	char path[256], cmd[256];
	sprintf(path, BBSHOME"/upload/%s", board);
	if (dashd(path)) {
		FILE *fp;
		sprintf(cmd , "du %s|cut -f1>%s/.size", path, path);
		system(cmd);
		sprintf(cmd, "%s/.size", path);
		if ((fp = fopen(cmd, "r")) == NULL)
			return true;
		fscanf(FCGI_ToFILE(fp), "%d", &now);
		fclose(fp);
		sprintf(cmd, "%s/.quota", path);
		if((fp = fopen(cmd, "r")) != NULL) {
			fscanf(FCGI_ToFILE(fp), "%d", &all);
			if(now >= all)
				return true;
			fclose(fp);
		}
		return false;
	}
	return true;
}

static int addtodir(const char *board, const char *tmpfile) 
{
	char file[100], dir[100], url_filename[256];
	static struct fileheader x;
	
	x.reid = 1;
	strlcpy(x.owner, currentuser.userid, sizeof(x.owner));
	strlcpy(x.filename, tmpfile, sizeof(x.filename));
	strtourl(url_filename, x.filename);
	x.timeDeleted = time(NULL);
	snprintf(file, sizeof(file), BBSHOME"/upload/%s/%s", board, x.filename);
	snprintf(dir, sizeof(dir), BBSHOME"/upload/%s/.DIR", board);
	// TODO: ...
	struct stat st;
	if (stat(file, &st) < 0)
		x.id = 0;
	else
		x.id = st.st_size;
	if (append_record(dir, &x, sizeof(x)) < 0) {
		unlink(file);
		return BBS_EINTNL;
	}

	char buf[256],log[100];
	snprintf(buf, sizeof(buf), "UP [%s] %s %dB %s %s FILE:%s\n",
			getdatestring(time(NULL), DATE_EN), currentuser.userid, x.id, 
			fromhost, board, x.filename);
	snprintf(log, sizeof(log), "%s/upload.log", BBSHOME);
	file_append(log, buf);

	xml_header("bbsupload");
	printf("<bbsupload><size>%d</size><user>%s</user>"
			"<url>http://"BBSHOST"/upload/%s/%s</url></bbsupload>",
			x.id, x.owner, board, x.filename);
	return 0;
}

static bool check_upload(char *buf, size_t size, char **begin, char **end, char **fileext)
{
	if (fread(buf, 1, size, stdin) < size)
		return false;
	char *bufend = buf + size;
	// find boundary
	char *p = memchr(buf, '\r', size);
	if (p == NULL)
		return false;
	*p = '\0';
	const char *boundary = buf;
	// line containing 'filename="xxxx"'
	char *fname = p + 2;  // "\r\n"
	if (fname > bufend)
		return false;
	p = memchr(fname, '\r', bufend - fname);
	if (p == NULL)
		return false;
	*p = '\0';

	// parse filename
	fname = strstr(fname, "filename=\"");
	if (fname == NULL)
		return false;
	p = strrchr(fname, '\\'); // windows path symbol
	if (p != NULL) {
		fname = p + 1;
	} else {
		// moz-browsers don't give out full path, just the very filename,
		// so no parsing needed
		fname += strlen("filename=\"");
	}
	if ((p = strchr(fname, '\"')) != NULL);
		*p = '\0';
	// Check filename extension
	// Only .jpg/.jpeg/.gif/.png/.pdf are allowed.
	p = strrchr(fname, '.');
	if (p == NULL || (strcasecmp(p, ".JPEG") && strcasecmp(p, ".JPG")
			&& strcasecmp(p, ".GIF") && strcasecmp(p, ".PNG")
			&& strcasecmp(p, ".PDF"))) {
		return false;
	}
	*fileext = p;
	// skip 2 lines
	p = memchr(p, '\r', bufend - p);
	if (p == NULL || ++p == bufend)
		return false;
	p = memchr(p, '\r', bufend - p);
	if (p == NULL || p + 2 >= bufend)
		return false;
	*begin = p + 2;
	// [content]\r\n[boundary]\r\n
	*end = bufend - 4 - strlen(boundary);
	if (*end < *begin + 1)
		return false;
	return true;
}

int bbsupload_main(void)
{
	if (!loginok) 
		return BBS_ELGNREQ;
	const char *board = getparm("b");
	struct boardheader *bp = getbcache(board);
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_ENOBRD;

	size_t size = strtoul(getsenv("CONTENT_LENGTH"), NULL, 10);
	if (size > UPLOAD_MAX + UPLOAD_OVERHEAD)
		return BBS_EINVAL;

	char *buf = malloc(size);
	if (buf == NULL) 
		return BBS_EINTNL;
	char *begin = NULL, *end = NULL, *fileext = NULL;
	if (!check_upload(buf, size, &begin, &end, &fileext)) {
		free(buf);
		return BBS_EINVAL;
	}
	if (end - begin > maxlen(board)) {
		free(buf);
		return BBS_EFBIG;
	}
	if (quota_exceeded(board)) {
		free(buf);
		return BBS_EATTQE;
	}
	// TODO: Possible collison..
	char fname[HOMELEN], fpath[HOMELEN];
	sprintf(fname, "%ld-%04d%s", time(NULL), 
			(int)(10000.0 * rand() / RAND_MAX), fileext);
	snprintf(fpath, sizeof(fpath), BBSHOME"/upload/%s/%s", board, fname);
	FILE *fp = fopen(fpath, "w");
	if (fp != NULL) {
		fwrite(begin, 1, end - begin, fp);
		fclose(fp);
	} else {
		return BBS_EINTNL;
	}
	free(buf);
	return addtodir(board, fname);
}
