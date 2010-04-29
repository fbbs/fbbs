#include "libweb.h"
#include "post.h"

extern bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp);

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
		strlcpy(title, getparm("title"), sizeof(title));
		printable_filter(title);
		if (*title == '\0')
			return BBS_EINVAL;
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
		post_request_t pr = { .autopost = false, .crosspost = false,
			.userid = NULL, .nick = NULL, .user = &currentuser,
			.bp = bp, .title = title, .content = getparm("text"),
			.sig = strtol(getparm("sig"), NULL, 0), .ip = mask_host(fromhost),
			.o_fp = reply ? &fh : NULL, .noreply = false, .mmark = false };
		if (do_post_article(&pr) < 0)
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
