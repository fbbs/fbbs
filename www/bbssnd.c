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
 * @user the owner.
 * @bp the board to post.
 * @title title.
 * @content content.
 * @ip owner's IP address.
 * @o_fp pointer to the replied post. NULL if this is a new thread.
 * @return 0 on success, -1 on error.
 */
int post_article(const struct userec *user, const struct boardheader *bp,
		const char *title, const char *content, 
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
	fprintf(fptr, "\033[m\033[1;%2dm※ 来源:·"BBSNAME" "BBSHOST
			"·HTTP [FROM: %-.20s]\033[m\n", 31 + rand() % 7, ip);
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

int bbssnd_main(void)
{
	if (parse_post_data() < 0)
		return BBS_EINVAL;
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);

	if (!loginok)
		return BBS_ELGNREQ;
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	unsigned int fid;
	struct fileheader fh;
	char *f = getparm("f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (!bbscon_search(bp, fid, 0, &fh))
			return BBS_ENOFILE;
		if (fh.accessed[0] & FILE_NOREPLY)
			return BBS_EACCES;
	}

	char title[sizeof(fh.title)];
	char *t = getparm("title");
	if (*t == '\0')
		return BBS_EINVAL;
	else
		strlcpy(title, t, sizeof(title));
	ansi_filter(title, title);

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
	if (post_article(&currentuser, bp, title, 
			getparm("text"), fromhost, reply ? &fh : NULL) < 0)
		return BBS_EINTNL;

	if (!junkboard(bp)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[sizeof(fh.title) + sizeof(bp->filename)];
	snprintf(buf, sizeof(buf), "posted '%s' on %s", title, bp->filename);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "doc?board=%s", bp->filename);
	http_header();
	refreshto(1, buf);
	printf("</head>\n<body>发表成功，1秒钟后自动转到<a href='%s'>版面</a>\n"
			"</body>\n</html>\n", buf);
	return 0;
}

