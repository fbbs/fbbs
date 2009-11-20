#include <stdlib.h>
#include <string.h>
#include "hash.h"

/**
 * Default hash funtion (djb algorithm).
 * @param [in] key The key.
 * @param [in,out] klen The length of the key. See ::hash_func_t.
 * @return The calculated hash value.
 */
unsigned int hash_func_default(const char *key, unsigned int *klen)
{
	unsigned int value = 0;
	if (*klen == HASH_KEY_STRING) {
		const char *str = key;
		while (*str != '\0') {
			value += value * 33 + *str++;
		}
		*klen = str - key;
	} else {
		unsigned int len = *klen;
		while (len-- > 0) {
			value += value * 33 + *key++;
		}
	}
	return value;
}

/**
 * Create a new hash table.
 * @param [out] ht Hash table to create.
 * @param [in] max Max slot number (0-based). Set to default if 0.
 * @param [in] func hash function. Use default if NULL.
 * @return 0 on success, -1 on error.
 * @note max must be 2^n - 1, n = 1, 2, 3, ...
 */
int hash_create(hash_t *ht, unsigned int max, hash_func_t func)
{
	if (max == 0)
		max = HASH_DEFAULT_MAX;
	size_t bytes = sizeof(hash_entry_t *) * (max + 1);
	hash_entry_t **array = malloc(bytes);
	if (array == NULL)
		return -1;
	memset(array, 0, bytes);

	ht->count = 0;
	ht->max = max;
	ht->array = array;
	ht->free = NULL;
	if (func == NULL)
		ht->func = hash_func_default;
	else
		ht->func = func;
	return 0;
}

/**
 * Expand an existing hash table.
 * @param [in, out] ht Hash table to be expanded.
 * @return 0 on success, -1 on error.
 */
static int hash_expand(hash_t *ht)
{
	unsigned int max = 2 * ht->max + 1;
	size_t bytes = sizeof(hash_entry_t *) * (max + 1);
	hash_entry_t **array = malloc(bytes);
	if (array == NULL)
		return -1;
	memset(array, 0, bytes);

	unsigned int i;
	for (i = 0; i <= ht->max; ++i) {
		hash_entry_t *entry = ht->array[i];
		while (entry) {
			hash_entry_t **ptr = array + (entry->hash & max);
			hash_entry_t *next = entry->next;
			entry->next = *ptr;
			*ptr = entry;
			entry = next;
		}
	}
	ht->max = max;
	free(ht->array);
	ht->array = array;
	return 0;
}

/**
 * Find an entry in a hash table.
 * @param [in,out] ht The hash table.
 * @param [in] key The key.
 * @param [in] klen The length of the key. See ::hash_func_t.
 * @param [in] val if there is no entry associated with the key and val is not
 *        NULL, the function will allocate a new entry to store the key-value
 *        pair.
 * @return Pointer to pointer to the entry if found or new entry allocated,
 *         pointer to NULL otherwise.
 */
static hash_entry_t **find_entry(hash_t *ht, const char *key,
		unsigned int klen, const void *val)
{
	unsigned int hash = (*ht->func)(key, &klen);
	hash_entry_t **ptr = ht->array + (hash & ht->max), *entry;
	for (entry = *ptr; entry; ptr = &entry->next, entry = *ptr) {
		if (entry->klen == klen && entry->hash == hash
				&& memcmp(entry->key, key, klen) == 0)
			break;
	}
	if (entry || !val)
		return ptr;

	if ((entry = ht->free) != NULL)
		ht->free = entry->next;
	else
		entry = malloc(sizeof(*entry));
	if (!entry)
		return NULL;
	entry->next = NULL;
	entry->key = key;
	entry->klen = klen;
	entry->hash = hash;
	entry->val = val;
	*ptr = entry;
	ht->count++;
	return ptr;
}

/**
 * Put a key-value pair into a hash table.
 * @param [in,out] ht The hash table.
 * @param [in] key The key.
 * @param [in] klen The length of the key. See ::hash_func_t.
 * @param [in] val The value associated with the key. Delete the pair if NULL.
 * @return 0 on success, -1 on error.
 */
int hash_set(hash_t *ht, const char *key, unsigned int klen, const void *val)
{
	hash_entry_t **ptr = find_entry(ht, key, klen, val);
	if (val) {
		if (!*ptr)
			return -1;
		(*ptr)->val = val;
		if (ht->count > ht->max) {
			return hash_expand(ht);
		}
	} else {
		if (*ptr) {
			// deletion.
			hash_entry_t *old = *ptr;
			*ptr = old->next;
			old->next = ht->free;
			ht->free = old;
			ht->count--;
		}
	}
	return 0;
}

/**
 * Look up the value associated with a key in a hash table.
 * @param [in] ht The hash table.
 * @param [in] key The key.
 * @param [in] klen The length of the key. See ::hash_func_t.
 * @return The value associated with the key, NULL if not found.
 */
void *hash_get(hash_t *ht, const void *key, unsigned int klen)
{
	hash_entry_t **ptr = find_entry(ht, key, klen, NULL);
	if (*ptr)
		return (void *)((*ptr)->val);
	return NULL;
}

/**
 * Destroy a hash table.
 * @param [in,out] ht The hash table.
 */
void hash_destroy(hash_t *ht)
{
	unsigned int i;
	hash_entry_t *entry, *next;
	for (i = 0; i < ht->max; ++i) {
		entry = ht->array[i];
		while (entry) {
			next = entry->next;
			free(entry);
			entry = next;
		}
	}
	entry = ht->free;
	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
	free(ht->array);
}
