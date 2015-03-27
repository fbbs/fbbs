#include <errno.h>
#include <iconv.h>
#include <string.h>
#include "mmap.h"
#include "fbbs/convert.h"
#include "fbbs/string.h"

#ifdef LINUX
# define fb_iconv(cd, i, ib, o, ob)  iconv(cd, (char **)i, ib, o, ob)
#else
# define fb_iconv(cd, i, ib, o, ob) iconv(cd, i, ib, o, ob)
#endif

static iconv_t _u2g = (iconv_t) -1;
static iconv_t _g2u = (iconv_t) -1;

/**
 * 打开编码转换描述符
 * @param type 编码转换类型
 * @return 成功返回true, 否则false
 */
bool convert_open(convert_type_e type)
{
	if (type == CONVERT_U2G) {
		_u2g = iconv_open("GBK", "UTF-8");
		return _u2g != (iconv_t) -1;
	} else {
		_g2u = iconv_open("UTF-8", "GBK");
		return _g2u != (iconv_t) -1;
	}
}

/**
 * 重置编码转换状态
 * @param cd 编码转换描述符
 */
static void convert_reset(iconv_t cd)
{
	iconv(cd, NULL, NULL, NULL, NULL);
}

static iconv_t convert_descriptor(convert_type_e type)
{
	if (type == CONVERT_U2G)
		return _u2g;
	if (type == CONVERT_G2U)
		return _g2u;
	return (iconv_t) -1;
}

/**
 * 转换字符串编码
 * @param type 编码转换类型
 * @param from 要转换的字符串. NULL会被看成空字符串
 * @param len 输入字符串的长度. CONVERT_ALL表示自动计算长度
 * @param buf 输出缓冲区. 如果是NULL, 将使用内部缓冲区
 * @param size 输出缓冲区大小
 * @param handler 处理转换结果的回调函数. 如果其返回负数, 转换即停止.
 *                如果是NULL, 则输出缓冲区必须足够长.
 * @param arg 传给回调函数的参数
 * @return 成功转换全部输入字符串返回0, 否则返回已转换的字节数
 */
int convert(convert_type_e type, const char *from, size_t len,
		char *buf, size_t size, convert_handler_t handler, void *arg)
{
	iconv_t cd = convert_descriptor(type);
	convert_reset(cd);

	char internal_buffer[1024];

	if (len == (size_t) -1)
		len = from ? strlen(from) : 0;

	const char *f = from;
	size_t l = len;

	char *buffer = buf ? buf : internal_buffer;
	size = buf ? size - 1: sizeof(internal_buffer) - 1;

	if (!l) {
		buffer[0] = '\0';
		return 0;
	}

	char *b;
	size_t oleft;
	int ret = 0;
	for (b = buffer, oleft = size; l; ) {
		if (handler) {
			b = buffer;
			oleft = size;
		}

		size_t s = fb_iconv(cd, &f, &l, &b, &oleft);
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
						handler("?", 1, arg);
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
			if (handler(buffer, size - oleft, arg) < 0)
				return ret;
		}
	}
	return ret;
}

/**
 * 关闭编码转换描述符
 */
void convert_close(void)
{
	iconv_close(_u2g);
	iconv_close(_g2u);
}

static int write_to_file(const char *buf, size_t len, void *arg)
{
	return fwrite(buf, 1, len, arg) < len ? -1 : 0;
}

int convert_to_file(convert_type_e type, const char *from, size_t len, FILE *fp)
{
	return convert(type, from, len, NULL, 0, write_to_file, fp);
}

bool convert_file(const char *from, const char *to, convert_type_e type)
{
	if (!from || !to)
		return false;

	char temp[80];
	bool ok = false;

	bool inplace = streq(from, to);
	if (inplace) {
		file_temp_name(temp, sizeof(temp));
		to = temp;
	}

	FILE *fp = fopen(to, "w");
	if (!fp)
		return false;

	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(from, &m) != 0) {
		convert_to_file(type, m.ptr, m.size, fp);
		mmap_close(&m);
		ok = true;
	}
	fclose(fp);

	if (inplace) {
		if (ok)
			rename(temp, from);
		else
			unlink(temp);
	}
	return ok;
}
