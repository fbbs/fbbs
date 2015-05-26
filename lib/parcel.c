#include <stdlib.h>
#include <string.h>
#include "fbbs/fileio.h"
#include "fbbs/parcel.h"
#include "fbbs/util.h"

enum {
	PARCEL_DEFAULT_CAPACITY = 4096,
};

void parcel_new(parcel_t *parcel)
{
	parcel->size = PARCEL_SIZE_LENGTH;
	parcel->capacity = PARCEL_DEFAULT_CAPACITY;
	parcel->error = false;
	parcel->ptr = malloc(parcel->capacity);
}

void parcel_free(parcel_t *parcel)
{
	free(parcel->ptr);
}

parcel_size_t parcel_size(const parcel_t *parcel)
{
	return parcel->size;
}

void parcel_clear(parcel_t *parcel)
{
	parcel->size = 0;
}

static void parcel_write(parcel_t *parcel, const void *ptr, parcel_size_t size)
{
	if (parcel->error)
		return;

	parcel_size_t new_size = parcel->size + size;
	if (parcel->capacity < new_size) {
		while (parcel->capacity < new_size)
			parcel->capacity *= 2;
		parcel->ptr = realloc(parcel->ptr, parcel->capacity);
	}
	if (parcel->capacity < new_size || !parcel->ptr) {
		parcel->error = true;
		return;
	}

	memcpy(parcel->ptr + parcel->size, ptr, size);
	parcel->size += size;
}

void parcel_write_varuint64(parcel_t *parcel, uint64_t val)
{
	uchar_t buf[10] = { 0 };
	int cur = 0;

	while (1) {
		buf[cur] = val & 0x7f;
		val >>= 7;
		if (val) {
			buf[cur] |= 0x80;
			++cur;
		} else {
			break;
		}
	}
	parcel_write(parcel, buf, ++cur);
}

void parcel_write_varint(parcel_t *parcel, int32_t val)
{
	uint32_t v = (val << 1) ^ (val >> 31);
	parcel_write_varuint64(parcel, v);
}

void parcel_write_varint64(parcel_t *parcel, int64_t val)
{
	parcel_write_varuint64(parcel, (val << 1) ^ (val >> 63));
}

void parcel_write_string_with_size(parcel_t *parcel, const char *str,
		parcel_size_t size)
{
	if (str) {
		if (!size)
			size = strlen(str);
		parcel_write_bool(parcel, false);
		parcel_write_varuint64(parcel, size);
		parcel_write(parcel, str, size);
		parcel_write(parcel, "", 1);
	} else {
		parcel_write_bool(parcel, true);
	}
}

void parcel_write_string(parcel_t *parcel, const char *str)
{
	parcel_write_string_with_size(parcel, str, 0);
}

void parcel_write_bool(parcel_t *parcel, bool val)
{
	uchar_t v = val;
	parcel_write(parcel, &v, 1);
}

void parcel_write_int(parcel_t *parcel, int32_t val)
{
	parcel_write(parcel, &val, sizeof(val));
}

void parcel_write_int64(parcel_t *parcel, int64_t val)
{
	parcel_write(parcel, &val, sizeof(val));
}

void parcel_read_new(const char *ptr, parcel_size_t size, parcel_t *parcel)
{
	parcel->size = 0;
	parcel->capacity = size;
	parcel->ptr = (uchar_t *) ptr;
	parcel->error = false;
}

static uchar_t parcel_read_uchar(parcel_t *parcel)
{
	if (parcel->size <= parcel->capacity)
		return parcel->ptr[parcel->size++];
	parcel->error = true;
	return 0;
}

uint64_t parcel_read_varuint64(parcel_t *parcel)
{
	uint64_t val = 0;
	int offset = 0;
	while (1) {
		uint64_t v = parcel_read_uchar(parcel);
		if (v & 0x80) {
			val |= (v & 0x7f) << offset;
			offset += 7;
		} else {
			val |= v << offset;
			break;
		}
	}
	return val;
}

int32_t parcel_read_varint(parcel_t *parcel)
{
	return parcel_read_varint64(parcel);
}

int64_t parcel_read_varint64(parcel_t *parcel)
{
	uint64_t val = parcel_read_varuint64(parcel);
	return (val >> 1) ^ (-(val & 1));
}

const char *parcel_read_string_and_size(parcel_t *parcel, parcel_size_t *size)
{
	bool is_null = parcel_read_bool(parcel);
	if (is_null)
		return NULL;

	*size = parcel_read_varuint64(parcel);
	const char *str = (const char *) (parcel->ptr + parcel->size);
	parcel->size += *size + 1;
	if (parcel->size > parcel->capacity)
		parcel->error = true;
	return str;
}

const char *parcel_read_string(parcel_t *parcel)
{
	parcel_size_t size;
	return parcel_read_string_and_size(parcel, &size);
}

bool parcel_read_bool(parcel_t *parcel)
{
	return parcel_read_uchar(parcel);
}

static void parcel_read(parcel_t *parcel, void *buf, parcel_size_t size)
{
	if (parcel->size + size <= parcel->capacity) {
		memcpy(buf, parcel->ptr + parcel->size, size);
		parcel->size += size;
	} else {
		parcel->error = true;
	}
}

int32_t parcel_read_int(parcel_t *parcel)
{
	int32_t val = 0;
	parcel_read(parcel, &val, sizeof(val));
	return val;
}

int64_t parcel_read_int64(parcel_t *parcel)
{
	int64_t val = 0;
	parcel_read(parcel, &val, sizeof(val));
	return val;
}

bool parcel_ok(const parcel_t *parcel)
{
	return !parcel->error;
}

bool parcel_flush(parcel_t *parcel, int fd)
{
	if (parcel->error || fd < 0)
		return false;
	for (int i = 0; i < PARCEL_SIZE_LENGTH; ++i) {
		parcel->ptr[i] = 0x7f & (parcel->size >> (i * 8));
	}
	return file_write(fd, parcel->ptr, parcel->size) == parcel->size;
}
