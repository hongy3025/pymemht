#ifndef __HASH_TABLE__
#define __HASH_TABLE__

#ifdef _WIN32
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

typedef struct __hashtable {
    uint32_t magic;
    uint32_t ref_cnt;
    uint32_t orig_capacity;
    uint32_t capacity;
    uint32_t size;
    uint32_t flag_offset;
    uint32_t bucket_offset;
	uint32_t max_key_size;
	uint32_t max_value_size;
	uint32_t bucket_size;
} hashtable;

typedef struct _ht_str {
    uint32_t size;
    char str[1];
} ht_str;

typedef struct _ht_iter {
    hashtable *ht;
    size_t pos;
    ht_str *key;
	ht_str *value;
} ht_iter;

size_t ht_memory_size(size_t max_key_size, size_t max_value_size, size_t capacity);
hashtable* ht_init(void *buff_base_addr, size_t buff_size, size_t max_key_size, size_t max_value_size,
	size_t capacity, int force_init);
ht_iter* ht_get_iterator(hashtable *ht);
void ht_free_iterator(ht_iter * iter);
int ht_iter_next(ht_iter* iter);
ht_str* ht_get(hashtable *ht, const char *key, uint32_t key_size);
bool ht_set(hashtable *ht, const char *key, uint32_t key_size, const char *value, uint32_t value_size);
void ht_clear(hashtable *ht);
size_t ht_size(hashtable* ht);
bool ht_remove(hashtable *ht, const char *key, uint32_t key_size);
bool ht_destroy(hashtable *ht);
bool ht_is_valid(hashtable *ht);
size_t ht_max_key_length(hashtable* ht);
size_t ht_max_value_length(hashtable* ht);

#endif
