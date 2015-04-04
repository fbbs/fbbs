#include "libweb.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/web.h"

static bool _is_mail_read(const struct fileheader *fp)
{
	return (fp->accessed[0] & FILE_READ);
}

static int _get_mail_mark(const struct fileheader *fp)
{
	int mark = ' ';
	if (fp->accessed[0] & MAIL_REPLY)
		mark = 'r';
	if (fp->accessed[0] & FILE_MARKED) {
		if (mark == 'r')
			mark = 'b';
		else
			mark = 'm';
	}
	if (!_is_mail_read(fp)) {
		if (mark == ' ')
			mark = '+';
		else
			mark = toupper(mark);
	}
	return mark;
}

int bbsmail_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	int start = strtol(web_get_param("start"), NULL, 10);
	int page = strtol(web_get_param("page"), NULL, 10);
	if (page <= 0 || page > TLINES)
		page = TLINES;
	
	char buf[HOMELEN];
	setmdir(buf, currentuser.userid);
	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;

	int total = m.size / sizeof(struct fileheader);
	if (start <= 0 || start > total - page + 1)
		start = total - page + 1;
	if (start < 1)
		start = 1;

	const struct fileheader *begin = m.ptr, *end = begin + total;
	const struct fileheader *fp = begin + start + page - 2;
	if (fp >= end)
		fp = end - 1;

	xml_header(NULL);
	printf("<bbsmail start='%d' total='%d' page='%d' dpage='%d'>",
			start, total, page, TLINES);
	print_session();

	for (int i = 0; i < page && fp >= begin; ++i, --fp) {
		printf("\n<mail r='%d' m='%c' from='%s' date='%s' name='%s'>",
				_is_mail_read(fp), _get_mail_mark(fp), fp->owner,
				format_time(getfiletime(fp), TIME_FORMAT_XML), fp->filename);
		if (web_request_type(UTF8))
			xml_fputs4(fp->title, 0);
		else
			xml_fputs2(fp->title, check_gbk(fp->title) - fp->title);
		printf("</mail>");
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
		printf("<mail m='%c' from='%s' name='%s' n='%d' date='%s'>",
				_get_mail_mark(fp), fp->owner, fp->filename, count,
				format_time(date, TIME_FORMAT_XML));
		xml_fputs2(fp->title, check_gbk(fp->title) - fp->title);
		printf("</mail>");
	}
	return 0;
}

int bbsnewmail_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	xml_header(NULL);
	printf("<bbsmail new='1'>");
	print_session();
	char file[HOMELEN];
	setmdir(file, currentuser.userid);
	time_t limit = time(NULL) - 24 * 60 * 60 * NEWMAIL_EXPIRE;
	apply_record(file, print_new_mail, sizeof(struct fileheader), &limit,
			false, true, true);
	printf("</bbsmail>");
	return 0;
}

int bbsmailcon_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	const char *file = web_get_param("f");
	if (!valid_mailname(file))
		return BBS_EINVAL;

	char buf[HOMELEN];
	mmap_t m = { .oflag = O_RDWR };
	// deal with index
	setmdir(buf, currentuser.userid);
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;

	int total = m.size / sizeof(struct fileheader);
	struct fileheader *fh = bbsmail_search(m.ptr, m.size, file);
	if (!fh) {
		mmap_close(&m);
		return BBS_ENOFILE;
	}

	bool newmail = false;
	if (!(fh->accessed[0] & FILE_READ)) {
		newmail = true;
		fh->accessed[0] |= FILE_READ;
	}

	xml_header(NULL);
	printf("<bbsmailcon total='%d' dpage='%d'", total, TLINES);
	struct fileheader *prev = fh - 1;
	if (prev >= (struct fileheader *)m.ptr)
		printf(" prev='%s'", prev->filename);
	struct fileheader *next = fh + 1;
	if (next < (struct fileheader *)m.ptr + m.size / sizeof(*next))
		printf(" next='%s'", next->filename);
	if (newmail)
		printf(" new='1'");
	printf("><t>");
	if (web_request_type(UTF8))
		xml_fputs4(fh->title, 0);
	else
		xml_fputs2(fh->title, check_gbk(fh->title) - fh->title);
	printf("</t>");

	mmap_close(&m);

	// show mail content.
	if (file[0] == 's') // shared mail
		strlcpy(buf, file, sizeof(buf));
	else
		setmfile(buf, currentuser.userid, file);

	printf("<mail f='%s' n='%s'>", file, web_get_param("n"));
	xml_printfile(buf);
	fputs("</mail>\n", stdout);

	print_session();
	printf("</bbsmailcon>");
	return 0;
}

int bbsdelmail_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	const char *file = web_get_param("f");
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

int bbspstmail_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	if (!HAS_PERM2(PERM_MAIL, &currentuser))
		return BBS_EACCES;

	// TODO: mail quota check, signature
	int num = 0;
	mmap_t m = { .oflag = O_RDONLY };
	char file[HOMELEN];
	const char *str = web_get_param("n"); // 1-based

	bool found = false;
	struct fileheader fh;
	if (*str) {
		num = strtol(str, NULL, 10);
		if (num <= 0)
			return BBS_EINVAL;
		setmdir(file, currentuser.userid);
		if (mmap_open(file, &m) < 0)
			return BBS_EINTNL;
		int size = m.size / sizeof(fh);
		if (num > size) {
			mmap_close(&m);
			return BBS_ENOFILE;
		}
		memcpy(&fh, (struct fileheader *) m.ptr + num - 1, sizeof(fh));
		found = true;
		mmap_close(&m);
	}

	xml_header(NULL);
	printf("<bbspstmail ");

	printf(" ref='");
	const char *ref = get_referer();
	if (*ref == '\0')
		ref = "pstmail";
	xml_fputs(ref);

	printf("' recv='%s'>", found ? fh.owner : web_get_param("recv"));

	if (found) {
		printf("<t>");
		if (web_request_type(MOBILE) && !strneq2(fh.title, "Re: "))
			puts("Re: ");
		xml_fputs4(fh.title, 0);
		printf("</t><m>");

		setmfile(file, currentuser.userid, fh.filename);
		if (web_request_type(UTF8)) {
			mmap_t mm = { .oflag = O_RDONLY };
			if (mmap_open(file, &mm) == 0) {
				char *utf8_str = malloc(mm.size * 2 + 1);
				convert(CONVERT_G2U, mm.ptr, mm.size, utf8_str, mm.size * 2 + 1,
						NULL, NULL);
				post_quote_string(utf8_str, strlen(utf8_str), NULL,
						POST_QUOTE_AUTO, true, true, xml_fputs3);
				free(utf8_str);
				mmap_close(&mm);
			}
		} else {
			post_quote_file(file, NULL, POST_QUOTE_AUTO, true, false,
					xml_fputs3);
		}
		printf("</m>");
	}
	print_session();
	printf("</bbspstmail>");
	return 0;
}

int bbssndmail_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;
	if (!HAS_PERM2(PERM_MAIL, &currentuser))
		return BBS_EACCES;
	if (parse_post_data() < 0)
		return BBS_EINVAL;

	const char *recv = web_get_param("recv");
	if (*recv == '\0')
		return BBS_EINVAL;

	bool utf8 = web_request_type(UTF8);
	char title[STRLEN];
	if (utf8)
		convert_u2g(web_get_param("title"), title);
	else
		strlcpy(title, web_get_param("title"), sizeof(title));
	string_remove_non_printable_gbk(title);
	valid_title_gbk(title);
	if (*title == '\0')
		//% strlcpy(title, "没主题", sizeof(title));
		strlcpy(title, "\xc3\xbb\xd6\xf7\xcc\xe2", sizeof(title));

	const char *text = web_get_param("text");
	int len = strlen(text);
	char header[320];
	//% snprintf(header, sizeof(header), "寄信人: %s (%s)\n标  题: %s\n发信站: "
	snprintf(header, sizeof(header), "\xbc\xc4\xd0\xc5\xc8\xcb: %s (%s)\n\xb1\xea  \xcc\xe2: %s\n\xb7\xa2\xd0\xc5\xd5\xbe: "
			//% BBSNAME" (%s)\n来  源: %s\n\n", currentuser.userid,
			BBSNAME" (%s)\n\xc0\xb4  \xd4\xb4: %s\n\n", currentuser.userid,
			currentuser.username, title, format_time(fb_time(), TIME_FORMAT_ZH),
			mask_host(fromhost));

	char *gbk_text = (char *) text;
	if (utf8) {
		gbk_text = malloc(len + 1);
		convert(CONVERT_U2G, text, len, gbk_text, len, NULL, NULL);
		len = strlen(gbk_text);
	}

	// TODO: signature, error code
	if (do_mail_file(recv, title, header, gbk_text, len, NULL) < 0) {
		if (utf8)
			free(gbk_text);
		return BBS_EINVAL;
	}

	if (*web_get_param("backup") != '\0') {
		char title2[STRLEN];
		snprintf(title2, sizeof(title2), "{%s} %s", recv, title);
		do_mail_file(currentuser.userid, title2, header, gbk_text, len, NULL);
	}
	free(gbk_text);

	const char *ref = web_get_param("ref");
	http_header();
	refreshto(1, ref);
	if (utf8)
		printf("</head>\n<body>发表成功，1秒钟后自动转到<a href='%s'>原页面</a>\n");
	else
		printf("</head>\n<body>\xb7\xa2\xb1\xed\xb3\xc9\xb9\xa6\xa3\xac""1\xc3\xeb\xd6\xd3\xba\xf3\xd7\xd4\xb6\xaf\xd7\xaa\xb5\xbd<a href='%s'>\xd4\xad\xd2\xb3\xc3\xe6</a>\n"
			"</body>\n</html>\n", ref);
	return 0;
}

static int _mail_checked(void *ptr, void *file)
{
	struct fileheader *p = ptr;
	return streq(p->filename, file);
}

int web_mailman(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	parse_post_data();

	char index[HOMELEN];
	setmdir(index, currentuser.userid);

	xml_header(NULL);
	printf("<mailman>");
	print_session();

	const pair_t *p = NULL;
	for (int i = 0; (p = web_get_param_pair(i)); ++i) {
		if (streq(p->val, "on") && strncmp(p->key, "box", 3) == 0) {
			const char *file = p->key + sizeof("box") - 1;
			if (delete_record(index, sizeof(struct fileheader), 1,
					_mail_checked, (void *)file) == 0) {
				if (file[0] != 's') { // not shared mail
					char buf[HOMELEN];
					setmfile(buf, currentuser.userid, file);
					unlink(buf);
				}
				printf("<mail f='%s'/>", file);
			}
		}
	}

	printf("</mailman>");
	return 0;
}
