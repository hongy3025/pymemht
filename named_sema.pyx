# distutils: language = c++

cimport named_sema_c

class NamedSemaphoreError(Exception):
    pass

class NamedSemaphoreTimeout(NamedSemaphoreError):
    pass

cdef class NamedSemaphore(object):
    cdef named_sema_c.NamedSemaphore * sema

    def __cinit__(self):
        self.sema = new named_sema_c.NamedSemaphore()

    def __dealloc__(self):
        if self.sema != NULL:
            del self.sema

    def create(self, str name, long init_count, long max_count):
        if not self.sema.create(name, init_count, max_count):
            raise NamedSemaphoreError('create')

    def open(self, str name):
        if not self.sema.open(name):
            raise NamedSemaphoreError('open')

    def is_valid(self):
        return self.sema.is_valid()

    def wait(self, int timeout_msec=-1):
        result = self.sema.wait(timeout_msec)
        if result == named_sema_c.SEMA_WAIT_TIMEOUT:
            raise NamedSemaphoreTimeout('timeout')
        elif result != named_sema_c.SEMA_WAIT_OK:
            raise NamedSemaphoreError('wait')

    def post(self):
        if not self.sema.post():
            raise NamedSemaphoreError('post')

