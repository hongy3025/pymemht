cdef extern from "hashtable.h":
    ctypedef struct hashtable
    ctypedef struct ht_iter
    ctypedef unsigned int uint32_t
    ctypedef struct ht_str:
        uint32_t size
        char str[1]
    ctypedef struct ht_iter:
        hashtable *ht
        size_t pos
        ht_str *key
        ht_str *value

    ht_iter* ht_get_iterator(hashtable *ht)
    void ht_free_iterator(ht_iter * iter)
    int ht_iter_next(ht_iter* iter)

    size_t ht_memory_size(size_t capacity)
    hashtable* ht_init(void *base_addr, size_t capacity, int force_init)
    ht_str* ht_get(hashtable *ht, const char *key, uint32_t key_size)
    bint ht_set(hashtable *ht, const char *key, uint32_t key_size, const char *value, uint32_t value_size)
    bint ht_remove(hashtable *ht, const char *key, uint32_t key_size)
    bint ht_destroy(hashtable *ht)
    bint ht_is_valid(hashtable *ht)
