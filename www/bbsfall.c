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
	printf("<bbsfall ");
	print_session();
	printf(">");
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
	printf("<bbsfadd ");
	print_session();
	printf(">%s</bbsfadd>", id);
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

static void override_info(void)
{
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, "friends");
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return;
	hash_t ht;
	if (hash_create(&ht, 0, NULL) < 0) {
		mmap_close(&m);
		return;
	}
	int count = m.size / sizeof(override_t);
	if (count > 0) {
		override_t *ov = m.ptr;
		for (int i = 0; i < count; i++) {
			hash_set(&ht, ov->id, HASH_KEY_STRING, ov->id);
			ov++;
		}
		time_t now = time(NULL);
		struct user_info *uinfo = utmpshm->uinfo;
		struct userec *user;
		const char *ip;
		int idle;
		for (int i = 0; i < MAXACTIVE; ++i) {
			if (uinfo->active
					&& !(uinfo->invisible && !HAS_PERM(PERM_SEECLOAK))
					&& hash_get(&ht, uinfo->userid, HASH_KEY_STRING)) {
				user = uidshm->passwd + (uinfo->uid - 1);
				if (HAS_DEFINE(user->userdefine, DEF_NOTHIDEIP))
					ip = mask_host(uinfo->from);
				else
					ip = "......";
				if (uinfo->mode == BBSNET)
					idle = 0;
				else
					idle = (now - uinfo->idle_time) / 60;
				printf("<ov id='%s' action='%s' idle='%d' ip='%s'>",
						uinfo->userid, mode_type(uinfo->mode), idle, ip);
				xml_fputs(uinfo->username, stdout);
				printf("</ov>");
			}
			uinfo++;
		}
	}
	hash_destroy(&ht);
	mmap_close(&m);
}

int bbsovr_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header("bbsovr2");
	printf("<bbsovr2 ");
	print_session();
	printf(">");
	override_info();
	printf("</bbsovr2>");
	return 0;
}
