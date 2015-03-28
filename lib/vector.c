#include <stdlib.h>
#include <string.h>
#include "fbbs/vector.h"

enum {
	VECTOR_DEFAULT_CAPACITY = 8, ///< 动态数组的默认初始长度
};

/**
 * 初始化一个动态数组
 * @param v 动态数组
 * @param len 单个数组元素的长度
 * @param capacity 动态数组的元素数量
 * @return 初始化成功与否
 */
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

/**
 * 申请数组空间
 * @param v 动态数组
 * @param capacity 要申请的元素数量
 * @return 空间分配成功与否
 */
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

/**
 * 获取动态数组的长度
 * @param v 动态数组
 * @return 动态数组的长度
 */
vector_size_t vector_size(const vector_t *v)
{
	return v->size;
}

/**
 * 获取动态数组指定位置的元素
 * @param v 动态数组
 * @param position 位置
 * @return 指定位置的元素
 */
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

/**
 * 在动态数组尾部分配更多空间
 * @param v 动态数组
 * @param size 要增加的元素数目
 * @return 新分配的元素起始地址, 失败返回NULL
 */
void *vector_grow(vector_t *v, vector_size_t size)
{
	if (v && _vector_grow(v, size) && v->size >= size)
		return ((char *) v->data) + v->len * (v->size - size);
	return NULL;
}

/**
 * 删除动态数组中的指定元素
 * @param v 动态数组
 * @param position 位置
 * @return 删除成功与否
 */
bool vector_erase(vector_t *v, vector_size_t position)
{
	return vector_erase_range(v, position, position + 1);
}

static void _vector_move(vector_t *v, vector_size_t dst, vector_size_t src,
		vector_size_t size)
{
	memmove((char *) v->data + dst * v->len,
			(char *) v->data + src * v->len,
			v->len * size);
}

/**
 * 删除动态数组中的指定元素
 * @param v 动态数组
 * @param first 要删除的第一个元素位置
 * @param last 要删除的最后一个元素位置（不含此元素）
 * @return 删除成功与否
 */
bool vector_erase_range(vector_t *v, vector_size_t first, vector_size_t last)
{
	if (v && last > first && first < v->size) {
		if (last <= v->size) {
			_vector_move(v, first, last, v->size - last + 1);
			v->size -= last - first;
		} else {
			v->size = first;
		}
		return true;
	}
	return false;
}

/*
 * 向动态数组插入元素
 * @param v 动态数组
 * @param position 插入元素的位置, 允许在末端, 但不能超出末端
 * @param val 要插入的元素
 * @return 成功返回true, 否则false
 */
bool vector_insert(vector_t *v, vector_size_t position, void *val)
{
	if (position > v->size || !val || !_vector_grow(v, 1))
		return false;
	if (position != v->size - 1)
		_vector_move(v, position + 1, position, v->size - 1 - position);
	memcpy((char *) v->data + position * v->len, val, v->len);
	return true;
}

/**
 * 释放动态数组
 * @param v 动态数组
 */
void vector_free(vector_t *v)
{
	if (v) {
		free(v->data);
		v->capacity = v->size = 0;
	}
}
