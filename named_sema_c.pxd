cdef extern from "named_sema.h":
    cdef enum SemaWaitResult:
            SEMA_WAIT_OK = 0
            SEMA_WAIT_TIMEOUT = 1
            SEMA_WAIT_ERROR = 2
    cdef cppclass NamedSemaphore:
        NamedSemaphore()
        bint create(const char * name, long init_count, long max_count)
        bint open(const char * name)
        bint is_valid()
        bint wait(int timeout_msec)
        bint post()
