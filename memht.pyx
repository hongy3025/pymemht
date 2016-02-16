# distutils: language = c++

from cpython.bytes cimport *
from hashtable cimport *

class HashTableError(Exception):
    pass

cdef class MemoryHashTable(object):
    cdef hashtable * ht

    def __cinit__(self):
        self.ht = NULL

    def __init__(self):
        pass

    def __dealloc__(self):
        if self.ht != NULL:
            ht_destroy(self.ht)

    cpdef bytes get(self, bytes key):
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef ht_str *value = ht_get(self.ht, key_s, key_n)
        if value == NULL:
            return None
        cdef bytes result = value.str[:value.size]
        return result

    cpdef void set(self, bytes key, bytes value):
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)
        cdef char * value_s = NULL
        cdef Py_ssize_t value_n = 0
        PyBytes_AsStringAndSize(value, &value_s, &value_n)

        cdef bint result = ht_set(self.ht, key_s, key_n, value_s, value_n)
        if not result:
            raise HashTableError('set hashable')

    cpdef void remove(self, bytes key):
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef bint result = ht_remove(self.ht, key_s, key_n)
        if not result:
            raise HashTableError('remove hashable key')

    cpdef bint has_key(self, bytes key):
        cdef bytes result = self.get(key)
        if result is None:
            return False
        return True

    def __contains__(self, key):
        return self.has_key(key)

    def __getitem__(self, key):
        cdef result = self.get(key)
        if result is None:
            raise KeyError('no such key')
        return result

    def iteritems(self):
        cdef ht_iter *iter = ht_get_iterator(self.ht)
        cdef bytes pykey
        cdef bytes pyvalue
        cdef ht_str *key
        cdef ht_str *value
        try:
            while ht_iter_next(iter):
                key = iter.key
                value = iter.value
                pykey = key.str[:key.size]
                pyvalue = value.str[:value.size]
                yield (pykey, pyvalue)
        finally:
            ht_free_iterator(iter)




