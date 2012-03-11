#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
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

static char *_check_character(char *text)
{
	char *dst = text, *src;
	for (src = text; *src != '\0'; ++src) {
		switch (*src) {
			case '\x1': case '\x2': case '\x3': case '\x4': case '\x5':
			case '\x6': case '\x7': case '\x8': case '\xb': case '\xc':
			case '\xe': case '\xf': case '\x10': case '\x11': case '\x12':
			case '\x13': case '\x14': case '\x15': case '\x16': case '\x17':
			case '\x18': case '\x19': case '\x1a': case '\x1c': case '\x1d':
			case '\x1e': case '\x1f': case '\r':
				break;
			default:
				*dst++ = *src;
				break;
		}
	}
	*dst = '\0';
	return text;
}

int bbssnd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (parse_post_data() < 0)
		return BBS_EINVAL;

	board_t board;
	if (!get_board_by_bid(strtol(get_param("bid"), NULL, 10), &board)
			|| !has_post_perm(&currentuser, &board))
		return BBS_ENOBRD;
	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	bool isedit = (*(get_param("e")) == '1');
	unsigned int fid = 0;
	struct fileheader fh;
	const char *f = get_param("f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (bbscon_search(board.name, fid, 0, &fh, false) <= 0)
			return BBS_ENOFILE;
		if (!am_bm(&board)
				&& !streq(fh.owner, currentuser.userid)) {
			if (!isedit && fh.accessed[0] & FILE_NOREPLY)
				return BBS_EPST;
			if (isedit)
				return BBS_EACCES;
		}
	}

	char title[sizeof(fh.title)];
	if (!isedit) {
		if (ctx.r->flag & REQUEST_UTF8) {
			convert_u2g(get_param("title"), title);
		} else {
			strlcpy(title, get_param("title"), sizeof(title));
		}
		printable_filter(title);
		valid_title(title);
		if (*title == '\0')
			return BBS_EINVAL;
	}

	time_t now = time(NULL);
	int diff = now - get_last_post_time();
	set_last_post_time(now);
	if (diff < 6)
		return BBS_EPFREQ;

	char *text = (char *)get_param("text");
	_check_character(text);

	if (isedit) {
		char file[HOMELEN];
		setbfile(file, board.name, fh.filename);
		if (edit_article(file, text, mask_host(fromhost)) < 0)
			return BBS_EINTNL;
	} else {
		post_request_t pr = { .autopost = false, .crosspost = false,
			.userid = NULL, .nick = NULL, .user = &currentuser,
			.board = &board, .title = title, .content = text,
			.sig = strtol(get_param("sig"), NULL, 0), .ip = mask_host(fromhost),
			.o_fp = reply ? &fh : NULL, .mmark = false,
			.noreply = reply && fh.accessed[0] & FILE_NOREPLY,
			.anony = strtol(get_param("anony"), NULL, 0),
			.cp = (ctx.r->flag & REQUEST_UTF8) ? env.u2g : NULL
		};
		if (!(fid = do_post_article(&pr)))
			return BBS_EINTNL;
	}

	if (!isedit && !(board.flag & BOARD_JUNK_FLAG)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[sizeof(fh.title) + sizeof(board.name)];
	snprintf(buf, sizeof(buf), "%sed '%s' on %s", isedit ? "edit" : "post",
			title, board.name);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "%sdoc?board=%s", get_doc_mode_str(),
			board.name);
	http_header();
	refreshto(1, buf);
	printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>发表</a>"
			"成功，1秒钟后自动转到<a href='%s'>版面</a>\n</body>\n</html>\n",
			board.id, fid, buf);
	return 0;
}
