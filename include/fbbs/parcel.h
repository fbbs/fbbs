#ifndef FB_PARCEL_H
#define FB_PARCEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	size_t size;
	size_t capacity;
	unsigned char *ptr;
	bool error;
} parcel_t;

extern void parcel_new(parcel_t *parcel);
extern void parcel_write_varuint64(parcel_t *parcel, uint64_t val);
extern void parcel_write_varint(parcel_t *parcel, int32_t val);
extern void parcel_write_varint64(parcel_t *parcel, int64_t val);
extern void parcel_write_string(parcel_t *parcel, const char *str, size_t size);
extern void parcel_write_bool(parcel_t *parcel, bool val);
extern void parcel_write_int(parcel_t *parcel, int32_t val);
extern void parcel_write_int64(parcel_t *parcel, int64_t val);

extern void parcel_read_new(const char *ptr, size_t size, parcel_t *parcel);
extern uint64_t parcel_read_varuint64(parcel_t *parcel);
extern int32_t parcel_read_varint(parcel_t *parcel);
extern int64_t parcel_read_varint64(parcel_t *parcel);
extern const char *parcel_read_string_and_size(parcel_t *parcel, size_t *size);
extern const char *parcel_read_string(parcel_t *parcel);
extern bool parcel_read_bool(parcel_t *parcel);
extern int32_t parcel_read_int(parcel_t *parcel);
extern int64_t parcel_read_int64(parcel_t *parcel);

bool parcel_error(const parcel_t *parcel);

#endif // FB_PARCEL_H
