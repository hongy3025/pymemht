# distutils: language = c++

from cpython.bytes cimport *
from hashtable cimport *

cdef extern from "Python.h":
    int PyObject_AsWriteBuffer(object obj, void **buf, Py_ssize_t *buf_len)

class HashTableError(Exception):
    pass

cdef class MemoryHashTable(object):
    cdef object buf_obj
    cdef hashtable * ht

    def __cinit__(self, obj, Py_ssize_t max_key_size, Py_ssize_t max_value_size,
                  Py_ssize_t capacity, int offset=0):
        self.ht = NULL
        cdef void * buf = NULL
        cdef Py_ssize_t buf_len = 0
        cdef int result = PyObject_AsWriteBuffer(obj, &buf, &buf_len)
        if result == -1:
            raise
        cdef size_t ht_size = ht_memory_size(max_key_size, max_value_size, capacity)
        if <int>(offset + ht_size) > <int>buf_len:
            desc = 'no enought space to hold hashtable. need {} bytes'.format(ht_size)
            raise MemoryError(desc)
        cdef void * buff_base = <char*>buf + offset
        cdef size_t buff_size = buf_len - offset
        cdef hashtable * ht = ht_init(buff_base, buff_size,
                                      max_key_size, max_value_size, capacity, 1)
        if ht == NULL:
            raise RuntimeError('can not init hashtable')
        self.buf_obj = obj
        self.ht = ht

    def __dealloc__(self):
        if self.ht != NULL:
            ht_destroy(self.ht)
        self.buf_obj = None

    cpdef bytes get(self, bytes key):
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef ht_str *value = ht_get(self.ht, key_s, key_n)
        if value == NULL:
            return None
        cdef bytes result = value.str[:value.size]
        return result

    cpdef int set(self, bytes key, bytes value) except? -1:
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)
        cdef char * value_s = NULL
        cdef Py_ssize_t value_n = 0
        PyBytes_AsStringAndSize(value, &value_s, &value_n)

        cdef bint result = ht_set(self.ht, key_s, key_n, value_s, value_n)
        if not result:
            raise HashTableError('set hashable')
        return 0

    cpdef int remove(self, bytes key) except? -1:
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef bint result = ht_remove(self.ht, key_s, key_n)
        if not result:
            raise HashTableError('remove hashable key')
        return 0

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

    def __len__(self):
        return ht_size(self.ht)




