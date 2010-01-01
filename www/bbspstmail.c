#include "libweb.h"

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
	xml_header("bbspstmail");
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
