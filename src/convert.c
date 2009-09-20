#include "bbs.h"

#ifdef ALLOWSWITCHCODE

#define B2G_TABLE "etc/b2g_table"
#define G2B_TABLE "etc/g2b_table"

enum {
	G2B_COUNT = 7614,  ///< Count of characters in g2b table
	B2G_COUNT = 13973, ///< Count of characters in b2g table
};

extern void redoscr();

int convcode = 0;
static unsigned char *g2b, *b2g;

/**
 * Revert convert option and redraw screen.
 */
void switch_code(void)
{
	convcode = !convcode;
	redoscr();
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

//	将str字符串中的GB码汉字转换成相应的BIG5码汉字,并调用write函数输出
int write2(int port, char *str, int len)
{
	register int i, locate;
	register unsigned char ch1, ch2, *ptr;

	for(i=0, ptr=str; i < len;i++) {
		ch1 = (ptr+i)[0];
		if(ch1 < 0xA1 || (ch1> 0xA9 && ch1 < 0xB0) || ch1> 0xF7)
		continue;
		ch2 = (ptr+i)[1];
		i ++;
		if(ch2 < 0xA0 || ch2 == 0xFF )
		continue;
		if((ch1> 0xA0) && (ch1 < 0xAA)) //01～09区为符号数字
		locate = ((ch1 - 0xA1)*94 + (ch2 - 0xA1))*2;
		else //if((buf > 0xAF) && (buf < 0xF8)){ //16～87区为汉字
		locate = ((ch1 - 0xB0 + 9)*94 + (ch2 - 0xA1))*2;
		(ptr+i-1)[0] = g2b[locate++];
		(ptr+i-1)[1] = g2b[locate];
	}
	return write(port, str, len);
}

enum {
	CONVERT_BEG,
	CONVERT_GOT,
	CONVERT_STR,
};

/**
 * Convert big5 bytes to gbk bytes.
 * @param ch byte to convert. if ch < 0, test if there is second byte to 
 *        get.
 * @return the first byte of converted value, successive call will return 
 *         the second byte. -1 on error.
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
			if (ch >= 0x40 && ch <= 0x7E)
				locate = ((got - 0xA1) * 157 + (ch - 0x40)) * 2;
			else if (ch >= 0xA1 && ch <= 0xFE)
				locate = ((got - 0xA1) * 157 + (ch - 0xA1) + 63) * 2;
			else {
				status = CONVERT_BEG;
				return -1;
			}
			status = CONVERT_STR;
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
