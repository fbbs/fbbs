#ifndef FB_TERMIO_H
#define FB_TERMIO_H

#include <stdlib.h>
#include <stdbool.h>

#include "fbbs/util.h"

enum {
	IOBUF_SIZE = 4096,
};

enum {
	KEY_TAB = 9,
	KEY_ESC = 27,
	KEY_UP = 0x0101,
	KEY_DOWN = 0x0102,
	KEY_RIGHT = 0x0103,
	KEY_LEFT = 0x0104,
	KEY_HOME = 0x0201,
	KEY_INS = 0x0202,
	KEY_DEL = 0x0203,
	KEY_BACKSPACE = 0x7f,
	KEY_END = 0x0204,
	KEY_PGUP = 0x0205,
	KEY_PGDN = 0x0206,
	KEY_CTRL_A = 1, KEY_CTRL_B, KEY_CTRL_C, KEY_CTRL_D, KEY_CTRL_E,
	KEY_CTRL_F, KEY_CTRL_G, KEY_CTRL_H, KEY_CTRL_I, KEY_CTRL_J, KEY_CTRL_L,
	KEY_CTRL_M, KEY_CTRL_N, KEY_CTRL_O, KEY_CTRL_P, KEY_CTRL_Q, KEY_CTRL_R,
	KEY_CTRL_S, KEY_CTRL_T, KEY_CTRL_U, KEY_CTRL_V, KEY_CTRL_W, KEY_CTRL_X,
	KEY_CTRL_Y, KEY_CTRL_Z,
};

typedef struct iobuf_t {
	size_t cur;
	size_t size;
	uchar_t buf[IOBUF_SIZE];
} iobuf_t;

typedef struct telconn_t {
	int fd;
	bool cr;
	int lines;
	int cols;
	iobuf_t inbuf;
	iobuf_t outbuf;
} telconn_t;

extern void telnet_init(telconn_t *tc, int fd);
extern int telnet_flush(telconn_t *tc);
extern void telnet_write(telconn_t *tc, const uchar_t *str, int size);
extern int telnet_putc(telconn_t *tc, int c);
extern bool buffer_empty(telconn_t *tc);
extern int term_getch(telconn_t *tc);

#endif // FB_TERMIO_H
