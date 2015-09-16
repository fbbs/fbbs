#define _GNU_SOURCE
#include "bbs.h"
#include "fbbs/record.h"
#include "fbbs/string.h"
#include "fbbs/tui_list.h"
#include "fbbs/terminal.h"

extern post_list_position_t *post_list_get_position(const char *key);
extern void post_list_set_position(post_list_position_t *plp, int top, int cur);
extern void post_list_position_key(int board_id, int type, char *buf);
extern void post_list_position_load(const post_list_position_t *plp, int *top, int *cur);

typedef struct {
	const char *bname;
	record_t *record;
	struct fileheader *buf;
	post_list_position_t *plp;
	int board_id;
	int size;
} tui_attachment_list_t;

typedef struct {
	char uname[IDLEN + 1];
	UTF8_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
} attachment_filter_t;

static int attachment_filter(void *ptr, void *args, int offset)
{
	const struct fileheader *fp = ptr;
	const attachment_filter_t *filter = args;
	if (*filter->uname && !strcaseeq(fp->owner, filter->uname))
		return RECORD_CALLBACK_CONTINUE;
	if (*filter->utf8_keyword && !strcasestr(fp->title, filter->utf8_keyword))
		return RECORD_CALLBACK_CONTINUE;
	return RECORD_CALLBACK_MATCH;
}

static int attachment_search(tui_list_t *tl,
		const attachment_filter_t *filter, bool upward)
{
	tui_attachment_list_t *tal = tl->data;
	int pos = record_search(tal->record, attachment_filter, (void *) filter,
			tl->cur, upward);
	if (pos > 0) {
		tl->cur = pos;
		tl->valid = false;
		return PARTUPDATE;
	}
	return MINIUPDATE;
}

static int attachment_record_cmp(const void *r1, const void *r2)
{
	const struct fileheader *f1 = r1, *f2 = r2;
	return strcmp(f1->filename, f2->filename);
}

static int attachment_open_record(const char *bname, record_perm_e rdonly,
		record_t *record)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "upload/%s/.DIR", bname);
	return record_open(file, attachment_record_cmp, sizeof(struct fileheader),
				rdonly, record);
}

static void save_post_list_position(const tui_list_t *tl)
{
	tui_attachment_list_t *tal = tl->data;
	post_list_set_position(tal->plp, tl->begin, tl->cur);
}

tui_list_loader_t tui_attachment_loader(tui_list_t *tl)
{
	tui_attachment_list_t *tal = tl->data;
	tl->all = record_count(tal->record);
	if (!tal->plp) {
		char key[POST_LIST_POSITION_KEY_LEN];
		post_list_position_key(tal->board_id, POST_LIST_ATTACHMENT, key);
		tal->plp = post_list_get_position(key);
		post_list_position_load(tal->plp, &tl->begin, &tl->cur);
		if (tl->begin < 0) {
			tl->begin = tl->all - tl->lines;
			tl->cur = tl->all - 1;
		}
	}
	if (tl->begin < 0)
		tl->begin = 0;
	if (tl->cur < 0)
		tl->cur = 0;
	tal->size = record_read_after(tal->record, tal->buf, tl->lines, tl->begin);
	save_post_list_position(tl);
	return 0;
}

extern void _post_list_title(int archive_list, const char *mode);

tui_list_title_t tui_attachment_title(tui_list_t *tl)
{
	// 附件区
	_post_list_title(false, "\xb8\xbd\xbc\xfe\xc7\xf8");
}

extern void format_post_date(fb_time_t stamp, char *buf, size_t size);

tui_list_display_t tui_attachment_display(tui_list_t *tl, int offset)
{
	tui_attachment_list_t *tal = tl->data;
	offset -= tl->begin;
	if (offset < tal->size) {
		const struct fileheader *fp = tal->buf + offset;

		char date[14];
		format_post_date(fp->timeDeleted, date, sizeof(date));

		char type = post_mark_raw(0, fp->accessed[0]);

		char buf[STRLEN];
		snprintf(buf, sizeof(buf), " %5d %c %-12.12s %s\033[m %s\n",
				tl->begin + offset + 1, type, fp->owner, date,
				fp->filename);
		outs(buf);
	}
	return 0;
}

static void print_attachment_info(const struct fileheader *fp,
		const char *bname, int offset)
{
	char url[STRLEN];
	snprintf(url, sizeof(url), "://"BBSHOST_PUBLIC"/upload/%s/%s", bname,
			fp->filename);

	screen_clear();
	screen_printf("文件详细信息\n\n"
			"版名: %s\n"
			"序号: 第 %d 篇\n"
			"文件: %s\n"
			"上传: %s\n"
			"日期: %s\n"
			"大小: %d 字节\n"
			"地址:\n"
			"https%s\n\n"
			"http%s",
			bname, offset + 1, fp->filename, fp->owner,
			format_time(fp->timeDeleted, TIME_FORMAT_UTF8_ZH), fp->id,
			url, url);
}

static int attachment_search_author(tui_list_t *tl,
		const struct fileheader *fp, bool upward)
{
	char prompt[80];
	snprintf(prompt, sizeof(prompt), "向%s搜索作者 [%s]: ",
			upward ? "上" : "下", fp->owner);
	char ans[IDLEN + 1];
	tui_input(-1, prompt, ans, sizeof(ans));

	attachment_filter_t filter = { .utf8_keyword = { '\0' } };
	strlcpy(filter.uname, *ans ? ans : fp->owner, sizeof(filter.uname));
	return attachment_search(tl, &filter, upward);
}

static int attachment_search_title(tui_list_t *tl, bool upward)
{
	char prompt[80];
	GBK_BUFFER(title, POST_TITLE_CCHARS) = "";
	//% 向%s搜索标题
	snprintf(prompt, sizeof(prompt),
			"\xcf\xf2%s\xcb\xd1\xcb\xf7\xb1\xea\xcc\xe2[%s]: ",
			//% "上" : "下"
			upward ? "\xc9\xcf" : "\xcf\xc2", gbk_title);

	GBK_BUFFER(ans, POST_TITLE_CCHARS);
	getdata(-1, 0, prompt, gbk_ans, sizeof(gbk_ans), DOECHO, YEA);

	if (*gbk_ans != '\0')
		strlcpy(gbk_title, gbk_ans, sizeof(gbk_title));

	if (!*gbk_title != '\0')
		return MINIUPDATE;

	attachment_filter_t filter = { .utf8_keyword = { '\0' } };
	convert_g2u(gbk_title, filter.utf8_keyword);
	return attachment_search(tl, &filter, upward);
}

typedef struct {
	struct fileheader *fp;
	post_flag_e flag;
} attachment_update_flag_t;

static record_callback_e attachment_update_flag(void *ptr, void *args,
		int offset)
{
	struct fileheader *fp = ptr;
	attachment_update_flag_t *auf = args;
	if (streq(fp->filename, auf->fp->filename)) {
		if (fp->accessed[0] & auf->flag)
			fp->accessed[0] &= ~auf->flag;
		else
			fp->accessed[0] |= auf->flag;
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int attachment_reopen_record(tui_attachment_list_t *tal)
{
	record_close(tal->record);
	return attachment_open_record(tal->bname, RECORD_WRITE, tal->record);
}

static int toggle_attachment_flag(tui_list_t *tl, struct fileheader *fp,
		post_flag_e flag)
{
	tui_attachment_list_t *tal = tl->data;
	if (fp && am_curr_bm()) {
		if (attachment_reopen_record(tal) < 0)
			return DONOTHING;
		attachment_update_flag_t auf = { .fp = fp, .flag = flag };
		record_update(tal->record, fp, tl->cur, attachment_update_flag, &auf);
		tl->valid = false;
		return PARTUPDATE;
	}
	return DONOTHING;
}

typedef struct {
	const tui_attachment_list_t *tal;
	const struct fileheader *fp;
} delete_attachment_callback_t;

static record_callback_e delete_attachment_callback(void *ptr, void *args,
		int offset)
{
	const struct fileheader *fp = ptr;
	delete_attachment_callback_t *dac = args;
	if (streq(fp->filename, dac->fp->filename)) {
		char file[HOMELEN];
		snprintf(file, sizeof(file), "upload/%s/%s", dac->tal->bname,
				dac->fp->filename);
		unlink(file);
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int delete_attachment(tui_list_t *tl, struct fileheader *fp)
{
	if (fp && ((streq(currentuser.userid, fp->owner)
			&& currentuser.firstlogin < fp->timeDeleted) || am_curr_bm())) {
		screen_move(-1, 0);
		//% 确定删除
		if (askyn("\xc8\xb7\xb6\xa8\xc9\xbe\xb3\xfd", NA, NA)) {
			tui_attachment_list_t *tal = tl->data;
			if (attachment_reopen_record(tal) < 0)
				return DONOTHING;
			delete_attachment_callback_t dac = { .tal = tal, .fp = fp };
			record_delete(tal->record, fp, tl->cur,
					delete_attachment_callback, &dac);
			tl->valid = false;
			return PARTUPDATE;
		}
		return MINIUPDATE;
	}
	return DONOTHING;
}

extern int tui_follow_uname(const char *uname);

static tui_list_query_t tui_attachment_query(tui_list_t *tl)
{
	struct fileheader *fp = NULL;
	tui_attachment_list_t *tal = tl->data;
	if (tl->cur >= tl->begin && tl->cur - tl->begin < tal->size)
		fp = tal->buf + tl->cur - tl->begin;
	if (fp) {
		screen_clear();
		print_attachment_info(fp, tal->bname, tl->cur);
		tl->in_query = true;
		return DONOTHING;
	}
	tl->in_query = false;
	tl->update = FULLUPDATE;
	return FULLUPDATE;
}

tui_list_handler_t tui_attachment_handler(tui_list_t *tl, int key)
{
	struct fileheader *fp = NULL;
	tui_attachment_list_t *tal = tl->data;
	if (tl->cur - tl->begin < tal->size)
		fp = tal->buf + tl->cur - tl->begin;

	switch (key) {
		case 'q': case 'e': case KEY_LEFT: case EOF:
			save_post_list_position(tl);
			return READ_AGAIN;
		case 'M':
			m_new();
			return FULLUPDATE;
		default:
			if (!fp)
				return READ_AGAIN;
	}

	switch (key) {
		case '\n': case '\r':
		case KEY_RIGHT: case 'r': case Ctrl('S'): case 'p':
			return tui_attachment_query(tl);
		case 'm':
			return toggle_attachment_flag(tl, fp, POST_FLAG_MARKED);
		case 'g':
			return toggle_attachment_flag(tl, fp, POST_FLAG_DIGEST);
		case 'w':
			return toggle_attachment_flag(tl, fp, POST_FLAG_WATER);
		case 'd':
			return delete_attachment(tl, fp);
		case 'a':
			return attachment_search_author(tl, fp, false);
		case 'A':
			return attachment_search_author(tl, fp, true);
		case '/':
			return attachment_search_title(tl, false);
		case '?':
			return attachment_search_title(tl, true);
		case Ctrl('A'):
			return t_query(fp->owner);
		case 'P': case Ctrl('B'): case KEY_PGUP:
			return tui_list_seek(tl, KEY_PGUP, true, true);
		case 'k': case KEY_UP:
			if (tl->in_query) {
				if (tl->cur == 0) {
					tl->in_query = false;
					return FULLUPDATE;
				}
				return READ_AGAIN;
			}
			return tui_list_seek(tl, KEY_UP, true, true);
		case 'N': case Ctrl('F'): case KEY_PGDN: case ' ':
			return tui_list_seek(tl, KEY_PGDN, true, true);
		case 'j': case KEY_DOWN:
			if (tl->in_query) {
				if (tl->cur == tl->all - 1) {
					tl->in_query = false;
					return FULLUPDATE;
				}
				return READ_AGAIN;
			}
			return tui_list_seek(tl, KEY_DOWN, true, true);
		case '$': case KEY_END:
			tl->cur = tl->all - 1;
			if (tl->cur < 0)
				tl->cur = 0;
			if (tl->begin + tl->lines <= tl->cur) {
				tl->begin = tl->cur - tl->lines + 1;
				if (tl->begin < 0)
					tl->begin = 0;
			}
			tl->valid = false;
			return PARTUPDATE;
		case KEY_HOME:
			tl->begin = tl->cur = 0;
			tl->valid = false;
			return PARTUPDATE;
		case 'O':
			return tui_follow_uname(fp->owner);
		default:
			return READ_AGAIN;
	}
}

int tui_attachment_list(const board_t *board)
{
	record_t record;
	if (attachment_open_record(board->name, RECORD_READ, &record) < 0)
		return DONOTHING;

	int lines = screen_lines() - 4;
	struct fileheader *buf = malloc(lines * sizeof(*buf));

	tui_attachment_list_t tal = {
		.board_id = board->id,
		.bname = board->name,
		.record = &record,
		.buf = buf,
		.size = 0,
	};

	tui_list_t tl = {
		.data = &tal,
		.loader = tui_attachment_loader,
		.title = tui_attachment_title,
		.display = tui_attachment_display,
		.handler = tui_attachment_handler,
		.query = tui_attachment_query,
	};

	tui_list(&tl);

	record_close(&record);
	free(buf);
	return FULLUPDATE;
}
