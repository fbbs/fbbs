#include "libweb.h"

/**
 * Print override information.
 * @param buf the starting address of override struct.
 * @param count not used.
 * @param args not used.
 * @return 0.
 */
static int print_override(void *buf, int count, void *args)
{
	override_t *ov = buf;
	printf("<ov id='%s'>", ov->id);
	xml_fputs(ov->exp, stdout);
	printf("</ov>");
	return 0;
}

int bbsfall_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header("bbsovr");
	printf("<bbsfall %s>", get_session_str());
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, "friends");
	apply_record(file, print_override, sizeof(override_t), NULL, false,
			false, true);
	printf("</bbsfall>");
	return 0;
}

static int cmpname(void *arg, void *buf)
{
	override_t *ov = (override_t *)buf;
	return !strncasecmp(arg, ov->id, sizeof(ov->id));
}

static int cmp_override(const void *key, const void *buf)
{
	override_t *ov = (override_t *)buf;
	return strncasecmp(((override_t *)key)->id, ov->id, sizeof(ov->id));
}

int bbsfadd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	const char *id = getparm("id");
	const char *desc = getparm("desc");
	if (*id != '\0') {
		override_t ov;
		memset(&ov, 0, sizeof(ov));
		strlcpy(ov.id, id, sizeof(ov.id));
		if (!searchuser(ov.id))
			return BBS_ENOUSR;
		strlcpy(ov.exp, desc, sizeof(ov.exp));
		char file[HOMELEN];
		sethomefile(file, currentuser.userid, "friends");
		if (get_num_records(file, sizeof(ov)) == MAXFRIENDS)
			return BBS_EFRNDQE;
		// TODO: be atomic
		if (!search_record(file, NULL, sizeof(ov), cmpname, ov.id))
			append_record(file, &ov, sizeof(ov));
		printf("Location: fall\n\n");
		return 0;
	}
	xml_header("bbsovr");
	printf("<bbsfadd %s>%s</bbsfadd>", get_session_str(), id);
	return 0;
}

int bbsfdel_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char *user = getparm("u");
	if (*user != '\0') {
		char file[HOMELEN];
		sethomefile(file, currentuser.userid, "friends");
		record_t r;
		if (record_open(file, O_RDWR, &r) < 0)
			return BBS_EINTNL;
		override_t key;
		strlcpy(key.id, user, sizeof(key.id));
		override_t *ptr =
				record_search(&r, &key, sizeof(key), lsearch, cmp_override);
		if (ptr != NULL)
			record_delete(&r, ptr, sizeof(*ptr));
		record_close(&r);
	}
	printf("Location: fall\n\n");
	return 0;
}
