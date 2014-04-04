#include <stdlib.h>
#include <string.h>
#include "fbbs/vector.h"

enum {
	VECTOR_DEFAULT_CAPACITY = 8,
};

bool vector_init(vector_t *v, vector_size_t len, vector_size_t capacity)
{
	if (v) {
		v->len = len;
		v->size = 0;
		v->capacity = 0;
		v->data = NULL;
		return vector_reserve(v, capacity);
	}
	return false;
}

static vector_size_t grow_capacity(vector_size_t capacity)
{
	vector_size_t val = VECTOR_DEFAULT_CAPACITY;
	while (val < capacity)
		val *= 2;
	return val;
}

bool vector_reserve(vector_t *v, vector_size_t capacity)
{
	if (v) {
		if (capacity > v->capacity) {
			v->capacity = grow_capacity(capacity);
			v->data = realloc(v->data, v->len * v->capacity);
			return v->data;
		}
		return true;
	}
	return false;
}

vector_size_t vector_size(const vector_t *v)
{
	return v->size;
}

void *vector_at(const vector_t *v, vector_size_t position)
{
	if (v && v->data && position < v->size)
		return (char *) v->data + v->len * position;
	return NULL;
}

static bool _vector_grow(vector_t *v, vector_size_t size)
{
	if (v) {
		v->size += size;
		return vector_reserve(v, v->size);
	}
	return false;
}

void *vector_grow(vector_t *v, vector_size_t size)
{
	if (v && _vector_grow(v, size) && v->size >= size)
		return ((char *) v->data) + v->len * (v->size - size);
	return NULL;
}

bool vector_erase(vector_t *v, vector_size_t position)
{
	return vector_erase_range(v, position, position + 1);
}

bool vector_erase_range(vector_t *v, vector_size_t first, vector_size_t last)
{
	if (v && last > first && first < v->size) {
		if (last <= v->size) {
			memmove((char *) v->data + first * v->len,
					(char *) v->data + last * v->len,
					v->len * (v->size - last + 1));
			v->size -= last - first;
		} else {
			v->size = first;
		}
		return true;
	}
	return false;
}

void vector_free(vector_t *v)
{
	if (v) {
		free(v->data);
		v->capacity = v->size = 0;
	}
}
