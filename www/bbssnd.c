#include "libweb.h"
#include "fbbs/board.h"
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
				file_close(fd);
				return BBS_EINTNL;
			}
			e = buf + bytes;
		}
		ptr = e - 2; // skip last '\n'
		while (ptr >= buf && *ptr != '\n')
			--ptr;
		if (ptr >= buf) {
			//% if (!strncmp(ptr + 1, "\033[m\033[1;36m※ 修改", 17)) {
			if (!strncmp(ptr + 1, "\033[m\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4", 17)) {
				e = ptr + 1;
				--ptr;
				while (ptr >= buf && *ptr != '\n')
					--ptr;
			}
		}

		lseek(fd, begin, SEEK_SET);
		size_t len = strlen(content);
		size_t size = begin + len;
		int ret = file_write(fd, content, len);
		if (ret == 0 && ptr != e)
			ret = file_write(fd, ptr, e - ptr);
		//% len = snprintf(buf, sizeof(buf), "\033[m\033[1;36m※ 修改:·%s 于 "
		len = snprintf(buf, sizeof(buf), "\033[m\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4:\xa1\xa4%s \xd3\xda "
				//% "%22.22s·HTTP [FROM: %s]\033[m\n", currentuser.userid,
				"%22.22s\xa1\xa4HTTP [FROM: %s]\033[m\n", currentuser.userid,
				format_time(fb_time(), TIME_FORMAT_ZH), mask_host(ip));
		if (ret == 0)
			ret = file_write(fd, buf, len);
		size += (e - ptr) + len;
		ret = ftruncate(fd, size);
		fb_flock(fd, LOCK_UN);
		file_close(fd);
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

extern int search_pid(int bid, post_id_t pid, post_info_t *pi);

int bbssnd_main(void)
{
	if (!session.id)
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

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (!isedit) {
		if (request_type(REQUEST_UTF8)) {
			convert_u2g(get_param("title"), gbk_title);
		} else {
			strlcpy(gbk_title, get_param("title"), sizeof(gbk_title));
		}
		printable_filter(gbk_title);
		valid_title(gbk_title);
		if (*gbk_title == '\0')
			return BBS_EINVAL;
	}

	time_t now = fb_time();
	int diff = now - get_my_last_post_time();
	set_my_last_post_time(now);
	if (diff < 6)
		return BBS_EPFREQ;

	post_id_t pid = 0;
	post_info_t pi;

	const char *f = get_param("f");
	bool reply = !(*f == '\0');

	if (isedit || reply) {
		pid = strtol(f, NULL, 10);
		if (!pid || !search_pid(board.id, pid, &pi))
			return BBS_ENOFILE;

		if (!am_bm(&board) && session.uid != pi.uid) {
			if (!isedit && (pi.flag & POST_FLAG_LOCKED))
				return BBS_EPST;
			if (isedit)
				return BBS_EACCES;
		}
	}

	char *text = (char *)get_param("text");
	_check_character(text);

	if (isedit) {
		char buffer[4096];
		char *utf8_content = post_content_get(pi.id, buffer, sizeof(buffer));

		char file[HOMELEN];
		dump_content_to_gbk_file(utf8_content, CONVERT_ALL, file,
				sizeof(file));

		if (utf8_content != buffer)
			free(utf8_content);

		int ret = edit_article(file, text, mask_host(fromhost));
		unlink(file);

		if (ret < 0)
			return BBS_EINTNL;
	} else {
		post_request_t pr = {
			.autopost = false, .crosspost = false, .uname = NULL, .nick = NULL,
			.user = &currentuser, .board = &board, .title = gbk_title,
			.content = text, .sig = strtol(get_param("sig"), NULL, 0),
			.ip = mask_host(fromhost), .reid = reply ? pi.id : 0,
			.tid = reply ? pi.tid : 0, .marked = false,
			.locked = reply && (pi.flag & POST_FLAG_LOCKED),
			.anony = strtol(get_param("anony"), NULL, 0),
			.cp = request_type(REQUEST_UTF8) ? env_u2g : NULL,
		};
		pid = publish_post(&pr);
		if (!pid)
			return BBS_EINTNL;
	}

	if (!isedit && !(board.flag & BOARD_JUNK_FLAG)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[128];
	snprintf(buf, sizeof(buf), "%sed '%s' on %s", isedit ? "edit" : "post",
			gbk_title, board.name);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "%sdoc?board=%s", get_doc_mode_str(),
			board.name);
	http_header();
	refreshto(1, buf);
	//% printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>发表</a>"
	printf("</head>\n<body><a id='url' href='con?new=1&bid=%d&f=%u'>\xb7\xa2\xb1\xed</a>"
			//% "成功，1秒钟后自动转到<a href='%s'>版面</a>\n</body>\n</html>\n",
			"\xb3\xc9\xb9\xa6\xa3\xac""1\xc3\xeb\xd6\xd3\xba\xf3\xd7\xd4\xb6\xaf\xd7\xaa\xb5\xbd<a href='%s'>\xb0\xe6\xc3\xe6</a>\n</body>\n</html>\n",
			board.id, pid, buf);
	return 0;
}
