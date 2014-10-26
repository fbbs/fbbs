#include <wchar.h>
#include "bbs.h"
#include "mmap.h"
#include "fbbs/helper.h"
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
	text_line_size_t buffer_pos; ///< 当前光标所在行的缓冲区位置
	text_line_size_t screen_pos; ///< 当前光标所在屏幕列
	text_line_size_t request_pos; ///< 上下移动时记录光标所在屏幕列
	char input_buffer[6]; ///< 输入缓冲区, 等待宽字符
	char pending_bytes; ///< 输入缓冲区已用字节数
	bool redraw; ///< 是否重绘整个屏幕
} editor_t;

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

	if (!fit) {
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
 * 将字符串加入编辑器行缓冲区末尾
 * @param editor 编辑器
 * @param begin 字符串起始指针
 * @param end 字符串尾端指针
 * @return 操作成功与否
 */
static bool text_line_append(editor_t *editor, const char *begin,
		const char *end)
{
	text_line_t *tl = vector_grow(&editor->lines, 1);
	return tl ? text_line_make(editor, tl, begin, end) : false;
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

	for (const char *ptr = begin; ptr < end; ) {
		const char *line_end = get_newline(ptr, end);
		text_line_append(editor, ptr, line_end);
		ptr = line_end;
	}

	if (!utf8)
		free((void *) begin);
	mmap_close(&m);

	char buf[80];
	snprintf(buf, sizeof(buf), ":·%s %s·", BBSNAME_UTF8, BBSHOST);
	size_t size = strlen(buf);

	editor->allow_edit_end = vector_size(&editor->lines);
	if (editor->allow_edit_end)
		--editor->allow_edit_end;
	for (vector_size_t i = editor->allow_edit_end; i; --i) {
		text_line_t *tl = editor_line(editor, i);
		if (tl && match_string(tl, buf, size)) {
			editor->allow_edit_end = i;
			break;
		}
	}
	return true;
}

static bool editor_init(editor_t *editor, const char *file, bool utf8,
		bool allow_modify_header)
{
	editor->memory = 0;

	if (!vector_init(&editor->lines, sizeof(text_line_t), 0)
			|| !read_file(editor, file, utf8))
		return false;

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
	editor->buffer_pos = editor->screen_pos = editor->request_pos = 0;
	editor->pending_bytes = 0;
	editor->redraw = true;
	return true;
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
				const char *last = tl->buf, *end = tl->buf + tl->size;
				for (const char *ptr = tl->buf; ptr < end; ++ptr) {
					if (*ptr == '\033') {
						screen_puts(last, ptr - last);
						screen_puts("\033[33m*\033[m", 0);
						last = ptr + 1;
					}
				}
				if (end > last) {
					screen_puts(last, end - last);
				}
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

	move(editor->current_line - editor->window_top, editor->screen_pos);
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
 * 检查指定行是否含有换行符
 * @param tl 行数据结构
 * @return 指定行是否含有换行符
 */
static bool contains_newline(const text_line_t *tl)
{
	return tl->size && tl->buf[tl->size - 1] == '\n';
}

/**
 * 从编辑器当前光标所在行开始自动折行
 * @param editor 编辑器
 */
static void wrap_long_lines(editor_t *editor)
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
			++editor->allow_edit_end;
		}

		++editor->current_line;
		editor->buffer_pos = 0;
		if (raw_insert(editor, p, tl->size - (p - tl->buf)))
			tl->size = p - tl->buf;
		else
			break;
	}
	editor->current_line = saved_line;
	editor->buffer_pos = saved_pos;

	text_line_t *tl = editor_current_line(editor);
	editor->screen_pos = display_width(tl->buf, saved_pos);
}

static void wrap_short_lines(editor_t *editor)
{
	vector_size_t saved_line = editor->current_line;
	text_line_size_t saved_pos = editor->buffer_pos;

	text_line_t *tl = editor_current_line(editor);
	while (!contains_newline(tl)) {
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
		}
	}
	editor->current_line = saved_line;
	editor->buffer_pos = saved_pos;
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

	text_line_t *tl = editor_current_line(editor);
	bool ok = raw_insert(editor, buf, s);
	if (ok) {
		tl->redraw = true;
		editor->screen_pos += display_width_wchar(wc);
		editor->request_pos = editor->screen_pos;
		wrap_long_lines(editor);
	}
}

/**
 * 在当前位置插入一个制表符
 * @param 编辑器
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
	if (editor->buffer_pos >= tl->size)
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

static void request_redraw(editor_t *editor)
{
	editor->redraw = true;
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
	if (editor->screen_pos) {
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

	if (base < lines)
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
		editor->request_pos = editor->buffer_pos = tl->size - 1;
		editor->screen_pos = display_width(tl->buf, tl->size);
	}
}

static void move_file_begin(editor_t *editor)
{
	editor->current_line = editor->allow_edit_begin;
	move_line_begin(editor);
}

static void move_file_end(editor_t *editor)
{
	if (editor->allow_edit_begin < editor->allow_edit_end) {
		editor->current_line = editor->allow_edit_end - 1;
		move_line_begin(editor);
	}
}

static void delete_char_forward(editor_t *editor)
{
	text_line_t *tl = editor_current_line(editor);
	if (!tl)
		return;

	char *begin = tl->buf + editor->buffer_pos, *end = tl->buf + tl->size;
	char *ptr = (char *) string_next_utf8_start(begin, end);
	if (ptr > end || (ptr == end && contains_newline(tl)))
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
			request_redraw(editor);
			break;
		case Ctrl('Q'):
			show_help("help/edithelp");
			request_redraw(editor);
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
			// TODO
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
			break;
		case Ctrl('F'): case KEY_PGDN:
			move_down(editor, screen_lines() - 2);
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

static bool write_body(const editor_t *editor, FILE *fp, const board_t *board,
		const char *host, bool anonymous)
{
	vector_size_t lines = vector_size(&editor->lines);
	for (vector_size_t i = editor->allow_edit_begin; i < lines; ++i) {
		text_line_t *tl = editor_line(editor, i);
		if (tl && tl->buf) {
			if (!strneq2(tl->buf, "\033[m\033[1;36m※ 修改:·"))
				fwrite(tl->buf, 1, tl->size, fp);
		}
	}

	if (session_status() == ST_EDIT) {
		fprintf(fp, "\033[m\033[1;36m※ 修改:·%s 于 %s·[FROM: %s]\033[m\n",
				currentuser.userid,
				format_time(fb_time(), TIME_FORMAT_UTF8_ZH), host);
	} else {
		char fname[HOMELEN];
		setuserfile(fname, "signatures");
		if (!dashf(fname) || currentuser.signature == 0 || anonymous)
			fputs("--\n", fp);

		int color = (currentuser.numlogins % 7) + 31;
		fprintf(fp, "\033[m\033[1;%2dm※ 来源:·%s %s·[FROM: %s]\033[m\n",
				color, BBSNAME_UTF8, BBSHOST,
				anonymous ? ANONYMOUS_SOURCE_UTF8 : host);
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

	bool anonymous = post_header->anonymous
			&& (board.flag & BOARD_FLAG_ANONY);

	UTF8_BUFFER(host, IP_LEN / 2);
	convert_g2u(mask_host(fromhost), utf8_host);

	bool ok = true;
	if (write_header_to_file) {
		ok = write_header_utf8(fp, post_header, &board, utf8_host, anonymous,
				in_mail);
	}
	if (ok)
		write_body(editor, fp, &board, utf8_host, anonymous);

	fclose(fp);
	if (!utf8) {
		ok = false;
		fp = fopen(file, "w");
		if (fp) {
			mmap_t m = { .oflag = O_RDONLY };
			if (mmap_open(temp, &m) == 0) {
				convert_to_file(CONVERT_U2G, m.ptr, m.size, fp);
				mmap_close(&m);
				ok = true;
			}
			fclose(fp);
		}
	}
	return ok;
}

static tui_edit_e confirm_save_file(const char *file,
		struct postheader *post_header, bool confirmed)
{
	char ans[2];
	if (confirmed) {
		*ans = 'S';
	} else {
		screen_move_clear(-1);
		tui_input(-1, "S) 发表 A) 取消 T) 更改标题 E) 再编辑 [S]: ",
				ans, sizeof(ans), true);
		*ans = toupper(*ans);
	}

	if (*ans == 'S') {
		return TUI_EDIT_SAVED;
	} else if (*ans == 'A') {
		struct stat st;
		if (stat(file, &st) || st.st_size == 0)
			unlink(file);
		return TUI_EDIT_ABORTED;
	} else if (*ans == 'T') {
		// TODO
	}
	return TUI_EDIT_CONTINUE;
}

/**
 * 编辑文件
 * @param file 要编辑的文件
 * @param utf8 文件编码
 * @param write_header_to_file 是否加上信头
 * @param allow_modify_header 是否允许修改信头部分
 * @param post_header 信头数据结构
 * @return 用户确认保存返回TUI_EDIT_SAVED, 否则TUI_EDIT_ABORTED
 */
tui_edit_e tui_edit(const char *file, bool utf8, bool write_header_to_file,
		bool allow_modify_header, struct postheader *post_header)
{
	tui_edit_e ans = TUI_EDIT_CONTINUE;

	editor_t editor;
	if (!editor_init(&editor, file, utf8, allow_modify_header)) {
		ans = TUI_EDIT_ABORTED;
		goto e;
	}

	screen_clear();
	display(&editor, false);
//  status_bar

	wchar_t wc;
	bool confirmed = false;
	while (ans == TUI_EDIT_CONTINUE
			&& (wc = get_next_wchar(&editor)) != WEOF) {
		switch (wc) {
			case Ctrl('X'):
				confirmed = true;
				// fall through
			case Ctrl('W'): {
				ans = confirm_save_file(file, post_header, confirmed);
				if (ans == TUI_EDIT_SAVED) {
					write_file(&editor, file, post_header, utf8,
							write_header_to_file);
				}
				break;
			}
			case KEY_ESC:
				// TODO
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

	for (int i = vector_size(&editor.lines); i > 0; ) {
		text_line_t *tl = editor_line(&editor, --i);
		editor_free(&editor, tl);
	}
e:	vector_free(&editor.lines);
	return ans;
}
