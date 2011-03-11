#include "libweb.h"
#include "fbbs/fileio.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

static int edit_article(const char *file, const char *content, const char *ip)
{
	if (file == NULL || content == NULL || ip == NULL)
		return BBS_EINTNL;
	int fd = open(file, O_RDWR);
	if (fd < 0)
		return BBS_EINTNL;
	fb_flock(fd, LOCK_EX);
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
				fb_flock(fd, LOCK_UN);
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
				"%22.22s·HTTP [FROM: %s]\033[m\n", currentuser.userid,
				getdatestring(time(NULL), DATE_ZH), mask_host(ip));
		if (ret == 0)
			ret = safer_write(fd, buf, len);
		size += (e - ptr) + len;
		ret = ftruncate(fd, size);
		fb_flock(fd, LOCK_UN);
		restart_close(fd);
		if (ret == 0)
			return 0;
		return BBS_EINTNL;
	}
	return BBS_EINTNL;	
}

int bbssnd_main(web_ctx_t *ctx)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (parse_post_data(ctx->r) < 0)
		return BBS_EINVAL;
	int bid = strtol(get_param(ctx->r, "bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	bid = bp - bcache + 1;

	bool isedit = (*(get_param(ctx->r, "e")) == '1');
	unsigned int fid;
	struct fileheader fh;
	const char *f = get_param(ctx->r, "f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (bbscon_search(bp, fid, 0, &fh, false) <= 0)
			return BBS_ENOFILE;
		if (!chkBM(bp, &currentuser) && !streq(fh.owner, currentuser.userid)) {
			if (!isedit && fh.accessed[0] & FILE_NOREPLY)
				return BBS_EPST;
			if (isedit)
				return BBS_EACCES;
		}
	}

	char title[sizeof(fh.title)];
	if (!isedit) {
		if (ctx->r->flag & REQUEST_UTF8) {
			convert(ctx->u2g, get_param(ctx->r, "title"), 0,
					title, sizeof(title), NULL, NULL);
		} else {
			strlcpy(title, get_param(ctx->r, "title"), sizeof(title));
		}
		printable_filter(title);
		valid_title(title);
		if (*title == '\0')
			return BBS_EINVAL;
	}

	time_t now = time(NULL);
	int diff = now - u_info->last_post_time;
	u_info->last_post_time = now;
	if (diff < 6)
		return BBS_EPFREQ;

	if (isedit) {
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);
		if (edit_article(file, get_param(ctx->r, "text"), mask_host(fromhost)) < 0)
			return BBS_EINTNL;
	} else {
		post_request_t pr = { .autopost = false, .crosspost = false,
			.userid = NULL, .nick = NULL, .user = &currentuser,
			.bp = bp, .title = title, .content = get_param(ctx->r, "text"),
			.sig = strtol(get_param(ctx->r, "sig"), NULL, 0), .ip = mask_host(fromhost),
			.o_fp = reply ? &fh : NULL, .mmark = false,
			.noreply = fh.accessed[0] & FILE_NOREPLY,
			.anony = strtol(get_param(ctx->r, "anony"), NULL, 0),
			.cp = (ctx->r->flag & REQUEST_UTF8) ? ctx->u2g : NULL
		};
		if (!(fid = do_post_article(&pr)))
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

	snprintf(buf, sizeof(buf), "%sdoc?board=%s", get_doc_mode_str(),
			bp->filename);
	http_header();
	refreshto(1, buf);
	printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>发表</a>"
			"成功，1秒钟后自动转到<a href='%s'>版面</a>\n</body>\n</html>\n",
			bid, fid, buf);
	return 0;
}
