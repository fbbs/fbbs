#ifndef FB_VECTOR_H
#define FB_VECTOR_H

#include <stdbool.h>

typedef unsigned int vector_size_t;

typedef struct {
	vector_size_t len;
	vector_size_t size;
	vector_size_t capacity;
	void *data;
} vector_t;

extern bool vector_init(vector_t *v, vector_size_t len, vector_size_t capacity);
extern bool vector_reserve(vector_t *v, vector_size_t capacity);
extern void *vector_grow(vector_t *v);
extern void vector_free(vector_t *v);

#endif // FB_VECTOR_H
