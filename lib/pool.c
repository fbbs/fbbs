#include <stdlib.h>
#include <unistd.h>

#include "fbbs/pool.h"
#include "fbbs/util.h"

enum {
	ALIGNMENT = sizeof(unsigned long),
};

typedef struct pool_large_t {
	void *ptr;
	struct pool_large_t *next;
} pool_large_t;

typedef struct pool_block_t {
	uchar_t *last;
	uchar_t *end;
	struct pool_block_t *next;
} pool_block_t;

struct pool_t {
	struct pool_block_t *head;
	pool_large_t *large;
};

static int _max_pool_alloc = 0;

/**
 * Align pointer.
 * @param ptr The pointer to be aligned.
 * @return The aligned pointer.
 */
static inline void *_align_ptr(void *ptr)
{
	return (void *)(((uintptr_t)ptr + ALIGNMENT - 1) & ~(ALIGNMENT - 1));
}

/**
 * Create a memory pool.
 * @param size Size of each memory block.
 * @return A pointer to created pool, NULL on error.
 */
pool_t *pool_create(size_t size)
{
	if (!_max_pool_alloc)
		_max_pool_alloc = sysconf(_SC_PAGESIZE);

	if (size <= sizeof(pool_block_t))
		return NULL;

	pool_block_t *b = malloc(size);
	if (!b)
		return NULL;

	b->last = _align_ptr((uchar_t *)b + sizeof(*b));
	b->end = (uchar_t *)b + size;
	b->next = NULL;

	pool_t *p = (pool_t *) b->last;
	b->last = (uchar_t *)p + sizeof(*p);
	p->head = b;
	p->large = NULL;

	return p;
}

/**
 * Clear memory pool.
 * Large memories are freed. Blocks are cleared.
 * @param p The memory pool.
 */
void pool_clear(pool_t *p)
{
	for (pool_large_t *l = p->large; l; l = l->next) {
		if (l->ptr)
			free(l->ptr);
	}
	p->large = NULL;

	for (pool_block_t *b = p->head->next; b; b = b->next) {
		b->last = (uchar_t *)b + sizeof(*b);
	}
	p->head->last = _align_ptr((uchar_t *)(p->head) + sizeof(pool_block_t));
	p->head->last += sizeof(*p);
}

/**
 * Destroy memory pool.
 * @param p The memory pool.
 */
void pool_destroy(pool_t *p)
{
	for (pool_large_t *l = p->large; l; l = l->next) {
		if (l->ptr)
			free(l->ptr);
	}
	p->large = NULL;

	pool_block_t *b = p->head, *next;
	while (b) {
		next = b->next;
		free(b);
		b = next;
	}
}

/**
 * Allocate memory from pool by creating a new block.
 * @param p The pool to allocate from.
 * @param size Required memory size.
 * @return On success, a pointer to allocated space, NULL otherwise.
 */
static void *_pool_alloc_block(pool_t *p, size_t size)
{
	size_t bsize = p->head->end - (uchar_t *)(p->head);
	pool_block_t *b = malloc(bsize);
	if (!b)
		return NULL;

	b->last = (uchar_t *)b + sizeof(*b);
	b->end = (uchar_t *)b + bsize;
	b->next = p->head->next;
	p->head->next = b;

	uchar_t *ret = _align_ptr(b->last);
	b->last = ret + size;
	return ret;
}

/**
 * Allocate a large memory directly.
 * @param p The memory pool.
 * @param size Required memory size.
 * @return On success, a pointer to allocated space, NULL otherwise.
 */
static void *_pool_alloc_large(pool_t *p, size_t size)
{
	pool_large_t *l;
	l = pool_alloc(p, sizeof(*l));
	if (!l)
		return NULL;

	l->next = p->large;
	p->large = l;

	l->ptr = malloc(size);
	return l->ptr;
}

/**
 * Allocate memory from pool.
 * @param p The pool to allocate from.
 * @param size Required memory size.
 * @return On success, a pointer to allocated space, NULL otherwise.
 */
void *pool_alloc(pool_t *p, size_t size)
{
	uchar_t *ret;
	pool_block_t *b;
	if (size < _max_pool_alloc) {
		b = p->head;
		do {
			ret = _align_ptr(b->last);
			if (b->end - ret >= size) {
				b->last = (uchar_t *)ret + size;
				return ret;
			}
			b = b->next;
		} while (b);
		return _pool_alloc_block(p, size);
	}
	return _pool_alloc_large(p, size);
}
