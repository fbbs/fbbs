#include <errno.h>
#include <string.h>
#include "fbbs/convert.h"

/**
 * Open a conversion descriptor.
 * @param cp The conversion descriptor.
 * @param to Output codeset.
 * @param from Input codeset.
 * @return 0 on success, -1 on error.
 */
int convert_open(convert_t *cp, const char *to, const char *from)
{
	cp->cd = iconv_open(to, from);
	if (cp->cd == (iconv_t)-1)
		return -1;
	return 0;
}

/**
 * Set conversion state to initial state.
 * @param cp The conversion descriptor.
 */
void convert_reset(convert_t *cp)
{
	iconv(cp->cd, NULL, NULL, NULL, NULL);
}

/**
 * Convert string.
 * @param cp The conversion descriptor.
 * @param from Input string.
 * @param len Length of the input string. If zero, the length will be
 *            counted automatically.
 * @param buf If not NULL, it will be used instead of internal buffer.
 * @param size Size of buf.
 * @param handler Function to handle the converted bits. If it returns a
 *                negative number, the conversion will stop.
 *                If handler is NULL, output buffer will not be reused.
 * @param arg Argument for handler.
 * @return 0 on success, negative on handler failure, input bytes converted
 *         if the input string ends with an imcomplete multibyte sequence.
 */
int convert(convert_t *cp, const char *from, size_t len,
		char *buf, size_t size, convert_handler_t handler, void *arg)
{
	if (len == 0)
		len = strlen(from);

	char *f = (char *)from;
	size_t l = len;

	char *buffer = buf ? buf : cp->buf;
	size = buf ? size : sizeof(cp->buf);

	char *b;
	size_t oleft;
	int ret = 0;
	for (b = buffer, oleft = size; l > 0; ) {
		if (handler) {
			b = buffer;
			oleft = size;
		}

		size_t s = iconv(cp->cd, &f, &l, &b, &oleft);
		buffer[size - oleft] = '\0';

		if (s == (size_t) -1) {
			switch (errno) {
				case E2BIG:
					if (!handler)
						return 0;
					break;
				case EILSEQ:
					++f;
					--l;
					if (handler)
						(*handler)("?", 1, arg);
					else
						;
					break;
				case EINVAL:
					ret = len - l;
					l = 0;
					break;
				default:
					break;
			}
		}

		if (handler && oleft < size) {
			s = (*handler)(buffer, size - oleft, arg);
			if (s < 0)
				return ret;
		}
	}
	return ret;
}

/**
 * Close a conversion descriptor.
 * @param cp The conversion descriptor.
 */
int convert_close(convert_t *cp)
{
	return iconv_close(cp->cd);
}
