#include "bbs.h"
#include "mmap.h"
#include "record.h"

int getmailboxsize(unsigned int userlevel)
{
	if (userlevel & (PERM_SYSOPS))
		return MAILBOX_SIZE_SYSOP;
	if (userlevel & (PERM_LARGEMAIL))
		return MAILBOX_SIZE_LARGE;
	if (userlevel & (PERM_XEMPT))
		return MAILBOX_SIZE_BM;
	if (userlevel & (PERM_BOARDS))
		return MAILBOX_SIZE_BM;
	if (userlevel & (PERM_REGISTER))
		return MAILBOX_SIZE_NORMAL;
	return 15;
}

int getmailboxhold(unsigned int userlevel)
{
	if (userlevel & (PERM_SYSOPS))
		return MAX_SYSOPMAIL_HOLD;
	if (userlevel & (PERM_LARGEMAIL))
		return MAX_SYSOPMAIL_HOLD;
	if (userlevel & (PERM_XEMPT))
		return MAX_BMMAIL_HOLD;
	if (userlevel & (PERM_BOARDS))
		return MAX_BMMAIL_HOLD;
	if (userlevel & (PERM_REGISTER))
		return MAX_MAIL_HOLD;
	return MAX_MAIL_HOLD;
}

/**
 * Get apparent mailbox size.
 * The result is cached in tmp directory.
 * @param user The user.
 * @return Mailbox size in kilobytes.
 */
int getmailsize(const char *user)
{
	char index[HOMELEN], tmp[HOMELEN];
	setmdir(index, user);
	snprintf(tmp, sizeof(tmp), "tmp/%s.mailsize", user);

	int size = 0;
	struct stat st, st2;
	if (stat(index, &st) != 0 || st.st_size == 0)
		return 0;
	if (stat(tmp, &st2) == 0 && st2.st_size != 0
			&& st2.st_ctime >= st.st_ctime) {
		FILE *fp = fopen(tmp, "r");
		if (fp != NULL) {
			fscanf(fp, "%d", &size);
			fclose(fp);
			return size;
		}
	}

	size = st.st_size;
	struct fileheader fh;
	char file[HOMELEN];
	FILE *fp = fopen(index, "r");
	if (fp) {
		while (fread(&fh, sizeof(fh), 1, fp) == 1) {
			setmfile(file, currentuser.userid, fh.filename);
			if (stat(file, &st) == 0)
				size += st.st_size;
		}
		fclose(fp);
	}
	size /= 1024;

	fp = fopen(tmp, "w");
	if (fp) {
		fprintf(fp, "%d", size);
		fclose(fp);
	}

	return size;
}

int getmailnum(const char *userid)
{
	int mail_count;
	char buf[256];
	sprintf(buf, "mail/%c/%s/%s", toupper(userid[0]) , userid, DOT_DIR);
	mail_count=get_num_records(buf, sizeof(struct fileheader));
	return mail_count;
}

static int cmpfname(void *userid ,void *ov)
{
	const char *user = userid;
	const struct override *uv = ov;
	return !strcasecmp(user, uv->id);
}

int do_mail_file(const char *recv, const char *title, const char *header,
		const char *text, int len, const char *source)
{
	struct fileheader fh;
	struct override ov;
	struct stat st;
	char fname[HOMELEN], filepath[HOMELEN], *ip;
	int fd, count;
	int maxmail;
	char *user;

	if (!getuser(recv))
		return BBS_EINTNL;
	user = lookupuser.userid;
	sethomefile(filepath, user, "rejects");
	if (search_record(filepath, &ov, sizeof(ov), cmpfname,
			currentuser.userid))
		return BBS_EBLKLST;
	if (getmailboxsize(lookupuser.userlevel) * 2 
			< getmailsize(lookupuser.userid))
		return BBS_ERMQE;
	maxmail = getmailboxhold(lookupuser.userlevel);
	if (getmailnum(lookupuser.userid) > maxmail * 2)
		return BBS_ERMQE;

	memset(&fh, 0, sizeof(fh));
	strlcpy(fh.owner, currentuser.userid, sizeof(fh.owner));
	strlcpy(fh.title, title, sizeof(fh.title));

	sprintf(filepath, "mail/%c/%s", toupper(user[0]), user);
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0755) == -1)
			return BBS_EINTNL;
	} else {
		if (!S_ISDIR(st.st_mode))
			return BBS_EINTNL;
	}
	// TODO: get_fname?
	sprintf(fname, "M.%ld.A", time(NULL));
	sprintf(filepath, "mail/%c/%s/%s", toupper(user[0]), user, fname);
	ip = strrchr(fname, 'A');
	count = 0;
	while ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(filepath, "mail/%c/%s/%s", toupper(user[0]), user,
				fname);
		if (count++ > MAX_POSTRETRY) {
			return BBS_EINTNL;
		}
	}
	fb_flock(fd, LOCK_EX);
	strlcpy(fh.filename, fname, sizeof(fh.filename));
	sprintf(filepath, "mail/%c/%s/%s", toupper(user[0]), user, fname);
	if (header != NULL)
		write(fd, header, strlen(header));
	write(fd, text, len);
	if (source != NULL)
		write(fd, source, strlen(source));
	fb_flock(fd, LOCK_UN);
	close(fd);
	setmdir(fname, user);
	if (append_record(fname, &fh, sizeof(fh)) == -1)
		return BBS_EINTNL;

	char buf[256];
	snprintf(buf, sizeof(buf), "mailed %s: %s ", user, title);
	report(buf, currentuser.userid);
	return 0;
}

int mail_file(const char *file, const char *recv, const char *title)
{
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return BBS_EINTNL;
	int ret = do_mail_file(recv, title, NULL, m.ptr, m.size, NULL);
	mmap_close(&m);
	return ret;
}

