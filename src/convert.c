#include "bbs.h"

#ifdef ALLOWSWITCHCODE

#define B2G_TABLE "etc/b2g_table"  ///< BIG5 to GB2312 table.
#define G2B_TABLE "etc/g2b_table"  ///< GB2312 to BIG5 table.

enum {
	G2B_COUNT = 7614,  ///< Count of characters in g2b table.
	B2G_COUNT = 13973, ///< Count of characters in b2g table.
};

extern void redoscr();

int convcode = 0;  ///< Whether to convert between BIG5 and GB2312.
static unsigned char *g2b;  ///< the starting address of mapped g2b table.
static unsigned char *b2g;  ///< the starting address of mapped b2g table.

/**
 * Revert convert option and redraw screen.
 * @return Current convert option.
 */
int switch_code(void)
{
	convcode = !convcode;
	redoscr();
	return convcode;
}

/**
 * Resolve GBK BIG5 mapping tables.
 * @param file file to resolve
 * @param start start address to copy to
 * @param count count of characters (16 bits each)
 * @param d1 default first byte
 * @param d2 default second byte
 */
static void do_resolve_gbkbig5_table(const char *file, unsigned char *start,
		int count, int d1, int d2)
{
	int i;
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0) {
		start = (unsigned char *)start;
		for (i = 0; i < count; ++i) {
			*start++ = d1;
			*start++ = d2;
		}
	} else {
		memcpy(start, m.ptr, count * 2);
		mmap_close(&m);
	}
}

/**
 * Resolve GBK BIG5 mapping tables.
 * @return 0 on success, -1 on error.
 */
int resolve_gbkbig5_table(void)
{
	b2g = attach_shm("CONV_SHMKEY", 3013, G2B_COUNT * 2 + B2G_COUNT * 2);
	if (b2g == NULL)
		return -1;
	do_resolve_gbkbig5_table(B2G_TABLE, b2g, B2G_COUNT, 0xA1, 0xF5);
	g2b = b2g + B2G_COUNT * 2;
	do_resolve_gbkbig5_table(G2B_TABLE, g2b, G2B_COUNT, 0xA1, 0xBC);
	return 0;
}

enum {
	CONVERT_BEG,
	CONVERT_GOT,
	CONVERT_STR,
};

/**
 * Convert GB2312 bytes to BIG5 bytes.
 * @param ch byte to convert. If ch < 0, test if there is second byte to get.
 * @return the first byte of converted value. successive call will return
 *         the second byte. original byte on error, -1 indicates a second
 *         byte is needed.
 */
int convert_g2b(int ch)
{
	static int status = CONVERT_BEG, got, second;
	int locate;

	if (status != CONVERT_STR && ch < 0)
		return -1;
	switch (status) {
		case CONVERT_BEG:
			if (ch < 0xA1 || (ch > 0xA9 && ch < 0xB0) || ch > 0xF7) {
				return ch;
			} else {
				got = ch;
				status = CONVERT_GOT;
				return -1;
			}
		case CONVERT_GOT:
			status = CONVERT_STR;
			if (ch < 0xA0 || ch == 0xFF) {
				second = ch;
				return got;
			}
			if (got > 0xA0 && got < 0xAA)
				locate = ((got - 0xA1) * 94 + (ch - 0xA1)) * 2;
			else
				locate = ((got - 0xB0 + 9) * 94 + (ch - 0xA1)) * 2;
			second = g2b[locate + 1];
			return g2b[locate];
		case CONVERT_STR:
			status = CONVERT_BEG;
			return second;
		default:
			return -1;
	}
}

/**
 * Convert BIG5 bytes to GBK bytes.
 * @param ch byte to convert. if ch < 0, test if there is second byte to get.
 * @return the first byte of converted value. successive call will return
 *         the second byte. original byte on error, -1 indicates a second
 *         byte is needed.
 */
int convert_b2g(int ch)
{
	static int status = CONVERT_BEG, got, second;
	int locate;

	if (status != CONVERT_STR && ch < 0)
		return -1;
	switch (status) {
		case CONVERT_BEG:
			if (ch < 0xA1 || ch == 0xFF) {
				return ch;
			} else {
				got = ch;
				status = CONVERT_GOT;
				return -1;
			}
		case CONVERT_GOT:
			status = CONVERT_STR;
			if (ch >= 0x40 && ch <= 0x7E)
				locate = ((got - 0xA1) * 157 + (ch - 0x40)) * 2;
			else if (ch >= 0xA1 && ch <= 0xFE)
				locate = ((got - 0xA1) * 157 + (ch - 0xA1) + 63) * 2;
			else {
				second = ch;
				return got;
			}
			second = b2g[locate + 1];
			return b2g[locate];
		case CONVERT_STR:
			status = CONVERT_BEG;
			return second;
		default:
			return -1;
	}
}

#endif // ALLOWSWITCHCODE
