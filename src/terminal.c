// Terminal I/O handlers.

#include <arpa/telnet.h>
#include <sys/select.h>
#ifdef ENABLE_SSH
#include "libssh/libssh.h"
#endif // ENABLE_SSH
#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/mail.h"
#include "fbbs/msg.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

enum {
	INPUT_BUFFER_SIZE = 48,
	OUTPUT_BUFFER_SIZE = 4088,
};

typedef struct {
	unsigned char ptr[INPUT_BUFFER_SIZE];
	size_t cur;
	size_t size;
} input_buffer_t;

typedef struct {
	unsigned char ptr[OUTPUT_BUFFER_SIZE];
	size_t size;
} output_buffer_t;

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif
extern struct screenline *big_picture;
#ifdef ENABLE_SSH
extern ssh_channel ssh_chan;
#endif // ENABLE_SSH
extern int msg_num, RMSG;

static input_buffer_t input_buffer;   ///< Input buffer.
static output_buffer_t output_buffer;  ///< Output buffer.

int KEY_ESC_arg;

int terminal_read(unsigned char *buf, size_t size)
{
#ifdef ENABLE_SSH
	return channel_read(ssh_chan, buf, size, 0);
#else // ENABLE_SSH
	return file_read(STDIN_FILENO, buf, size);
#endif // ENABLE_SSH
}

int terminal_write(const unsigned char *buf, size_t len)
{
#ifdef ENABLE_SSH
	return channel_write(ssh_chan, buf, len);
#else // ENABLE_SSH
	return file_write(STDIN_FILENO, buf, len);
#endif // ENABLE_SSH

}

/**
 * Flush output buffer.
 * @return 0 on success, -1 on error.
 */
int terminal_flush(void)
{
	int ret = 0;
	if (output_buffer.size > 0)
		ret = terminal_write(output_buffer.ptr, output_buffer.size);
	output_buffer.size = 0;
	return ret;
}

/**
 * Put a byte into output buffer.
 * @param ch byte to put.
 */
static void put_raw_ch(int ch)
{
	output_buffer.ptr[output_buffer.size++] = ch;
	if (output_buffer.size == sizeof(output_buffer.ptr))
		terminal_flush();
}

/**
 * Put a byte into output buffer. Do translation if needed.
 * @param ch byte to put.
 */
void terminal_putchar(int ch)
{
	ch = (unsigned char)ch;
#ifdef ALLOWSWITCHCODE
	if (convcode) {
		ch = convert_g2b(ch);
		while (ch > 0) {
			put_raw_ch(ch);
			ch = convert_g2b(-1);
		}
	} else {
		put_raw_ch(ch);
	}
#else
	put_raw_ch(ch);
#endif // ALLOWSWITCHCODE
}

/**
 * Put bytes into output buffer.
 * @param str pointer to the first byte.
 * @param size bytes to output.
 * @note IAC is not handled.
 */
void terminal_write_cached(const unsigned char *str, int size)
{
	int convert = 0;
#ifdef ALLOWSWITCHCODE
	convert = convcode;
#endif // ALLOWSWITCHCODE
	if (convert) {
		while (size-- > 0)
			terminal_putchar(*str++);
	} else {
		while (size > 0) {
			int len = sizeof(output_buffer.ptr) - output_buffer.size;
			if (size > len) {
				memcpy(output_buffer.ptr + output_buffer.size, str, len);
				output_buffer.size += len;
				terminal_flush();
				size -= len;
				str += len;
			} else {
				memcpy(output_buffer.ptr + output_buffer.size, str, size);
				output_buffer.size += size;
				return;
			}
		}
	}
}

static int i_newfd = 0;
static struct timeval *i_top = NULL;

bool terminal_input_buffer_empty(void)
{
	return (input_buffer.cur >= input_buffer.size);
}

/**
 * Get raw byte from stdin.
 * @return next byte from stdin
 */
static int get_raw_ch(void)
{
	if (input_buffer.cur >= input_buffer.size) {
		fd_set rset;
		struct timeval to;
		int fd = i_newfd;
		int nfds, ret;

		FD_ZERO(&rset);
		FD_SET(STDIN_FILENO, &rset);
		if (fd) {
			FD_SET(fd, &rset);
			nfds = fd + 1;
		} else {
			nfds = 1;
		}

		session_set_idle_cached();

		to.tv_sec = to.tv_usec = 0;
		ret = select(nfds, &rset, NULL, NULL, &to);
#ifdef ENABLE_SSH
		if (FD_ISSET(STDIN_FILENO, &rset))
			ret = channel_poll(ssh_chan, 0);
#endif
		if (ret <= 0) {
			if (big_picture)
				refresh();
			else
				terminal_flush();

			FD_ZERO(&rset);
			FD_SET(0, &rset);
			if (fd)
				FD_SET(fd, &rset);
			while ((ret = select(nfds, &rset, NULL, NULL, i_top)) < 0) {
				if (errno != EINTR)
					return -1;
			}
			if (ret == 0)
				return I_TIMEOUT;
		}
		if (fd && FD_ISSET(fd, &rset))
			return I_OTHERDATA;

		while (1) {
			ret = terminal_read(input_buffer.ptr, sizeof(input_buffer.ptr));
			if (ret > 0)
				break;
			if ((ret < 0) && (errno == EINTR))
				continue;
			abort_bbs(0);
		}
		input_buffer.cur = 0;
		input_buffer.size = ret;
	}
	return input_buffer.ptr[input_buffer.cur++];
}

/** Telnet option negotiation sequence status. */
typedef enum {
	OPTION_STATUS_NOR,  ///< normal byte
	OPTION_STATUS_IAC,  ///< right after IAC
	OPTION_STATUS_COM,  ///< right after IAC DO/DONT/WILL/WONT
	OPTION_STATUS_SUB,  ///< right after IAC SB
	OPTION_STATUS_SBC,  ///< right after IAC SB [COMMAND]
	OPTION_STATUS_END,  ///< end of an telnet option
} option_status_e;

/**
 * Handle telnet option negotiation.
 * @return the first character after IAC sequence.
 */
static int handle_iac(void)
{
	option_status_e status = OPTION_STATUS_IAC;
	while (status != OPTION_STATUS_END) {
		int ch = get_raw_ch();
		if (ch < 0)
			return ch;
		switch (status) {
			case OPTION_STATUS_IAC:
				if (ch == SB)
					status = OPTION_STATUS_SUB;
				else
					status = OPTION_STATUS_COM;
				break;
			case OPTION_STATUS_COM:
				status = OPTION_STATUS_END;
				break;
			case OPTION_STATUS_SUB:
				if (ch == SE)
					status = OPTION_STATUS_END;
				break;
			default:
				break;
		}	
	}
	return 0;
}

/** ESC process status */
typedef enum {
	ESC_STATUS_BEG,  ///< begin
	ESC_STATUS_CUR,  ///< Cursor keys
	ESC_STATUS_FUN,  ///< Function keys
	ESC_STATUS_ERR,  ///< Parse error
} esc_status_e;

/**
 * Handle ANSI ESC sequences.
 * @return converted key on success, next key on error.
 */
static int handle_esc(void)
{
	esc_status_e status = ESC_STATUS_BEG, ch, last = 0;
	while (1) {
		ch = get_raw_ch();
		if (ch < 0)
			return ch;
		switch (status) {
			case ESC_STATUS_BEG:
				if (ch == '[' || ch == 'O')
					status = ESC_STATUS_CUR;
				else if (ch == '1' || ch == '4')
					status = ESC_STATUS_FUN;
				else {
					KEY_ESC_arg = ch;  // TODO:...
					return KEY_ESC;
				}
				break;
			case ESC_STATUS_CUR:
				if (ch >= 'A' && ch <= 'D')
					return KEY_UP + ch - 'A';
				else if (ch >= '1' && ch <= '6')
					status = ESC_STATUS_FUN;
				else
					status = ESC_STATUS_ERR;
				break;
			case ESC_STATUS_FUN:
				if (ch == '~' && last >= '1' && last <= '6')
					return KEY_HOME + last - '1';
				else
					status = ESC_STATUS_ERR;
				break;
			case ESC_STATUS_ERR:
				return ch;
			default:
				break;
		}
		last = ch;
	}
	return 0;
}

/**
 * Get next byte from stdin, with special byte interpreted.
 * @return next byte from stdin
 */
int igetch(void)
{
	static bool cr = 0;
	int ch;
	while (1) {
		ch = get_raw_ch();
		switch (ch) {
			case IAC:
				handle_iac();
				continue;
			case KEY_ESC:
				ch = handle_esc();
				break;
			case Ctrl('L'):
				redoscr();
				continue;
			case '\r':
				ch = '\n';
				cr = true;
				break;
			case '\n':
				if (cr) {
					cr = false;
					continue;
				}
				break;
			case '\0':
				cr = false;
				continue;
			default:
				cr = false;
#ifdef ALLOWSWITCHCODE
				if (convcode) {
					ch = convert_b2g(ch);
					if (ch >= 0)
						return ch;
				}
#endif // ALLOWSWITCHCODE
				break;
		}
		break;
	}
	return ch;
}

static int do_terminal_getchar(void)
{
	int ch;
#ifdef ALLOWSWITCHCODE
	if (convcode) {
		ch = convert_b2g(-1); // If there is a byte left.
		while (ch < 0)
			ch = igetch();
	} else {
		ch = igetch();
	}
#else
	ch = igetch();
#endif // ALLOWSWITCHCODE

	// Handle messages.
	if (ch == Ctrl('Z')) {
		if (!msg_num)
			RMSG = true;
	}
	return ch;
}

/**
 * Get next byte from stdin in gbk encoding (if conversion is needed).
 */
int terminal_getchar(void)
{
	int ch = do_terminal_getchar();
	while ((RMSG || msg_num) && session_status() != ST_LOCKSCREEN) {
		msg_reply(ch);
		ch = do_terminal_getchar();
	}
	return ch;
}

int egetch(void)
{
	int rval;

	while (1) {
		rval = terminal_getchar();
		if (rval != Ctrl('L'))
			break;
		redoscr();
	}
	return rval;
}
