#ifndef __HASH_TABLE__
#define __HASH_TABLE__

#ifdef _WIN32
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

typedef struct __hashtable {
    unsigned magic;
    size_t ref_cnt, orig_capacity, capacity, size, flag_offset, bucket_offset;
} hashtable;

typedef struct _ht_str {
    uint32_t size;
    char str[1];
} ht_str;

typedef struct _ht_iter {
    hashtable *ht;
    size_t pos;
    ht_str *key, *value;
} ht_iter;

ht_iter* ht_get_iterator(hashtable *ht);
void ht_free_iterator(ht_iter * iter);
int ht_iter_next(ht_iter* iter);

size_t ht_memory_size(size_t capacity);
hashtable* ht_init(void *base_addr, size_t capacity, int force_init);
ht_str* ht_get(hashtable *ht, const char *key, uint32_t key_size);
bool ht_set(hashtable *ht, const char *key, uint32_t key_size, const char *value, uint32_t value_size);
bool ht_remove(hashtable *ht, const char *key, uint32_t key_size);
bool ht_destroy(hashtable *ht);
bool ht_is_valid(hashtable *ht);

#endif
