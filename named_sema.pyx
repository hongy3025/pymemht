# distutils: language = c++

cimport named_sema_c

cdef class NamedSemaphore(object):
    cdef named_sema_c.NamedSemaphore * sema

    def __cinit__(self):
        self.sema = new named_sema_c.NamedSemaphore()

    def __dealloc__(self):
        if self.sema != NULL:
            del self.sema

    def create(self, str name, long init_count, long max_count):
        return self.sema.create(name, init_count, max_count)

    def open(self, str name):
        return self.sema.open(name)

    def is_valid(self):
        return self.sema.is_valid()

    def wait(self, int timeout_msec=-1):
        return self.sema.wait(timeout_msec)

    def post(self):
        return self.sema.post()

