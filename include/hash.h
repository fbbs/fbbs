#ifndef FB_HASH_H
#define FB_HASH_H

/**
 * @file hash.h
 * @brief Hash Tables
 */

enum {
	HASH_DEFAULT_MAX = 15,  ///< default hash max.
	/** indicates a string key when passing as key length. */
	HASH_KEY_STRING = -1,
};

/**
 * Hash function prototype.
 * @param key The key.
 * @param klen The length of the key. ::HASH_KEY_STRING indicates that the key
 *        is a string and then the actual length is returned via the pointer.
 * */
typedef unsigned int (*hash_func_t)(const char *key, unsigned int *klen);

/** Hash entry struct. */
typedef struct hash_entry_t
{
	struct hash_entry_t *next;  ///< pointer to next entry.
	const char *key;            ///< key.
	unsigned int klen;          ///< key length.
	unsigned int hash;          ///< hash code.
	const void *val;            ///< value.
} hash_entry_t;

/** Hash table struct. */
typedef struct hash_t
{
	unsigned int count;    ///< number of entries in the table.
	unsigned int max;      ///< maximum slot number, must be 2^n - 1.
	hash_entry_t **array;  ///< array for hash entries.
	hash_entry_t *free;    ///< list of recycled entries.
	hash_func_t func;      ///< hash function.
} hash_t;

unsigned int hash_func_default(const char *key, unsigned int *klen);
int hash_create(hash_t *ht, unsigned int max, hash_func_t func);
int hash_set(hash_t *ht, const char *key, unsigned int klen, const void *val);
void *hash_get(hash_t *ht, const void *key, unsigned int klen);
void hash_destroy(hash_t *ht);

#endif // FB_HASH_H

