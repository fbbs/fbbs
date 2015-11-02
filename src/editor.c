#include <wchar.h>
#include "bbs.h"
#include "mmap.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/util.h"
#include "fbbs/vector.h"

/** 为行缓冲区偏移量定义的整型 */
typedef uint16_t text_line_size_t;

enum {
	TEXT_LINE_SIZE_MAX = UINT16_MAX, ///< 行缓冲区的最大字节数
	WRAP_MARGIN = 77, ///< 自动换行宽度
	TAB_STOP = 4, ///< 制表符对其位置
	/** 编辑器最大允许使用的内存字节数, 包括行缓冲区, 但不考虑内存碎片）*/
	MEMORY_MAX = 2 * 1024 * 1024,
	SEARCH_TEXT_CCHARS = 10,
};

typedef struct {
	char *buf; ///< 缓冲区
	text_line_size_t size; ///< 已用字节数
	text_line_size_t capacity; ///< 缓冲区大小
	bool redraw; ///< 是否需要重绘
} text_line_t;

typedef struct {
	vector_t lines; ///< 显示行的数组
	size_t memory; ///< 当前内存用量
	vector_size_t allow_edit_begin; ///< 允许编辑的起始行
	vector_size_t allow_edit_end; ///< 允许编辑的末尾行(不含此行)
	vector_size_t window_top; ///< 当前窗口起始行
	vector_size_t current_line; ///< 当前光标所在行
	vector_size_t mark_begin; ///< 标记头部
	vector_size_t mark_end; ///< 标记尾部
	text_line_size_t buffer_pos; ///< 当前光标所在行的缓冲区位置
	text_line_size_t screen_pos; ///< 当前光标所在屏幕列
	text_line_size_t request_pos; ///< 上下移动时记录光标所在屏幕列
	UTF8_BUFFER(search_text, SEARCH_TEXT_CCHARS) ; ///< 最后一次搜索的字符串
	char input_buffer[6]; ///< 输入缓冲区, 等待宽字符
	char pending_bytes; ///< 输入缓冲区已用字节数
	bool redraw; ///< 是否重绘整个屏幕
	bool hide_status_line; ///< 是否隐藏状态栏
} editor_t;

static editor_t *current_editor = NULL;

/**
 * 获取编辑器中指定行
 * @param[in] editor 编辑器
 * @param[in] line 行数
 * @return 指定行的数据结构
 */
static inline text_line_t *editor_line(const editor_t *editor,
		vector_size_t line)
{
	return vector_at(&editor->lines, line);
}

/**
 * 获取编辑器的当前行
 * @param[in] editor 编辑器
 * @return 当前行的数据结构
 */
static inline text_line_t *editor_current_line(const editor_t *editor)
{
	return vector_at(&editor->lines, editor->current_line);
}

/**
 * 向编辑器申请一块内存
 * @param editor 编辑器
 * @param size 申请的内存大小
 * @return 申请成功返回指针, 申请失败或者达到编辑器内存限制返回NULL
 */
static void *editor_allocate(editor_t *editor, size_t size)
{
	if (size >= MEMORY_MAX || editor->memory > MEMORY_MAX - size)
		return NULL;
	void *ptr = malloc(size);
	if (ptr)
		editor->memory += size;
	return ptr;
}

/**
 * 修改编辑器分配的行缓冲区大小
 * @param editor 编辑器
 * @param tl 行数据结构
 * @param new_size 内存块的新大小
 * @return 修改成功返回true, 申请失败或者达到编辑器内存限制返回false
 */
static bool editor_reallocate(editor_t *editor, text_line_t *tl,
		size_t new_size)
{
	if (!editor || !tl)
		return false;

	if (new_size > tl->capacity) {
		size_t diff = new_size - tl->capacity;
		if (diff >= MEMORY_MAX || editor->memory > MEMORY_MAX - diff)
			return false;
	}

	void *new_ptr = realloc(tl->buf, new_size);
	if (new_ptr) {
		if (new_size > tl->capacity)
			editor->memory += new_size - tl->capacity;
		else
			editor->memory -= tl->capacity - new_size;
		tl->capacity = new_size;
		tl->buf = new_ptr;
	}
	return new_ptr;
}

/**
 * 释放先前由编辑器分配的一块内存
 * @param editor 编辑器
 * @param tl 行数据结构
 */
static void editor_free(editor_t *editor, text_line_t *tl)
{
	if (tl) {
		free(tl->buf);
		editor->memory -= tl->capacity;
	}
}

/**
 * 新分配一行缓冲区, 或者调整已分配缓冲区的大小
 * @param editor 编辑器
 * @param tl 行数据结构, 必须已经分配并反映现状.
 *           视tl->capacity决定新分配或者调整已分配
 * @param capacity 缓冲区大小
 * @param fit 为true, 则申请多少就分配多少, 否则可能多分配一些
 * @return 分配成功与否
 */
static bool text_line_allocate(editor_t *editor,
		text_line_t *tl, text_line_size_t capacity, bool fit)
{
	if (!editor || !tl)
		return false;

	if (!fit && capacity) {
		text_line_size_t c = (text_line_size_t) -1;
		while (c / 2 >= capacity)
			c /= 2;
		capacity = c;
	}
	if (capacity < 2)
		capacity = 2;

	if (tl->capacity) {
		return capacity == tl->capacity
			|| editor_reallocate(editor, tl, capacity);
	}

	tl->buf = editor_allocate(editor, capacity);
	if (tl->buf) {
		tl->size = 0;
		tl->capacity = capacity;
		tl->redraw = true;
	}
	return tl->buf;
}

/**
 * 根据字符串初始化一行缓冲区
 * @param editor 编辑器
 * @param tl 已分配的行数据结构
 * @param begin 字符串起始指针
 * @param end 字符串尾端指针
 * @return 初始化成功与否
 */
static bool text_line_make(editor_t *editor, text_line_t *tl,
		const char *begin, const char *end)
{
	if (!tl || end <= begin || end > begin + TEXT_LINE_SIZE_MAX)
		return false;

	tl->buf = NULL;
	tl->size = 0;
	tl->capacity = 0;
	tl->redraw = 0;

	if (!text_line_allocate(editor, tl, end - begin, true))
		return false;

	tl->size = end - begin;
	memcpy(tl->buf, begin, tl->size);
	return true;
}

/**
 * 检查行缓冲区是否含有指定字符串
 * @param tl 行缓冲区
 * @param str 字符串起始指针
 * @param size 字符串长度
 * @return 行缓冲区是否含有指定字符串
 */
static bool match_string(const text_line_t *tl, const char *str, size_t size)
{
	if (tl->size < size)
		return false;

	size_t s = tl->size - size;
	for (int i = 0; i < s; ++i) {
		if (tl->buf[i] == str[0] && memcmp(tl->buf + i, str, size) == 0)
			return true;
	}
	return false;
}

static const char *get_newline(const char *begin, const char *end)
{
	while (begin < end) {
		if (*begin++ == '\n')
			return begin;
	}
	return begin;
}

static vector_size_t editor_end(const editor_t *editor)
{
	char buf[80];
	snprintf(buf, sizeof(buf), ":·%s %s·", BBSNAME_UTF8, BBSHOST);
	size_t size = strlen(buf);

	vector_size_t end = vector_size(&editor->lines);
	for (vector_size_t i = end ? end - 1 : 0; i; --i) {
		text_line_t *tl = editor_line(editor, i);
		if (tl && match_string(tl, buf, size)) {
			return i;
		}
	}
	return end;
}

/**
 * 检查指定行是否含有换行符
 * @param tl 行数据结构
 * @return 指定行是否含有换行符
 */
static bool contains_newline(const text_line_t *tl)
{
	return tl->size && tl->buf[tl->size - 1] == '\n';
}

/**
 * 计算一个宽字符的显示宽度
 * @param wc 一个宽字符
 * @return 该宽字符在编辑器终端上的显示宽度
 */
static size_t display_width_wchar(wchar_t wc)
{
	if (wc == L'\n')
		return 0;
	if (wc == L'\033')
		return 1;
	int width = fb_wcwidth(wc);
	return width < 1 ? 1 : width;
}

/**
 * 计算字符串的显示宽度
 * @param str UTF-8编码字符串
 * @param size 字符串的长度
 * @return 字符串在编辑器终端上的显示宽度
 */
static size_t display_width(const char *str, size_t size)
{
	size_t width = 0;
	for (wchar_t wc = next_wchar(&str, &size); wc && wc != WEOF; ) {
		width += display_width_wchar(wc);
		wc = next_wchar(&str, &size);
	}
	return width;
}

static void real_pos(const editor_t *editor, vector_size_t *x,
		text_line_size_t *y)
{
	*x = 1;
	for (vector_size_t i = editor->allow_edit_begin;
			i < editor->current_line; ++i) {
		if (contains_newline(editor_line(editor, i)))
			++*x;
	}

	*y = editor->screen_pos + 1;
	if (editor->current_line) {
		for (vector_size_t i = editor->current_line - 1;
				i >= editor->allow_edit_begin;) {
			const text_line_t *tl = editor_line(editor, i);
			if (contains_newline(tl))
				break;
			else
				*y += display_width(tl->buf, tl->size);
			if (i)
				--i;
			else
				break;
		}
	}
}

static void show_status_line(const editor_t *editor)
{
	screen_move_clear(-1);
	if (editor->hide_status_line)
		return;

	bool mail = chkmail();

	vector_size_t x;
	text_line_size_t y;
	real_pos(editor, &x, &y);

	screen_printf("\033[1;33;44m[%s] [\033[32m%s\033[33m]"
			" [\033[32m按\033[31mCtrl-Q\033[32m求救\033[33m]"
			" [\033[32m%d\033[33m,\033[32m%d\033[33m]\033[K\033[m",
			mail ? "\033[5;32m信\033[m\033[1;33;44m" : "  ",
			format_time(fb_time(), TIME_FORMAT_UTF8_ZH) + 7,
			x, y);
}

static void display(editor_t *editor, bool ansi)
{
	int line = 0, lines = screen_lines() - 1;
	for (vector_size_t i = editor->window_top;
			i < editor->allow_edit_end && line < lines;
			++i, ++line) {
		text_line_t *tl = editor_line(editor, i);
		if (editor->redraw || tl->redraw) {
			screen_move_clear(line);
			if (ansi) {
				screen_puts(tl->buf, tl->size);
			} else {
				bool marked = i >= editor->mark_begin && i < editor->mark_end;
				if (marked)
					screen_puts("\033[7m", 4);
				const char *last = tl->buf, *end = tl->buf + tl->size;
				for (const char *ptr = tl->buf; ptr < end; ++ptr) {
					if (*ptr == '\033') {
						if (ptr > last)
							screen_puts(last, ptr - last);
						screen_puts("\033[33m*", 0);
						if (marked)
							screen_puts("\033[37m", 0);
						else
							screen_puts("\033[m", 0);
						last = ptr + 1;
					}
				}
				if (end > last) {
					size_t size = end - last - contains_newline(tl);
					if (size)
						screen_puts(last, size);
				}
				if (marked)
					screen_puts("\033[m", 3);
			}
			tl->redraw = false;
		}
	}

	if (editor->redraw) {
		for (; line < lines; ++line) {
			screen_move_clear(line);
			screen_puts("\033[33m~\033[m", 0);
		}
	}

	show_status_line(editor);

	screen_move(editor->current_line - editor->window_top, editor->screen_pos);
	editor->redraw = false;
	return;
}

static wchar_t get_next_wchar(editor_t *editor)
{
	const char *buffer = NULL;
	size_t bytes = 0;
	char utf8_buffer[8] = { '\0' };

	int ch = terminal_getchar();
	editor->input_buffer[(int) editor->pending_bytes++] = ch;

	if (terminal_is_utf8()) {
		buffer = editor->input_buffer;
		bytes = editor->pending_bytes;
	} else {
		if (editor->pending_bytes > 1) {
			convert(CONVERT_G2U, editor->input_buffer, editor->pending_bytes,
					utf8_buffer, sizeof(utf8_buffer), NULL, NULL);
			editor->pending_bytes = 0;
			if (utf8_buffer[0] != '\0') {
				buffer = utf8_buffer;
				bytes = strlen(utf8_buffer);
			}
		} else if (!(ch & 0x80)) {
			editor->pending_bytes = 0;
			return ch;
		}
	}

	if (buffer) {
		wchar_t wc = next_wchar(&buffer, &bytes);
		if ((wc && wc != WEOF)
				|| editor->pending_bytes == sizeof(editor->input_buffer)) {
			editor->pending_bytes = 0;
			return wc;
		}
	}
	return 0;
}

static const char *seek_to_display_width(const char *str, size_t size,
		size_t width)
{
	const char *last = str;
	for (wchar_t wc = next_wchar(&str, &size); wc && wc != WEOF; ) {
		size_t w = display_width_wchar(wc);
		if (w > width) {
			return last;
		} else {
			last = str;
			width -= w;
		}
		wc = next_wchar(&str, &size);
	}
	return last;
}

#define fb_overflow(type, type_max, op1, op2) \
	(type_max - (type) (op1) < (type) (op2))

#define fb_add(type, op1, op2)  ((type) (op1) + (type) (op2))

/**
 * 向编辑器当前位置插入字符串
 * @param editor 编辑器
 * @param str 字符串起始指针
 * @param len 字符串长度
 * @return 操作成功与否
 */
static bool raw_insert(editor_t *editor, const char *str,
		text_line_size_t len)
{
	text_line_t *tl = editor_current_line(editor);

	if (fb_overflow(text_line_size_t, TEXT_LINE_SIZE_MAX, tl->size, len))
		return false;

	if (!text_line_allocate(editor, tl,
				fb_add(text_line_size_t, tl->size, len), false)) {
		return false;
	}

	text_line_size_t pos = editor->buffer_pos;
	memmove(tl->buf + pos + len, tl->buf + pos, tl->size - pos);
	memcpy(tl->buf + pos, str, len);
	tl->size += len;
	editor->buffer_pos += len;
	return true;
}

/**
 * 从编辑器当前光标所在行开始自动折行
 * @param editor 编辑器
 * @return 折行次数
 */
static int wrap_long_lines(editor_t *editor)
{
	vector_size_t saved_line, line;
	saved_line = line = editor->current_line;
	text_line_size_t saved_pos = editor->buffer_pos;

	while (1) {
		text_line_t *tl = editor_line(editor, line);
		const char *p = seek_to_display_width(tl->buf, tl->size, WRAP_MARGIN);
		if (p >= tl->buf + tl->size)
			break;

		editor->redraw = true;

		// 如果当前光标被折行，记录下新位置
		if (saved_line == line && p <= tl->buf + saved_pos) {
			saved_pos -= p - tl->buf;
			saved_line = line + 1;
		}

		++line;
		text_line_t *next_tl = editor_line(editor, line);
		if (contains_newline(tl) || !next_tl
				|| line >= editor->allow_edit_end) {
			text_line_t new_tl = { .buf = NULL };
			vector_insert(&editor->lines, line, &new_tl);
			tl = editor_line(editor, line - 1);
			++editor->allow_edit_end;
		}

		++editor->current_line;
		editor->buffer_pos = 0;
		if (raw_insert(editor, p, tl->size - (p - tl->buf)))
			tl->size = p - tl->buf;
		else
			break;
	}
	int diff = editor->current_line - saved_line;
	editor->current_line = saved_line;
	editor->buffer_pos = saved_pos;

	if (editor->current_line >= editor->window_top + screen_lines() - 1)
		editor->window_top = editor->current_line - screen_lines() + 2;

	text_line_t *tl = editor_current_line(editor);
	editor->screen_pos = display_width(tl->buf, saved_pos);
	return diff;
}

static void wrap_short_lines(editor_t *editor)
{
	vector_size_t saved_line = editor->current_line;
	text_line_size_t saved_pos = editor->buffer_pos;

	text_line_t *tl = editor_current_line(editor);
	while (tl && !contains_newline(tl)) {
		vector_size_t line = editor->current_line;
		size_t width = display_width(tl->buf, tl->size);
		if (width < WRAP_MARGIN && line + 1 < editor->allow_edit_end) {
			editor->redraw = true;

			text_line_t *next_tl = editor_line(editor, line + 1);
			const char *ptr = seek_to_display_width(next_tl->buf,
					next_tl->size, WRAP_MARGIN - width);

			editor->buffer_pos = tl->size;
			if (raw_insert(editor, next_tl->buf, ptr - next_tl->buf)) {
				memmove(next_tl->buf, ptr, next_tl->buf + next_tl->size - ptr);
				next_tl->size -= ptr - next_tl->buf;

				++editor->current_line;
				tl = editor_current_line(editor);

				if (!tl->size) {
					editor_free(editor, tl);
					vector_erase(&editor->lines, editor->current_line);
					--editor->allow_edit_end;
				}
			} else {
				break;
			}
		} else {
			break;
		}
	}
	editor->current_line = saved_line;
	editor->buffer_pos = saved_pos;
}

static void mark_clear(editor_t *editor)
{
	if (editor->mark_begin || editor->mark_end)
		editor->redraw = true;
	editor->mark_begin = editor->mark_end = 0;
}

static void insert_string(editor_t *editor, const char *str,
		text_line_size_t len)
{
	text_line_t *tl = editor_current_line(editor);
	bool ok = raw_insert(editor, str, len);
	if (ok) {
		tl->redraw = true;
		wrap_long_lines(editor);
		editor->request_pos = editor->screen_pos;
	}
	mark_clear(editor);
}

/**
 * 在当前位置插入一个宽字符
 * @param editor 编辑器
 * @param wc 要插入的一个宽字符
 */
static void insert_char(editor_t *editor, wchar_t wc)
{
	char buf[5];
	const wchar_t ws[] = { wc, L'\0' };
	size_t s = fb_wcstombs(buf, ws, 4);
	if (s == (size_t) -1)
		return;
	insert_string(editor, buf, s);
}

/**
 * 在当前位置插入一个制表符
 * @param editor 编辑器
 */
static void insert_tab(editor_t *editor)
{
	int spaces = TAB_STOP - editor->screen_pos % TAB_STOP;
	for (int i = 0; i < spaces; ++i) {
		insert_char(editor, ' ');
	}
}

static void split_line(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (editor->buffer_pos > tl->size)
		return;

	text_line_t new_tl = { .capacity = 0 };
	if (text_line_allocate(editor, &new_tl, tl->size - editor->buffer_pos,
			contains_newline(tl))) {
		new_tl.size = tl->size - editor->buffer_pos;
		memcpy(new_tl.buf, tl->buf + editor->buffer_pos, new_tl.size);

		tl->buf[editor->buffer_pos] = '\n';
		tl->size = editor->buffer_pos + 1;

		++editor->current_line;
		editor->screen_pos = editor->buffer_pos = 0;

		vector_insert(&editor->lines, editor->current_line, &new_tl);
		++editor->allow_edit_end;
		wrap_short_lines(editor);

		editor->redraw = true;
	}
}

static void move_left(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (!editor->screen_pos) {
		if (editor->current_line > editor->allow_edit_begin) {
			--editor->current_line;
			tl = editor_current_line(editor);
			editor->buffer_pos = tl->size;
			editor->screen_pos = display_width(tl->buf, tl->size);
			if (editor->current_line < editor->window_top) {
				editor->window_top = editor->current_line;
				editor->redraw = true;
			}
		}
	}
	if (editor->buffer_pos) {
		const char *ptr = string_previous_utf8_start(tl->buf,
				tl->buf + editor->buffer_pos);
		editor->screen_pos -= display_width(ptr,
				tl->buf + editor->buffer_pos - ptr);
		editor->buffer_pos = ptr - tl->buf;
	}
	editor->request_pos = editor->screen_pos;
}

static void move_right(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (tl) {
		const char *ptr = tl->buf + editor->buffer_pos;
		const char *end = tl->buf + tl->size;
		const char *pos = string_next_utf8_start(ptr, end);
		if (pos < end) {
			editor->screen_pos += display_width(ptr, pos - ptr);
			editor->buffer_pos = pos - tl->buf;
		} else {
			if (editor->current_line + 1 < editor->allow_edit_end) {
				++editor->current_line;
				editor->buffer_pos = editor->screen_pos = 0;
				if (editor->current_line - editor->window_top
						>= screen_lines() - 1) {
					++editor->window_top;
					editor->redraw = true;
				}
			}
		}
	}
	editor->request_pos = editor->screen_pos;
}

static void try_request_pos(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	const char *ptr = seek_to_display_width(tl->buf, tl->size,
			editor->request_pos);
	editor->buffer_pos = ptr - tl->buf;
	if (contains_newline(tl) && editor->buffer_pos == tl->size)
		--editor->buffer_pos;
	editor->screen_pos = display_width(tl->buf, editor->buffer_pos);
}

static vector_size_t _move_up(editor_t *editor, vector_size_t base,
		vector_size_t lines)
{
	if (base >= lines)
		base -= lines;
	else
		base = 0;

	if (base < editor->allow_edit_begin)
		base = editor->allow_edit_begin;

	return base;
}

static void move_up(editor_t *editor, vector_size_t lines)
{
	editor->current_line = _move_up(editor, editor->current_line, lines);
	if (editor->current_line < editor->window_top) {
		editor->window_top = _move_up(editor, editor->window_top, lines);
		editor->redraw = true;
	}
	try_request_pos(editor);
}

static vector_size_t _move_down(editor_t *editor, vector_size_t base,
		vector_size_t lines)
{
	if (editor->allow_edit_end - base > lines)
		return base + lines;
	return editor->allow_edit_end - 1;
}

static void move_down(editor_t *editor, vector_size_t lines)
{
	editor->current_line = _move_down(editor, editor->current_line, lines);
	if (editor->current_line >= editor->window_top + screen_lines() - 1) {
		editor->window_top = _move_down(editor, editor->window_top, lines);
		editor->redraw = true;
	}
	try_request_pos(editor);
}

static void move_line_begin(editor_t *editor)
{
	editor->request_pos = editor->buffer_pos = editor->screen_pos = 0;
}

static void move_line_end(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (tl) {
		editor->request_pos = editor->buffer_pos
				= tl->size - contains_newline(tl);
		editor->screen_pos = display_width(tl->buf, tl->size);
	}
}

static void move_file_begin(editor_t *editor)
{
	editor->window_top = editor->current_line = editor->allow_edit_begin;
	move_line_begin(editor);
	editor->redraw = true;
}

static void move_file_end(editor_t *editor)
{
	if (editor->allow_edit_begin < editor->allow_edit_end) {
		editor->current_line = editor->allow_edit_end - 1;
		editor->window_top = editor->current_line - screen_lines() + 2;
		if (editor->window_top < editor->allow_edit_begin)
			editor->window_top = editor->allow_edit_begin;
		move_line_begin(editor);
		editor->redraw = true;
	}
}

static void delete_char_forward(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (!tl)
		return;

	mark_clear(editor);

	if (editor->buffer_pos >= tl->size) {
		vector_size_t saved_line = editor->current_line;
		text_line_size_t saved_pos = editor->buffer_pos;

		++editor->current_line;
		editor->buffer_pos = 0;
		delete_char_forward(editor);

		editor->current_line = saved_line;
		editor->buffer_pos = saved_pos;
		return;
	}

	char *begin = tl->buf + editor->buffer_pos, *end = tl->buf + tl->size;
	char *ptr = (char *) string_next_utf8_start(begin, end);
	if (ptr > end)
		return;

	memmove(begin, ptr, end - ptr);
	tl->size -= ptr - begin;
	tl->redraw = true;

	wrap_short_lines(editor);
}

static void delete_char_backward(editor_t *editor)
{
	if (editor->buffer_pos > 0
			|| editor->current_line > editor->allow_edit_begin) {
		move_left(editor);
		delete_char_forward(editor);
	}
}

static void delete_to_end_of_line(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (tl) {
		tl->size = editor->buffer_pos;
		wrap_short_lines(editor);
	}
	mark_clear(editor);
}

static void mark_selection(editor_t *editor)
{
	if (editor->mark_begin >= editor->mark_end) {
		editor->mark_begin = editor->current_line;
		editor->mark_end = editor->mark_begin + 1;
	} else {
		if (editor->current_line < editor->mark_begin) {
			editor->mark_begin = editor->current_line;
		} else if (editor->current_line >= editor->mark_end) {
			editor->mark_end = editor->current_line + 1;
		} else {
			editor->mark_begin = editor->current_line;
			editor->mark_end = editor->mark_begin + 1;
		}
	}
	editor->redraw = true;
}

static void delete_selection(editor_t *editor)
{
	if (editor->mark_begin < editor->allow_edit_begin)
		editor->mark_begin = editor->allow_edit_begin;
	if (editor->mark_end > editor->allow_edit_end)
		editor->mark_end = editor->allow_edit_end;

	if (editor->mark_end <= editor->mark_begin) {
		editor->mark_begin = editor->mark_end = 0;
	}

	for (vector_size_t i = editor->mark_begin; i < editor->mark_end; ++i) {
		text_line_t *tl = editor_line(editor, i);
		editor_free(editor, tl);
	}
	vector_erase_range(&editor->lines, editor->mark_begin, editor->mark_end);

	vector_size_t diff = editor->mark_end - editor->mark_begin;
	if (editor->current_line >= editor->mark_end) {
		editor->current_line -= diff;
	} else if (editor->current_line >= editor->mark_begin) {
		editor->current_line = editor->mark_begin > 0 ?
				editor->mark_begin - 1 : 0;
	}
	editor->allow_edit_end -= diff;
	if (editor->current_line < editor->window_top)
		editor->window_top = editor->current_line;
	editor->buffer_pos = editor->screen_pos = editor->request_pos = 0;
	editor->mark_begin = editor->mark_end = 0;
	editor->redraw = true;
}

static void jump_to_line(editor_t *editor)
{
	char buf[7];
	tui_input(-1, "请问要跳到第几行: ", buf, sizeof(buf));
	vector_size_t line = strtol(buf, NULL, 10);
	if (!line)
		return;

	vector_size_t real_line = editor->allow_edit_begin;
	--line;
	while (line && real_line < editor->allow_edit_end) {
		if (contains_newline(editor_line(editor, real_line)))
			--line;
		++real_line;
	}

	if (real_line >= editor->allow_edit_end) {
		show_status_line(editor);
	} else {
		editor->current_line = real_line;
		if (real_line < editor->window_top
				|| real_line >= editor->window_top + screen_lines() - 1) {
			editor->window_top = real_line;
			editor->redraw = true;
		}
		editor->buffer_pos = editor->screen_pos = editor->request_pos = 0;
	}
}

static bool _search_text(editor_t *editor, vector_size_t *x, vector_size_t *y)
{
	const char *text = editor->utf8_search_text;
	size_t len = strlen(text);
	for (vector_size_t i = editor->current_line, j = editor->buffer_pos;
			i >= editor->allow_edit_begin && i < editor->allow_edit_end;
			++i, j = 0) {
		const text_line_t *tl = editor_line(editor, i);
		const char *ptr = tl->buf + j, *end = tl->buf + tl->size;
		while (ptr < end && (ptr = memchr(ptr, text[0], end - ptr))) {
			if (end - ptr >= len) {
				if (memcmp(ptr, text, len) == 0) {
					*x = i;
					*y = ptr - tl->buf + len;
					return true;
				}
			} else if (!contains_newline(tl)
					&& memcmp(ptr, text, end - ptr) == 0
					&& i + 1 < editor->allow_edit_end) {
				text_line_t *tl2 = editor_line(editor, i + 1);
				size_t compared = end - ptr, remain = len - compared;
				if (tl2->size >= remain
						&& memcmp(tl2->buf, text + compared, remain) == 0) {
					*x = i + 1;
					*y = remain;
					return true;
				}
			}
			++ptr;
		}
	}
	return false;
}

static void search_text(editor_t *editor, bool repeat_last_search)
{
	if (!repeat_last_search) {
		GBK_BUFFER(text, SEARCH_TEXT_CCHARS);
		tui_input(-1, "搜寻字串: ", gbk_text, sizeof(gbk_text));
		convert_g2u(gbk_text, editor->utf8_search_text);
	}
	if (editor->utf8_search_text[0] == '\0')
		return;

	vector_size_t x, y;
	if (!_search_text(editor, &x, &y))
		return;

	editor->current_line = x;
	editor->request_pos = editor->buffer_pos = y;
	const text_line_t *tl = editor_current_line(editor);
	editor->screen_pos = display_width(tl->buf, y);
	if (editor->current_line >= editor->window_top + screen_lines() - 1
			|| editor->current_line < editor->window_top) {
		editor->window_top = editor->current_line;
		editor->redraw = true;
	}
}

static void print(editor_t *editor, vector_size_t *old_line,
		text_line_size_t *old_pos, vector_size_t new_line,
		vector_size_t new_pos)
{
	for (vector_size_t i = *old_line; i <= new_line; ++i, *old_pos = 0) {
		text_line_t *tl = editor_line(editor, i);
		if (i < new_line) {
			if (tl->size > *old_pos)
				screen_puts(tl->buf + *old_pos, tl->size - *old_pos);
		} else {
			if (new_pos >= tl->size)
				new_pos = tl->size;
			if (i == *old_line) {
				if (new_pos > *old_pos)
					screen_puts(tl->buf + *old_pos, new_pos - *old_pos);
			} else {
				screen_puts(tl->buf, new_pos);
			}
		}
	}
	*old_line = new_line;
	*old_pos = new_pos;
}

static void preview(editor_t *editor)
{
	vector_size_t current = editor->window_top;
	while (current > editor->allow_edit_begin) {
		text_line_t *tl = editor_line(editor, current - 1);
		if (!contains_newline(tl))
			--current;
		else
			break;
	}

	screen_clear();
	int y = screen_lines() - 1, x = 0;
	const int line_width = 80;
	bool in_esc = false, end = false;
	vector_size_t line = current;
	text_line_size_t pos = 0;
	const char *code = "[0123456789;";
	const int len = strlen(code);
	for (vector_size_t i = current; !end && i < editor->allow_edit_end; ++i) {
		text_line_t *tl = editor_line(editor, i);
		const char *ptr = tl->buf, *old_ptr = tl->buf;
		size_t size = tl->size;
		wchar_t wc;
		while ((wc = next_wchar(&ptr, &size)) && wc != WEOF) {
			if (in_esc) {
				if (wc >= 0x80 || !memchr(code, wc, len)) {
					in_esc = false;
				}
			} else {
				if (wc == '\033') {
					in_esc = true;
				} else if (wc == '\n') {
					print(editor, &line, &pos, i, ptr - tl->buf);
					--y;
					x = 0;
				} else {
					int w = fb_wcwidth(wc);
					if (x + w == line_width) {
						print(editor, &line, &pos, i, ptr - tl->buf);
					} else if (x + w > line_width) {
						print(editor, &line, &pos, i, old_ptr - tl->buf);
						screen_puts("\n", 1);
						x = w;
						--y;
					} else {
						x += w;
					}
				}
				if (y <= 0) {
					end = true;
					break;
				}
			}
			old_ptr = ptr;
		}
	}
	screen_move_clear(-1);
	screen_printf("\033[m\033[1m已显示彩色编辑成果，请按任意键继续...\033[m");
	terminal_getchar();
	editor->redraw = true;
}

static bool read_file(editor_t *editor, const char *file, bool utf8)
{
	mmap_t m = { .oflag = O_RDWR | O_CREAT };
	if (mmap_open(file, &m) < 0)
		return false;

	const char *begin = m.ptr, *end = begin + m.size;
	if (!utf8) {
		size_t len = m.size * 3 / 2 + 1;
		begin = malloc(len);
		if (!begin) {
			mmap_close(&m);
			return false;
		}
		convert(CONVERT_G2U, m.ptr, m.size, (char *) begin, len, NULL, NULL);
		end = begin + strlen(begin);
	}

	bool empty = !vector_size(&editor->lines);
	for (const char *ptr = begin; ptr < end; ) {
		const char *line_end = get_newline(ptr, end);
		bool newline = line_end > ptr && line_end[-1] == '\n';
		if (empty || (!editor->buffer_pos && newline)) {
			text_line_t tl;
			text_line_make(editor, &tl, ptr, line_end);
			vector_insert(&editor->lines, editor->current_line, &tl);
			if (line_end - ptr > WRAP_MARGIN)
				editor->current_line += wrap_long_lines(editor);
			++editor->current_line;
			++editor->allow_edit_end;
		} else {
			raw_insert(editor, ptr, line_end - ptr - newline);
			wrap_long_lines(editor);
			if (newline)
				split_line(editor);
		}
		ptr = line_end;
	}

	if (!utf8)
		free((void *) begin);
	mmap_close(&m);

	return true;
}

static void clip_path(char *buf, size_t size, int index)
{
	snprintf(buf, size, "home/%c/%s/clip_new_%d",
			toupper(currentuser.userid[0]), currentuser.userid, index);
}

static void handle_clip(editor_t *editor, bool import)
{
	char ans[2];
	char prompt[80];
	snprintf(prompt, sizeof(prompt), "%s剪贴簿第几页? (1-8): ",
			import ? "读取" : "写入");
	tui_input(-1, prompt, ans, sizeof(ans));
	if (ans[0] < '1' || ans[0] > '8')
		return;

	int idx = ans[0] - '0';
	char file[HOMELEN];
	clip_path(file, sizeof(file), idx);
	bool success = false;
	if (import) {
		if (dashf(file)) {
			read_file(editor, file, true);
			success = true;
		} else {
			screen_printf("无法取出剪贴簿第 %d 页", idx);
		}
	} else {
		FILE *fp = fopen(file, "w");
		if (fp) {
			int begin = editor->allow_edit_begin;
			int end = editor->allow_edit_end;
			if (editor->mark_end > editor->mark_begin) {
				begin = editor->mark_begin;
				end = editor->mark_end;
			}

			for (int i = begin; i < end; ++i) {
				const text_line_t *tl = editor_line(editor, i);
				if (tl)
					fwrite(tl->buf, 1, tl->size, fp);
			}
			fclose(fp);
			mark_clear(editor);
			success = true;
		}
	}
	screen_move_clear(-1);
	screen_printf("%s%s剪贴簿第 %d 页", success ? "已" : "无法",
			import ? "取出" : "写入", idx);
	terminal_getchar();
	editor->redraw = true;
}

static void choose_color(editor_t *editor, bool foreground)
{
	char prompt[80];
	snprintf(prompt, sizeof(prompt), "选择%s景颜色"
			" 0)黑 1)红 2)绿 3)黄 4)蓝 5)紫 6)靛 7)白: \033[K",
			foreground ? "前" : "背");
	char ans[2];
	tui_input(-1, prompt, ans, sizeof(ans));
	if (ans[0] < '0' || ans[0] > '7')
		return;

	char buf[6];
	snprintf(buf, sizeof(buf), "\033[%d%cm",
			foreground ? 3 : 4, ans[0]);
	text_line_t *tl = editor_current_line(editor);
	tl->redraw = true;
	insert_string(editor, buf, sizeof(buf) - 1);
}

static void handle_edit(editor_t *editor, wchar_t wc)
{
	switch (wc) {
		case KEY_TAB:
			insert_tab(editor);
			break;
		case '\r': case '\n':
			split_line(editor);
			break;
		case Ctrl('G'):
			editor->redraw = true;
			break;
		case Ctrl('Q'):
			show_help("help/edithelp");
			editor->redraw = true;
			break;
		case KEY_LEFT:
			move_left(editor);
			break;
		case KEY_RIGHT:
			move_right(editor);
			break;
		case Ctrl('C'):
			// TODO
			break;
		case Ctrl('U'):
			mark_selection(editor);
			break;
		case Ctrl('V'):
			// TODO
			break;
		case Ctrl('P'): case KEY_UP:
			move_up(editor, 1);
			break;
		case Ctrl('N'): case KEY_DOWN:
			move_down(editor, 1);
			break;
		case Ctrl('B'): case KEY_PGUP:
			move_up(editor, screen_lines() - 2);
			editor->window_top = editor->current_line;
			editor->redraw = true;
			break;
		case Ctrl('F'): case KEY_PGDN:
			move_down(editor, screen_lines() - 2);
			editor->window_top = editor->current_line;
			editor->redraw = true;
			break;
		case Ctrl('A'): case KEY_HOME:
			move_line_begin(editor);
			break;
		case Ctrl('E'): case KEY_END:
			move_line_end(editor);
			break;
		case Ctrl('S'):
			move_file_begin(editor);
			break;
		case Ctrl('T'):
			move_file_end(editor);
			break;
		case Ctrl('O'): case KEY_INS:
			return; // no toggle insert/overwrite
		case Ctrl('H'): case KEY_BACKSPACE:
			delete_char_backward(editor);
			break;
		case Ctrl('D'): case KEY_DEL:
			delete_char_forward(editor);
			break;
		case Ctrl('K'):
			delete_to_end_of_line(editor);
			break;
		case Ctrl('Y'):
			editor->buffer_pos = editor->screen_pos = 0;
			delete_to_end_of_line(editor);
			break;
		default:
			insert_char(editor, wc);
			break;
	}
}

static bool write_header_utf8(FILE *fp, const struct postheader *post_header,
		const board_t *board, const char *host, bool anonymous, bool mail)
{
	char user_name[20];
	strlcpy(user_name, currentuser.userid, sizeof(user_name));

	UTF8_BUFFER(nick, NAMELEN / 2);
	convert_g2u(currentuser.username, utf8_nick);

	if (mail) {
		fprintf(fp, "寄信人: %s (%s)\n", user_name, utf8_nick);
	} else {
		fprintf(fp, "发信人: %s (%s), 信区: %s\n",
				anonymous ? ANONYMOUS_ACCOUNT : user_name,
				anonymous ? ANONYMOUS_NICK_UTF8 : utf8_nick, board->name);
	}

	UTF8_BUFFER(title, sizeof(post_header->title) / 2);
	convert_g2u(post_header->title, utf8_title);
	fprintf(fp, "标  题: %s\n", utf8_title);

	fprintf(fp, "发信站: %s (%s)", BBSNAME_UTF8,
			format_time(fb_time(), TIME_FORMAT_UTF8_ZH));

	if (mail)
		fprintf(fp, "\n来  源: %s\n\n", host);
	else
		fprintf(fp, ", 站内信件\n\n");
	return true;
}

static bool write_body(const editor_t *editor, FILE *fp, const char *host,
		bool anonymous, bool write_header_to_file)
{
	bool ends_with_newline = false;
	vector_size_t lines = vector_size(&editor->lines);
	for (vector_size_t i = 0; i < lines; ++i) {
		text_line_t *tl = editor_line(editor, i);
		if (tl && tl->buf) {
			if (strneq2(tl->buf, "\033[m\033[1;36m※ 修改:·"))
				break;
			fwrite(tl->buf, 1, tl->size, fp);
			ends_with_newline = contains_newline(tl);
		}
	}
	if (!ends_with_newline)
		fputc('\n', fp);

	if (session_status() == ST_EDIT) {
		fprintf(fp, "\033[m\033[1;36m※ 修改:·%s 于 %s·[FROM: %s]\033[m\n",
				currentuser.userid,
				format_time(fb_time(), TIME_FORMAT_UTF8_ZH), host);
	} else if (write_header_to_file) {
		if (editor->allow_edit_end == vector_size(&editor->lines)) {
			int color = (currentuser.numlogins % 7) + 31;
			fprintf(fp, "\033[m\033[1;%2dm※ 来源:·%s %s·[FROM: %s]\033[m\n",
					color, BBSNAME_UTF8, BBSHOST,
					anonymous ? ANONYMOUS_SOURCE_UTF8 : host);
		}
	}
	return true;
}

/**
 * 将编辑内容写入文件
 * @param editor 编辑器
 * @param file 要写入的文件名
 * @param post_header 文件头
 * @param utf8 是否以UTF-8编码输出
 * @param write_header_to_file 是否写入文件头
 * @return 成功与否
 */
static int write_file(editor_t *editor, const char *file,
		const struct postheader *post_header, bool utf8,
		bool write_header_to_file)
{
	extern char fromhost[];

	FILE *fp;
	char temp[HOMELEN];
	if (utf8) {
		fp = fopen(file, "w");
	} else {
		file_temp_name(temp, sizeof(temp));
		fp = fopen(temp, "w+");
	}

	if (!fp)
		return false;

	board_t board;
	if (post_header && post_header->postboard
			&& !strcaseeq(currboard, post_header->ds)) {
		get_board(post_header->ds, &board);
	} else {
		memcpy(&board, currbp, sizeof(board));
	}

	bool anonymous = post_header && post_header->anonymous
			&& (board.flag & BOARD_FLAG_ANONY);

	UTF8_BUFFER(host, IP_LEN / 2);
	convert_g2u(mask_host(fromhost), utf8_host);

	bool ok = true;
	if (write_header_to_file) {
		ok = write_header_utf8(fp, post_header, &board, utf8_host, anonymous,
				in_mail);
	}
	if (ok)
		write_body(editor, fp, utf8_host, anonymous, write_header_to_file);

	fclose(fp);
	if (!utf8) {
		ok = convert_file(temp, file, CONVERT_U2G);
		unlink(temp);
	}
	return ok;
}

static editor_e confirm_save_file(const char *file,
		struct postheader *post_header, bool confirmed)
{
	char ans[2] = { '\0' };
	if (confirmed) {
		*ans = 'S';
	} else {
		screen_move_clear(-1);
		tui_input_no_clear(-1, "S) 发表 A) 取消 E) 再编辑 [S]: ",
				ans, sizeof(ans));
		*ans = toupper(*ans);
		if (*ans == '\0')
			*ans = 'S';
	}

	if (*ans == 'S') {
		return EDITOR_SAVE;
	} else if (*ans == 'A') {
		struct stat st;
		if (stat(file, &st) || st.st_size == 0)
			unlink(file);
		return EDITOR_ABORT;
	}
	return EDITOR_CONTINUE;
}

static void handle_esc(editor_t *editor)
{
	switch (KEY_ESC_arg) {
		case 'Q': case 'q':
			mark_clear(editor);
			break;
		case KEY_ESC:
			insert_char(editor, '\033');
			break;
		case 'd':
			delete_selection(editor);
			break;
		case 'g':
			jump_to_line(editor);
			break;
		case 'l':
			editor->hide_status_line = !editor->hide_status_line;
			show_status_line(editor);
			break;
		case 's':
			search_text(editor, false);
			break;
		case 'n':
			search_text(editor, true);
			break;
		case 'c':
			preview(editor);
			break;
		case 'i':
			handle_clip(editor, true);
			break;
		case 'e':
			handle_clip(editor, false);
			break;
		case 'b':
			choose_color(editor, false);
			break;
		case 'f':
			choose_color(editor, true);
			break;
		case 'r':
			insert_string(editor, "\033[m", 3);
			break;
		default:
			break;
	}
}

static bool editor_init(editor_t *editor, const char *file, bool utf8,
		bool allow_modify_header)
{
	editor->memory = 0;
	editor->current_line = 0;
	editor->buffer_pos = editor->screen_pos = editor->request_pos = 0;
	editor->mark_begin = editor->mark_end = 0;
	editor->pending_bytes = 0;

	if (!vector_init(&editor->lines, sizeof(text_line_t), 0)
			|| !read_file(editor, file, utf8))
		return false;

	if (!vector_size(&editor->lines)) {
		text_line_t tl = {
			.buf = NULL, .capacity = 0, .size = 0, .redraw = false
		};
		text_line_allocate(editor, &tl, 0, false);
		vector_insert(&editor->lines, 0, &tl);
	}

	editor->allow_edit_end = editor_end(editor);
	editor->allow_edit_begin = 0;
	if (!allow_modify_header) {
		for (vector_size_t i = 0; i < editor->allow_edit_end; ++i) {
			text_line_t *tl = editor_line(editor, i);
			// 找到一个"\n"
			if (tl->size == 1 && tl->buf[0] == '\n') {
				if (i + 1 < editor->allow_edit_end)
					++i;
				editor->allow_edit_begin = i;
				break;
			}
		}
	}
	editor->current_line = editor->window_top = editor->allow_edit_begin;
	editor->redraw = true;
	return true;
}

/**
 * 编辑文件
 * @param file 要编辑的文件
 * @param utf8 文件编码
 * @param write_header_to_file 是否加上信头
 * @param allow_modify_header 是否允许修改信头部分
 * @param post_header 信头数据结构
 * @return 用户确认保存返回EDITOR_SAVE, 否则EDITOR_ABORT
 */
editor_e editor(const char *file, bool utf8, bool write_header_to_file,
		bool allow_modify_header, struct postheader *post_header)
{
	editor_e ans = EDITOR_CONTINUE;

	editor_t editor;
	if (!editor_init(&editor, file, utf8, allow_modify_header)) {
		ans = EDITOR_ABORT;
		goto e;
	}
	current_editor = &editor;

	screen_clear();
	display(&editor, false);
//  status_bar

	wchar_t wc;
	bool confirmed = false;
	while (ans == EDITOR_CONTINUE
			&& (wc = get_next_wchar(&editor)) != WEOF) {
		switch (wc) {
			case Ctrl('X'):
				confirmed = true;
				// fall through
			case Ctrl('W'): {
				ans = confirm_save_file(file, post_header, confirmed);
				if (ans == EDITOR_SAVE) {
					if (!write_file(&editor, file, post_header, utf8,
							write_header_to_file))
						ans = EDITOR_ABORT;
				}
				break;
			}
			case KEY_ESC:
				handle_esc(&editor);
				break;
			case 0:
				break;
			default:
				handle_edit(&editor, wc);
		}

		if (terminal_input_buffer_empty()) {
			display(&editor, false);
		}
	}

	current_editor = NULL;

	for (int i = vector_size(&editor.lines); i > 0; ) {
		text_line_t *tl = editor_line(&editor, --i);
		editor_free(&editor, tl);
	}
e:	vector_free(&editor.lines);
	return ans;
}

static void editor_dump_path(char *buf, size_t size)
{
	snprintf(buf, size, "home/%c/%s/editor_dump",
			toupper(currentuser.userid[0]), currentuser.userid);
}

void editor_dump(void)
{
	if (current_editor) {
		char file[HOMELEN];
		editor_dump_path(file, sizeof(file));
		write_file(current_editor, file, NULL, true, false);
	}
}

void editor_restore(void)
{
	char file[HOMELEN];
	editor_dump_path(file, sizeof(file));

	if (streq(currentuser.userid, "guest") || !dashf(file))
		return;

	char ans[2];
	tui_input(-1, "您有一个编辑作业不正常中断 (S) 写入暂存档 (M) 寄回信箱"
			" (Q) 算了 [M]: ", ans, sizeof(ans));
	switch (toupper(ans[0])) {
		case 'Q':
			unlink(file);
			break;
		case 'S':
			while (1) {
				char ans2[2];
				tui_input(-1, "请选择暂存档 (1-8) [1]: ", ans2, sizeof(ans2));
				if (ans2[0] == '\0')
					ans2[0] = '1';
				if (ans2[0] >= '1' && ans2[0] <= '8') {
					int index = ans2[0] - '0';
					char buf[HOMELEN];
					clip_path(buf, sizeof(buf), index);
					if (dashf(buf)) {
						tui_input(-1, "暂存档已存在 (O)覆盖 (A)附加 [O]: ",
								ans2, sizeof(ans2));
						switch (toupper(ans2[0])) {
							case 'A':
								f_cp(file, buf, O_APPEND);
								unlink(file);
								break;
							default:
								unlink(buf);
								rename(file, buf);
								break;
						}
					} else {
						rename(file, buf);
					}
					break;
				}
			}
			break;
		default: {
			char temp[HOMELEN];
			file_temp_name(temp, sizeof(temp));
			if (convert_file(file, temp, CONVERT_G2U)) {
				mail_file(temp, currentuser.userid,
						//% 不正常断线所保留的部份...
						"\xb2\xbb\xd5\xfd\xb3\xa3\xb6\xcf\xcf\xdf\xcb\xf9"
						"\xb1\xa3\xc1\xf4\xb5\xc4\xb2\xbf\xb7\xdd...");
				unlink(temp);
			}
			unlink(file);
			break;
		}
	}
}
