#include "bbs.h"

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

int getmailsize(const char *userid)
{
	struct fileheader fcache;
	struct stat DIRst, SIZEst, st;
	char sizefile[50], dirfile[256], mailfile[256];
	FILE *fp;
	int mailsize= -1, fd, ssize = sizeof(struct fileheader);

	setmdir(dirfile, userid);
	sprintf(sizefile, "tmp/%s.mailsize", userid);
	if (stat(dirfile, &DIRst)==-1||DIRst.st_size==0)
		mailsize = 0;
	else if (stat(sizefile, &SIZEst)!=-1 && SIZEst.st_size!=0
			&& SIZEst.st_ctime >= DIRst.st_ctime) {
		fp = fopen(sizefile, "r");
		if (fp) {
			fscanf(fp, "%d", &mailsize);
			fclose(fp);
		}
	}
	if (mailsize != -1)
		return mailsize;

	mailsize = 0;
	if (stat(dirfile, &st)!=-1)
		mailsize+=(st.st_size/1024+1);
	fd = open(dirfile, O_RDONLY);
	if (fd != -1) {
		while (read(fd, &fcache, ssize) == ssize) {
			sprintf(mailfile, "mail/%c/%s/%s", toupper(userid[0]), userid,
					fcache.filename);
			if (stat(mailfile, &st)!=-1) {
				mailsize += (st.st_size/1024+1);
			}
		}
		close(fd);
	}
	fp = fopen(sizefile, "w+");
	if (fp) {
		fprintf(fp, "%d", mailsize);
		fclose(fp);
	}
	return mailsize;
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

	if (!getuser(recv))
		return BBS_EINTNL;
	sethomefile(filepath, recv, "rejects");
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

	sprintf(filepath, "mail/%c/%s", toupper(recv[0]), recv);
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0755) == -1)
			return BBS_EINTNL;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return BBS_EINTNL;
	}
	// TODO: get_fname?
	sprintf(fname, "M.%ld.A", time(NULL));
	sprintf(filepath, "mail/%c/%s/%s", toupper(recv[0]), recv, fname);
	ip = strrchr(fname, 'A');
	count = 0;
	while ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(filepath, "mail/%c/%s/%s", toupper(recv[0]), recv,
				fname);
		if (count++ > MAX_POSTRETRY) {
			return BBS_EINTNL;
		}
	}
	flock(fd, LOCK_EX);
	strlcpy(fh.filename, fname, sizeof(fh.filename));
	sprintf(filepath, "mail/%c/%s/%s", toupper(recv[0]), recv, fname);
	if (header != NULL)
		write(fd, header, strlen(header));
	write(fd, text, len);
	if (source != NULL)
		write(fd, source, strlen(source));
	flock(fd, LOCK_UN);
	close(fd);
	setmdir(fname, recv);
	if (append_record(fname, &fh, sizeof(fh)) == -1)
		return BBS_EINTNL;

	char buf[256];
	sprintf(buf, "mailed %s: %s ", recv, title);
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

