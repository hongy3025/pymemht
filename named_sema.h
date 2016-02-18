#include <windows.h>

class NamedSemaphore {
private:
	HANDLE sema;
public:
	NamedSemaphore(): sema(NULL) {
	}
	~NamedSemaphore() {
		if (this->sema != NULL) {
			CloseHandle(this->sema);
			this->sema = NULL;
		}
	}
	bool create(const char * name, long init_count, long max_count) {
		this->sema = CreateSemaphore(NULL, init_count, max_count, name);
		return this->sema != NULL;
	}
	bool open(const char * name) {
		this->sema = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name);
		return this->sema != NULL;
	}
	bool is_valid() {
		return sema != NULL;
	}
	bool wait(int timeout_msec) {
		if (!is_valid()) {
			return false;
		}
		DWORD timeout = INFINITE;
		if (timeout_msec >= 0) {
			timeout = timeout_msec;
		}
		DWORD result = WaitForSingleObject(this->sema, timeout);
		if (result == WAIT_OBJECT_0) {
			return true;
		}
		return false;
	}
	bool post() {
		if (!is_valid()) {
			return false;
		}
		if (!ReleaseSemaphore(this->sema, 1, NULL)) {
			return false;
		}
		return true;
	}
};
