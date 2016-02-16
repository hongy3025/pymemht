#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "hashtable.h"

#define ALLOC(type, n) ((type *)malloc(sizeof(type) * n))

#define ht_flag_base(ht) ((char *)(ht) + (ht)->flag_offset)
#define ht_bucket_base(ht) ((char *)(ht) + (ht)->bucket_offset)

static const unsigned ht_magic = 0xBFBF;

enum bucket_flag {
    empty = 0, used = 1, removed = 2
};


size_t header_size = 1024;

#define bucket_size     1280
#define max_key_size    256
#define max_value_size  (bucket_size - max_key_size)

const float max_load_factor = 0.65f;

static const unsigned int primes[] = { 
    53, 97, 193, 389,
    769, 1543, 3079, 6151,
    12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869,
    3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741
};
static const unsigned int prime_table_length = sizeof (primes) / sizeof (primes[0]);

static inline void fill_ht_str(ht_str *s, const char *str, const uint32_t size) {
    s->size = size;
    memcpy(s->str, str, size);
}

static unsigned int ht_get_prime_by(size_t capacity) {
    unsigned i = 0;
    capacity *= 2;
    for (i = 0; i < prime_table_length; i++) {
        if (primes[i] > capacity)
            return primes[i];
    }
    return 0;
}

size_t ht_memory_size(size_t capacity) {
    const int flag_size = 1; //char
    size_t aligned_capacity = (ht_get_prime_by(capacity) / 4 + 1) * 4; //round up to 4-byte alignment
    return header_size                      //header
         + flag_size * aligned_capacity     //flag
         + bucket_size * aligned_capacity;  //bucket
}

/*dbj2_hash function (copied from libshmht)*/
static unsigned int dbj2_hash (const char *str, size_t size) {
    unsigned long hash = 5381;
    while (size--) {
        char c = *str++;
        hash = ((hash << 5) + hash) + c;    /* hash * 33 + c */
    }
    return (unsigned int) hash;
}

bool is_equal(const char *a, size_t asize, const char *b, size_t bsize) {
    if (asize != bsize)
        return false;
    return strncmp(a, b, asize) ? false : true;
}

bool ht_is_valid(hashtable *ht) {
    return (ht->magic == ht_magic);
}

/*
 * The caller is responsible for the 4-byte alignment of base_addr
 * and the size of base_addr should be no less than ht_get_prime_by(capacity)
 */
hashtable* ht_init(void *base_addr, size_t capacity, int force_init) {
    hashtable* ht = (hashtable *)base_addr;
    if (force_init || !ht_is_valid(ht)) {
        ht->magic     = ht_magic;
        ht->ref_cnt   = 0;

        ht->orig_capacity = capacity;
        ht->capacity      = ht_get_prime_by(capacity);
        ht->size          = 0;

        ht->flag_offset   = header_size;
        ht->bucket_offset = ht->flag_offset + (ht->capacity / 4 + 1) * 4; //alignment

        memset(ht_flag_base(ht), 0, ht->capacity);
    }
    ht->ref_cnt += 1;
    return ht;
}

static size_t ht_position(hashtable *ht, const char *key, uint32_t key_size, bool treat_removed_as_empty) {
    char *flag_base = ht_flag_base(ht);
    char *bucket_base = ht_bucket_base(ht);
    size_t capacity = ht->capacity;
    unsigned long hval = dbj2_hash(key, key_size) % capacity;

    size_t i = hval, di = 1;
    while (true) {
        if (flag_base[i] == empty)
            break;
        if (flag_base[i] == removed && treat_removed_as_empty)
            break;
        if (flag_base[i] == used)
        {
            char *bucket = bucket_base + i * bucket_size;
            ht_str* bucket_key = (ht_str *)bucket;
            if (is_equal(key, key_size, bucket_key->str, bucket_key->size)) {
                break;
            }
        }
        i = (i + di) % capacity;
        di++;
        if (i == hval) {
            //extreme condition: when all flags are 'removed'
            memset(flag_base, 0, capacity);
            break;
        }
    }
    return i;
}

ht_str* ht_get(hashtable *ht, const char *key, uint32_t key_size) {
    size_t i = ht_position(ht, key, key_size, false); //'removed' bucket is not 'empty' when searching a chain.
    if (ht_flag_base(ht)[i] != used) {
        return NULL;
    }
    char *bucket = ht_bucket_base(ht) + i * bucket_size;
    return (ht_str*)(bucket + max_key_size);
}

bool ht_set(hashtable *ht, const char *key, uint32_t key_size, const char *value, uint32_t value_size) {
    if (sizeof(uint32_t) + key_size >= max_key_size || sizeof(uint32_t) + value_size >= max_value_size) {
        //the item is too large
        fprintf(stderr, "the item is too large: key_size(%u), value(%u)\n", key_size, value_size);
        return false;
    }

    char *flag_base = ht_flag_base(ht);
    char *bucket_base = ht_bucket_base(ht);

    ht_str *bucket_key = NULL, *bucket_value = NULL;

    //if it exists: just find and modify it's value
    bucket_value = ht_get(ht, key, key_size);
    if (bucket_value) { 
        fill_ht_str(bucket_value, value, value_size);
        return true;
    }

    //else: find an available bucket, which can be both 'empty' or 'removed'
    size_t i = ht_position(ht, key, key_size, true);

    if (ht->capacity * max_load_factor < ht->size) {
        //hash table is over loaded
        fprintf(stderr, "hash table is over loaded, capacity=%lu, size=%lu\n", ht->capacity, ht->size);
        return false;
    }

    ht->size += 1;
    flag_base[i] = used;

    char *bucket = bucket_base + i * bucket_size;
    bucket_key   = (ht_str*)bucket;
    bucket_value = (ht_str*)(bucket + max_key_size);
    fill_ht_str(bucket_key, key, key_size);
    fill_ht_str(bucket_value, value, value_size);
    return true;
}

bool ht_remove(hashtable *ht, const char *key, uint32_t key_size) {
    size_t i = ht_position(ht, key, key_size, false); //'removed' bucket is not 'empty' when searching a chain.
    if (ht_flag_base(ht)[i] != used) {
        return false;
    }
    ht_flag_base(ht)[i] = removed;
    ht->size -= 1;
    return true;
}

ht_iter* ht_get_iterator(hashtable *ht) {
    ht_iter* iter = ALLOC(ht_iter, 1);
    assert(iter != NULL);
    iter->ht    = ht;
    iter->pos   = -1;
    return iter;
}

void ht_free_iterator(ht_iter * iter) {
	free(iter);
}

int ht_iter_next(ht_iter* iter) {
    size_t i = 0;
    hashtable *ht = iter->ht;
    char *flag_base = ht_flag_base(ht);
    char *bucket_base = ht_bucket_base(ht);

    for (i = iter->pos + 1; i < ht->capacity; i++) {
        if (flag_base[i] == used) {
            char *bucket = bucket_base + i * bucket_size;
            iter->key = (ht_str*)bucket, iter->value = (ht_str*)(bucket + max_key_size);
            iter->pos = i;
            return true;
        }
    }
    return false;
}

bool ht_destroy(hashtable *ht) {
    ht->ref_cnt -= 1;
    return ht->ref_cnt == 0 ? true : false;
}

