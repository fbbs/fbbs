// Terminal I/O handlers.

#ifdef AIX
#include <sys/select.h>
#endif
#include <arpa/telnet.h>
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

/** Telnet option negotiation sequence status. */
enum {
	TELST_NOR,  ///< normal byte
	TELST_IAC,  ///< right after IAC
	TELST_COM,  ///< right after IAC DO/DONT/WILL/WONT
	TELST_SUB,  ///< right after IAC SB
	TELST_SBC,  ///< right after IAC SB [COMMAND]
	TELST_END,  ///< end of an telnet option
};

/** ESC process status */
enum {
	ESCST_BEG,  ///< begin
	ESCST_CUR,  ///< Cursor keys
	ESCST_FUN,  ///< Function keys
	ESCST_ERR,  ///< Parse error
};

typedef struct {
	int cur;
	size_t size;
	unsigned char buf[IOBUFSIZE];
} iobuf_t;

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif
extern struct screenline *big_picture;
#ifdef ENABLE_SSH
extern ssh_channel ssh_chan;
#endif // ENABLE_SSH
extern int msg_num, RMSG;

static iobuf_t inbuf;   ///< Input buffer.
static iobuf_t outbuf;  ///< Output buffer.

int KEY_ESC_arg;

/**
 *
 */
int read_stdin(unsigned char *buf, size_t size)
{
#ifdef ENABLE_SSH
	return channel_read(ssh_chan, buf, size, 0);
#else // ENABLE_SSH
	return read(STDIN_FILENO, buf, size);
#endif // ENABLE_SSH
}

/**
 *
 */
int write_stdout(const unsigned char *buf, size_t len)
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
int oflush(void)
{
	int ret = 0;
	if (outbuf.size > 0)
		ret = write_stdout(outbuf.buf, outbuf.size);
	outbuf.size = 0;
	return ret;
}

/**
 * Put a byte into output buffer.
 * @param ch byte to put.
 */
static void put_raw_ch(int ch)
{
	outbuf.buf[outbuf.size++] = ch;
	if (outbuf.size == sizeof(outbuf.buf))
		oflush();
}

/**
 * Put a byte into output buffer. Do translation if needed.
 * @param ch byte to put.
 */
void ochar(int ch)
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
void output(const unsigned char *str, int size)
{
	int convert = 0;
#ifdef ALLOWSWITCHCODE
	convert = convcode;
#endif // ALLOWSWITCHCODE
	if (convert) {
		while (size-- > 0)
			ochar(*str++);
	} else {
		while (size > 0) {
			int len = sizeof(outbuf.buf) - outbuf.size;
			if (size > len) {
				memcpy(outbuf.buf + outbuf.size, str, len);
				outbuf.size += len;
				oflush();
				size -= len;
				str += len;
			} else {
				memcpy(outbuf.buf + outbuf.size, str, size);
				outbuf.size += size;
				return;
			}
		}
	}
}

static int i_newfd = 0;
static struct timeval *i_top = NULL;

bool inbuf_empty(void)
{
	return (inbuf.cur >= inbuf.size);
}

/**
 * Get raw byte from stdin.
 * @return next byte from stdin
 */
static int get_raw_ch(void)
{
	if (inbuf.cur >= inbuf.size) {
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
				oflush();

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
			ret = read_stdin(inbuf.buf, sizeof(inbuf.buf));
			if (ret > 0)
				break;
			if ((ret < 0) && (errno == EINTR))
				continue;
			abort_bbs(0);
		}
		inbuf.cur = 0;
		inbuf.size = ret;
	}
	return inbuf.buf[inbuf.cur++];
}

/**
 * Handle telnet option negotiation.
 * @return the first character after IAC sequence.
 */
static int iac_handler(void)
{
	int status = TELST_IAC;
	while (status != TELST_END) {
		int ch = get_raw_ch();
		if (ch < 0)
			return ch;
		switch (status) {
			case TELST_IAC:
				if (ch == SB)
					status = TELST_SUB;
				else
					status = TELST_COM;
				break;
			case TELST_COM:
				status = TELST_END;
				break;
			case TELST_SUB:
				if (ch == SE)
					status = TELST_END;
				break;
			default:
				break;
		}	
	}
	return 0;
}

/**
 * Handle ANSI ESC sequences.
 * @return converted key on success, next key on error.
 */
static int esc_handler(void)
{
	int status = ESCST_BEG, ch, last = 0;
	while (1) {
		ch = get_raw_ch();
		if (ch < 0)
			return ch;
		switch (status) {
			case ESCST_BEG:
				if (ch == '[' || ch == 'O')
					status = ESCST_CUR;
				else if (ch == '1' || ch == '4')
					status = ESCST_FUN;
				else {
					KEY_ESC_arg = ch;  // TODO:...
					return KEY_ESC;
				}
				break;
			case ESCST_CUR:
				if (ch >= 'A' && ch <= 'D')
					return KEY_UP + ch - 'A';
				else if (ch >= '1' && ch <= '6')
					status = ESCST_FUN;
				else
					status = ESCST_ERR;
				break;
			case ESCST_FUN:
				if (ch == '~' && last >= '1' && last <= '6')
					return KEY_HOME + last - '1';
				else
					status = ESCST_ERR;
				break;
			case ESCST_ERR:
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
				iac_handler();
				continue;
			case KEY_ESC:
				ch = esc_handler();
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

static int do_igetkey(void)
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
int igetkey(void)
{
	int ch = do_igetkey();
	while ((RMSG || msg_num) && session_status() != ST_LOCKSCREEN) {
		msg_reply(ch);
		ch = do_igetkey();
	}
	return ch;
}

int egetch(void)
{
	int rval;

	while (1) {
		rval = igetkey();
		if (rval != Ctrl('L'))
			break;
		redoscr();
	}
	return rval;
}
