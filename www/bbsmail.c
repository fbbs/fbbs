#include "libweb.h"
#include "mmap.h"
#include "record.h"

int bbsmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	int start = strtol(getparm("start"), NULL, 10);
	char buf[HOMELEN];
	setmdir(buf, currentuser.userid);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;
	int total = m.size / sizeof(struct fileheader);
	if (start <= 0)
		start = total - TLINES + 1;
	if (start < 1)
		start = 1;
	struct fileheader *fh = (struct fileheader *)m.ptr + start - 1;
	struct fileheader *end = (struct fileheader *)m.ptr + total;
	xml_header("bbs");
	printf("<bbsmail start='%d' total='%d' page='%d' ", start, total, TLINES);
	print_session();
	printf(">");
	for (int i = 0; i < TLINES && fh != end; ++i) {
		int mark = ' ';
		if (fh->accessed[0] & MAIL_REPLY)
			mark = 'r';
		if (fh->accessed[0] & FILE_MARKED) {
			if (mark == 'r')
				mark = 'b';
			else
				mark = 'm';
		}
		if (!(fh->accessed[0] & FILE_READ)) {
			if (mark == ' ')
				mark = '+';
			else
				mark = toupper(mark);
		}
		printf("<mail m='%c' from='%s' date='%s' name='%s'>", mark, fh->owner,
				getdatestring(getfiletime(fh), DATE_XML), fh->filename);
		xml_fputs2(fh->title, check_gbk(fh->title) - fh->title, stdout);
		printf("</mail>");
		fh++;
	}
	mmap_close(&m);
	printf("</bbsmail>");
	return 0;
}

int print_new_mail(void *buf, int count, void *args)
{
	struct fileheader *fp = buf;
	time_t limit = *(time_t *)args;
	time_t date = getfiletime(fp);
	if (date < limit)
		return QUIT;
	if (!(fp->accessed[0] & FILE_READ)) {
		printf("<mail from='%s' date='%s' name='%s' n='%d'>", fp->owner, 
				getdatestring(date, DATE_XML), fp->filename, count);
		xml_fputs2(fp->title, check_gbk(fp->title) - fp->title, stdout);
		printf("</mail>");
	}
	return 0;
}

int bbsnewmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header("bbs");
	printf("<bbsnewmail ");
	print_session();
	printf(">");
	char file[HOMELEN];
	setmdir(file, currentuser.userid);
	time_t limit = time(NULL) - 24 * 60 * 60 * NEWMAIL_EXPIRE;
	apply_record(file, print_new_mail, sizeof(struct fileheader), &limit,
			false, true, true);
	printf("</bbsnewmail>");
	return 0;
}

int bbsmailcon_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char file[40];
	strlcpy(file, getparm("f"), sizeof(file));
	if (!valid_mailname(file))
		return BBS_EINVAL;
	char buf[HOMELEN];
	mmap_t m;
	m.oflag = O_RDWR;
	// deal with index
	setmdir(buf, currentuser.userid);
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;
	struct fileheader *fh = bbsmail_search(m.ptr, m.size, file);
	if (fh == NULL) {
		mmap_close(&m);
		return BBS_ENOFILE;
	}
	bool newmail = false;
	if (!(fh->accessed[0] & FILE_READ)) {
		newmail = true;
		fh->accessed[0] |= FILE_READ;
	}
	xml_header("bbs");
	printf("<bbsmailcon ");
	print_session();
	struct fileheader *prev = fh - 1;
	if (prev >= (struct fileheader *)m.ptr)
		printf(" prev='%s'", prev->filename);
	struct fileheader *next = fh + 1;
	if (next < (struct fileheader *)m.ptr + m.size / sizeof(*next))
		printf(" next='%s'", next->filename);
	if (newmail)
		printf(" new='1'");
	printf("><t>");
	xml_fputs2(fh->title, check_gbk(fh->title) - fh->title, stdout);
	printf("</t>");
	mmap_close(&m);

	// show mail content.
	if (file[0] == 's') // shared mail
		strlcpy(buf, file, sizeof(buf));
	else
		setmfile(buf, currentuser.userid, file);
	m.oflag = O_RDONLY;
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;
	printf("<mail f='%s' n='%s'>", file, getparm("n"));
	xml_fputs((char *)m.ptr, stdout);
	fputs("</mail>\n", stdout);
	mmap_close(&m);
	printf("</bbsmailcon>");
	return 0;
}

int bbsdelmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char file[40];
	strlcpy(file, getparm("f"), sizeof(file));
	if (!valid_mailname(file))
		return BBS_EINVAL;
	char buf[HOMELEN];
	mmap_t m;
	setmdir(buf, currentuser.userid);
	m.oflag = O_RDWR;
	if (mmap_open(buf, &m) < 0)
		return BBS_EINTNL;
	struct fileheader *fh = bbsmail_search(m.ptr, m.size, file);
	if (fh == NULL) {
		mmap_close(&m);
		return BBS_ENOFILE;
	}
	struct fileheader *end = 
			(struct fileheader *)m.ptr + m.size / sizeof(*fh);
	if (fh < end) {
		memmove(fh, fh + 1, (end - fh) * sizeof(*fh));
		mmap_truncate(&m, m.size - sizeof(*fh));
	}
	mmap_close(&m);
	if (file[0] != 's') {// not shared mail
		setmfile(buf, currentuser.userid, file);
		unlink(buf);
	}
	xml_header("bbsdelmail");
	printf("<bbsdelmail></bbsdelmail>\n");
	return 0;
}

extern int web_quotation(const char *str, size_t size, const char *owner, bool ismail);

int bbspstmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (!HAS_PERM2(PERM_MAIL, &currentuser))
		return BBS_EACCES;
	// TODO: mail quota check, signature
	int num = 0;
	mmap_t m;
	m.oflag = O_RDONLY;
	char file[HOMELEN];
	const char *str = getparm("n"); // 1-based
	const struct fileheader *fh = NULL;
	if (*str != '\0') {
		num = strtol(str, NULL, 10);
		if (num <= 0)
			return BBS_EINVAL;
		setmdir(file, currentuser.userid);
		if (mmap_open(file, &m) < 0)
			return BBS_EINTNL;
		int size = m.size / sizeof(*fh);
		if (num > size) {
			mmap_close(&m);
			return BBS_ENOFILE;
		}
		fh = (struct fileheader *)m.ptr + num - 1;
	}
	xml_header("bbs");
	printf("<bbspstmail ");
	print_session();

	printf(" ref='");
	const char *ref = get_referer();
	if (*ref == '\0')
		ref = "pstmail";
	xml_fputs(ref, stdout);

	printf("' recv='%s'>", fh == NULL ? getparm("recv") : fh->owner);

	if (fh != NULL) {
		printf("<t>");
		xml_fputs(fh->title, stdout);
		printf("</t><m>");
		mmap_t m2;
		m2.oflag = O_RDONLY;
		setmfile(file, currentuser.userid, fh->filename);
		if (mmap_open(file, &m2) == 0) {
			web_quotation(m2.ptr, m2.size, fh->owner, true);
		}
		mmap_close(&m2);
		printf("</m>");
	}
	mmap_close(&m);
	printf("</bbspstmail>");
	return 0;
}

int bbssndmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (!HAS_PERM2(PERM_MAIL, &currentuser))
		return BBS_EACCES;
	if (parse_post_data() < 0)
		return BBS_EINVAL;

	const char *recv = getparm("recv");
	if (*recv == '\0')
		return BBS_EINVAL;

	char title[STRLEN];
	strlcpy(title, getparm("title"), sizeof(title));
	printable_filter(title);
	if (*title == '\0')
		strlcpy(title, "没主题", sizeof(title));

	const char *text = getparm("text");
	int len = strlen(text);
	char header[320];
	snprintf(header, sizeof(header), "寄信人: %s (%s)\n标  题: %s\n发信站: "
			BBSNAME" (%s)\n来  源: %s\n\n", currentuser.userid,
			currentuser.username, title, getdatestring(time(NULL), DATE_ZH),
			mask_host(fromhost));
	// TODO: signature, error code
	if (do_mail_file(recv, title, header, text, len, NULL) < 0)
		return BBS_EINVAL;
	if (*getparm("backup") != '\0') {
		char title2[STRLEN];
		snprintf(title2, sizeof(title2), "{%s} %s", recv, title);
		do_mail_file(currentuser.userid, title2, header, text, len, NULL);
	}
	const char *ref = getparm("ref");
	http_header();
	refreshto(1, ref);
	printf("</head>\n<body>发表成功，1秒钟后自动转到<a href='%s'>原页面</a>\n"
			"</body>\n</html>\n", ref);
	return 0;
}
