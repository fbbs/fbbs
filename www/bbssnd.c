#include "libweb.h"

extern bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp);

// similar to 'date_to_fname()'.
// Creates a new file in 'dir' with prefix 'pfx'.
// Returns filename(in 'fname') and stream on success, NULL on error.
static FILE *get_fname(const char *dir, const char *pfx, char *fname, size_t size)
{
	if (dir == NULL || pfx == NULL)
		return NULL;
	const char c[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA";
	int t = (int)time(NULL);
	int count = snprintf(fname, size, "%s%s%d. ", dir, pfx, t);
	if (count < 0 || count >= size)
		return NULL;
	int fd;
	for (int i = sizeof(c) - 2; i >= 0; ++i) {
		fname[count - 1] = c[i];
		if ((fd = open(fname, O_CREAT | O_WRONLY | O_EXCL, 0644)) > 0)
			return fdopen(fd, "w");
	}
	return NULL;
}

/**
 * Post an article.
 * @param user The owner.
 * @param bp The board to post.
 * @param title The title.
 * @param content The content.
 * @param cross Whether this is a cross post.
 * @param ip The owner's IP address.
 * @param o_fp Pointer to the replied post. NULL if this is a new thread.
 * @return 0 on success, -1 on error.
 */
int post_article(const struct userec *user, const struct boardheader *bp,
		const char *title, const char *content, bool cross,
		const char *ip, const struct fileheader *o_fp)
{
	if (user == NULL || bp == NULL || title == NULL 
			|| content == NULL || ip == NULL)
		return -1;

	char fname[HOMELEN];
	char dir[HOMELEN];
	int idx = snprintf(dir, sizeof(dir), "boards/%s/", bp->filename);
	const char *pfx = "M.";
	FILE *fptr;
	if ((fptr = get_fname(dir, pfx, fname, sizeof(fname))) == NULL)
		return -1;
	fprintf(fptr, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			user->userid, user->username, bp->filename, title, BBSNAME,
			getdatestring(time(NULL), DATE_ZH));
	fputs(content, fptr);
	fprintf(fptr, "\n--\n");
	// TODO: signature
	fprintf(fptr, "\033[m\033[1;%2dm※ %s:·"BBSNAME" "BBSHOST
			"·HTTP [FROM: %-.20s]\033[m\n", 31 + rand() % 7,
			cross ? "转载" : "来源", ip);
	fclose(fptr);

	struct fileheader fh;
	memset(&fh, 0, sizeof(fh));	
	strlcpy(fh.filename, fname + idx, sizeof(fh.filename));
	strlcpy(fh.owner, user->userid, sizeof(fh.owner));
	strlcpy(fh.title, title, sizeof(fh.title));
	// TODO: assure fid order in .DIR
	fh.id = get_nextid2(bp);
	if (o_fp != NULL) { //reply
		fh.reid = o_fp->id;
		fh.gid = o_fp->gid;
	} else {
		fh.reid = fh.id;
		fh.gid = fh.id;
	}
	setwbdir(dir, bp->filename);
	append_record(dir, &fh, sizeof(fh));
	updatelastpost(bp->filename);
	return 0;
}

static int edit_article(const char *file, const char *content, const char *ip)
{
	if (file == NULL || content == NULL || ip == NULL)
		return BBS_EINTNL;
	int fd = open(file, O_RDWR);
	if (fd < 0)
		return BBS_EINTNL;
	flock(fd, LOCK_EX);
	char buf[4096];
	ssize_t bytes = read(fd, buf, sizeof(buf));
	if (bytes >= 0) {
		// skip header.
		char *ptr = buf, *e = buf + bytes;
		int n = 3;
		while (ptr != e && n >= 0) {
			if (*ptr == '\n')
				--n;
			++ptr;
		}
		int begin = ptr - buf;

		if (bytes == sizeof(buf)) {
			lseek(fd, -sizeof(buf), SEEK_END);
			bytes = read(fd, buf, sizeof(buf));
			if (bytes < sizeof(buf)) {
				flock(fd, LOCK_UN);
				restart_close(fd);
				return BBS_EINTNL;
			}
			e = buf + bytes;
		}
		ptr = e - 2; // skip last '\n'
		while (ptr >= buf && *ptr != '\n')
			--ptr;
		if (ptr >= buf) {
			if (!strncmp(ptr + 1, "\033[m\033[1;36m※ 修改", 17)) {
				e = ptr + 1;
				--ptr;
				while (ptr >= buf && *ptr != '\n')
					--ptr;
			}
		}

		lseek(fd, begin, SEEK_SET);
		size_t len = strlen(content);
		size_t size = begin + len;
		int ret = safer_write(fd, content, len);
		if (ret == 0 && ptr != e)
			ret = safer_write(fd, ptr, e - ptr);
		len = snprintf(buf, sizeof(buf), "\033[m\033[1;36m※ 修改:·%s 于 "
				"%22.22s·HTTP [FROM: %-.20s]\033[m\n", currentuser.userid,
				getdatestring(time(NULL), DATE_ZH), mask_host(ip));
		if (ret == 0)
			ret = safer_write(fd, buf, len);
		size += (e - ptr) + len;
		ftruncate(fd, size);
		flock(fd, LOCK_UN);
		restart_close(fd);
		if (ret == 0)
			return 0;
		return BBS_EINTNL;
	}
	return BBS_EINTNL;	
}

int bbssnd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (parse_post_data() < 0)
		return BBS_EINVAL;
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	bool isedit = (*(getparm("e")) == '1');
	unsigned int fid;
	struct fileheader fh;
	char *f = getparm("f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (!bbscon_search(bp, fid, 0, &fh))
			return BBS_ENOFILE;
		if (!isedit && fh.accessed[0] & FILE_NOREPLY)
			return BBS_EPST;
		if (isedit && !chkBM(bp, &currentuser)
				&& strcmp(fh.owner, currentuser.userid))
			return BBS_EACCES;
	}

	char title[sizeof(fh.title)];
	if (!isedit) {
		char *t = getparm("title");
		if (*t == '\0')
			return BBS_EINVAL;
		else
			strlcpy(title, t, sizeof(title));
		ansi_filter(title, title);
	}

// TODO: ...
#ifdef SPARC
		if(abs(time(0) - *(int*)(u_info->from+34))<6) { //modified from 36 to 34 for sparc solaris by roly 02.02.28
			*(int*)(u_info->from+34)=time(0); //modified from 36 to 34 for sparc solaris by roly 02.02.28
			return BBS_EPFREQ;
		}
		*(int*)(u_info->from+34)=time(0);//modified from 36 to 34 for sparc solaris by roly 02.02.28
#else
		if(abs(time(0) - *(int*)(u_info->from+36))<6) { //modified from 36 to 34 for sparc solaris by roly 02.02.28
			*(int*)(u_info->from+36)=time(0); //modified from 36 to 34 for sparc solaris by roly 02.02.28
			return BBS_EPFREQ;
		}
		*(int*)(u_info->from+36)=time(0);//modified from 36 to 34 for sparc solaris by roly 02.02.28
#endif

	if (isedit) {
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);
		if (edit_article(file, getparm("text"), mask_host(fromhost)) < 0)
			return BBS_EINTNL;
	} else {
		if (post_article(&currentuser, bp, title, getparm("text"),
				false, mask_host(fromhost), reply ? &fh : NULL) < 0)
			return BBS_EINTNL;
	}

	if (!isedit && !junkboard(bp)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[sizeof(fh.title) + sizeof(bp->filename)];
	snprintf(buf, sizeof(buf), "%sed '%s' on %s", isedit ? "edit" : "post",
			title, bp->filename);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "doc?board=%s", bp->filename);
	http_header();
	refreshto(1, buf);
	printf("</head>\n<body>发表成功，1秒钟后自动转到<a href='%s'>版面</a>\n"
			"</body>\n</html>\n", buf);
	return 0;
}
