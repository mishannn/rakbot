#pragma once

#include <Windows.h>

class Mutex {
private:
	HANDLE _mutex;

public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
};

class Lock {
private:
	Mutex *_mutex;

public:
	Lock(Mutex *mutex) {
		_mutex = mutex;
		_mutex->lock();
	}

	~Lock() {
		_mutex->unlock();
	}
};
