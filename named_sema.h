#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <semaphore.h>
#endif

#ifndef _WIN32
#undef HAVE_SEM_TIMEDWAIT
#ifndef HAVE_SEM_TIMEDWAIT
static inline int sem_timedwait(sem_t *sem, struct timespec *deadline) {
    unsigned long delay, difference;
    struct timeval now, tvdeadline, tvdelay;

    errno = 0;
    tvdeadline.tv_sec = deadline->tv_sec;
    tvdeadline.tv_usec = deadline->tv_nsec / 1000;

    for (delay = 0 ; ; delay += 1000) {
        /* poll */
        if (sem_trywait(sem) == 0)
            return 0;

        else if (errno != EAGAIN)
            return -1;

        /* get current time */
        if (gettimeofday(&now, NULL) < 0) {
            return -1;
		}

        /* check for timeout */
        if (tvdeadline.tv_sec < now.tv_sec ||
            (tvdeadline.tv_sec == now.tv_sec &&
             tvdeadline.tv_usec <= now.tv_usec)) {
            errno = ETIMEDOUT;
            return -1;
        }

        /* calculate how much time is left */
        difference = (tvdeadline.tv_sec - now.tv_sec) * 1000000 +
            (tvdeadline.tv_usec - now.tv_usec);

        /* check delay not too long -- maximum is 20 msecs */
        if (delay > 20000) {
            delay = 20000;
		}
        if (delay > difference) {
            delay = difference;
		}

        /* sleep */
        tvdelay.tv_sec = delay / 1000000;
        tvdelay.tv_usec = delay % 1000000;
        if (select(0, NULL, NULL, NULL, &tvdelay) < 0) {
            return -1;
		}
    }
	return 0;
}
#endif
#endif

class NamedSemaphore {
private:
	std::string name;
#ifdef _WIN32
	HANDLE sema;
#else
	sem_t * sema;
	bool own_sema;
#endif
public:
	NamedSemaphore(): sema(NULL), own_sema(false) {
	}
	~NamedSemaphore() {
		if (this->sema != NULL) {
#ifdef _WIN32
			CloseHandle(this->sema);
#else
			sem_close(this->sema);
			if (this->own_sema) {
				sem_unlink(this->name.c_str());
			}
#endif
			this->sema = NULL;
		}
	}
	bool create(const char * name, long init_count, long max_count) {
#ifdef _WIN32
		this->sema = CreateSemaphore(NULL, init_count, max_count, name);
		if (this->sema == NULL) {
			return false;
		}
#else
		this->sema = sem_open(name, O_CREAT|O_EXCL, 0644, init_count);
		if (this->sema == SEM_FAILED) {
			this->sema = NULL;
			return false;
		}
#endif
		this->own_sema = true;
		this->name = name;
		return true;
	}
	bool open(const char * name) {
#ifdef _WIN32
		this->sema = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name);
		if (this->sema == NULL) {
			return false;
		}
#else
		this->sema = sem_open(name, 0);
		if (this->sema == SEM_FAILED) {
			this->sema = NULL;
			return false;
		}
#endif
		this->name = name;
		return true;
	}
	bool is_valid() {
		return sema != NULL;
	}
	bool wait(int timeout_msec) {
		if (!is_valid()) {
			return false;
		}
#ifdef _WIN32
		DWORD timeout = INFINITE;
		if (timeout_msec >= 0) {
			timeout = timeout_msec;
		}
		DWORD result = WaitForSingleObject(this->sema, timeout);
		if (result == WAIT_OBJECT_0) {
			return true;
		}
		return false;
#else
		int result;
		do {
			if (timeout_msec < 0) {
				result = sem_wait(this->sema);
			} else if (timeout_msec == 0) {
				result = sem_trywait(this->sema);
			} else {
				struct timeval now;
				struct timespec deadline = {0};
				long sec, nsec;
				if (gettimeofday(&now, NULL) < 0) {
					return false;
				}
				double timeout = (double)timeout_msec / 1e3;
				sec = (long) timeout;
				nsec = (long) (1e9 * (timeout - sec) + 0.5);
				deadline.tv_sec = now.tv_sec + sec;
				deadline.tv_nsec = now.tv_usec * 1000 + nsec;
				deadline.tv_sec += (deadline.tv_nsec / 1000000000);
				deadline.tv_nsec %= 1000000000;
				result = sem_timedwait(this->sema, &deadline);
			}
		} while (result < 0 && errno == EINTR);
		if (result == 0) {
			return true;
		}
		return false;
#endif
	}
	bool post() {
		if (!is_valid()) {
			return false;
		}
#ifdef _WIN32
		if (!ReleaseSemaphore(this->sema, 1, NULL)) {
			return false;
		}
		return true;
#else
		if (sem_post(this->sema) == 0) {
			return true;
		}
		return false;
#endif
	}
};
