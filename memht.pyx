# distutils: language = c++

import inspect
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

    cdef bytes _get(self, bytes key):
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef ht_str *value = ht_get(self.ht, key_s, key_n)
        if value == NULL:
            return None
        cdef bytes result = value.str[:value.size]
        return result

    cdef int _set(self, bytes key, bytes value) except? -1:
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

    cdef int _remove(self, bytes key) except? -1:
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef bint result = ht_remove(self.ht, key_s, key_n)
        if not result:
            raise HashTableError('remove hashable key')
        return 0

    cdef bint _has_key(self, bytes key):
        cdef bytes result = self.get(key)
        if result is None:
            return False
        return True

    def get(self, bytes key):
        return self._get(key)

    def set(self, bytes key, bytes value):
        return self._set(key, value)

    def has_key(self, bytes key):
        return self._has_key(key)

    def update(self, *args, **kwargs):
        cdef object other
        if kwargs:
            for k, v in kwargs.iteritems():
                self.set(k, v)
        elif args:
            other = args[0]
            try:
                for k, v in other.iteritems():
                    self._set(k, v)
            except:
                for k, v in iter(other):
                    self._set(k, v)

    def pop(self, bytes key, bytes default=None):
        cdef char * key_s = NULL
        cdef Py_ssize_t key_n = 0
        PyBytes_AsStringAndSize(key, &key_s, &key_n)

        cdef bytes result
        cdef ht_str *ht_value = ht_get(self.ht, key_s, key_n)
        if ht_value == NULL:
            if default is None:
                raise KeyError('no such key')
            else:
                return default
        else:
            result = ht_value.str[:ht_value.size]
            ht_remove(self.ht, key_s, key_n)
            return result

    def clear(self):
        ht_clear(self.ht)

    def __contains__(self, bytes key):
        return self._has_key(key)

    def __getitem__(self, bytes key):
        cdef bytes result = self._get(key)
        if result is None:
            raise KeyError('no such key')
        return result

    def __setitem__(self, bytes key, bytes value):
        self._set(key, value)

    def __delitem__(self, bytes key):
        self._remove(key)

    def __len__(self):
        return ht_size(self.ht)

    def iterkeys(self):
        cdef ht_iter *iter = ht_get_iterator(self.ht)
        cdef bytes pykey
        cdef ht_str *key
        try:
            while ht_iter_next(iter):
                key = iter.key
                pykey = key.str[:key.size]
                yield pykey
        finally:
            ht_free_iterator(iter)

    def itervalues(self):
        cdef ht_iter *iter = ht_get_iterator(self.ht)
        cdef bytes pyvalue
        cdef ht_str *value
        try:
            while ht_iter_next(iter):
                value = iter.value
                pyvalue = value.str[:value.size]
                yield pyvalue
        finally:
            ht_free_iterator(iter)

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

def get_memory_size(size_t max_key_size, size_t max_value_size, size_t capacity):
    return ht_memory_size(max_key_size, max_value_size, capacity)
