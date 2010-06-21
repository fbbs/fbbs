#ifndef FB_SCREEN_H
#define FB_SCREEN_H

#include "fbbs/util.h"
#include "fbbs/termio.h"

enum {
	SCREENLINE_BUFSIZE = 512,
};

typedef struct screen_line_t {
	int oldlen;                        // Length after last flush.
	int len;                           // Length in the buffer.
	bool modified;                     // Modified or not.
	ushort_t smod;                     // Start of modification.
	ushort_t emod;                     // End of modification.
	uchar_t data[SCREENLINE_BUFSIZE];  // The buffer.
} screen_line_t;

typedef struct screen_t {
	int scr_lns;             // Lines of the screen.
	int scr_cols;            // Columns of the screen.
	int cur_ln;              // Current line.
	int cur_col;             // Current column.
	int tc_ln;               // Terminal's current line.
	int tc_col;              // Terminal's current column.
	int roll;                // Offset of first line in the array.
	int scroll_cnt;          // Count of scrolled lines since last update.
	bool show_ansi;          // Print ansi control codes verbatim.
	bool clear;              // Clear whole screen.
	int size;                // Size of screen lines array.
	screen_line_t *lines;    // Screen lines.
	telconn_t *tc;           // Associated telnet connection.
} screen_t;

extern void screen_init(telconn_t *tc, int lines, int cols);
extern void move(int line, int col);
extern void clear(void);
extern void clrtoeol(void);
extern void scroll(void);
extern void outc(int c);
extern void outs(const char *str);
extern void redoscr(void);
extern void refresh(void);
extern int get_screen_width(void);
extern int get_screen_height(void);
extern void prints(const char *fmt, ...);

extern void presskeyfor(const char *msg, int x);
extern void pressanykey(void);

#endif // FB_SCREEN_H
