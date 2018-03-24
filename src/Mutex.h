#pragma once

#include <mutex>
#include <Windows.h>

class Mutex {
private:
	HANDLE _mutex;
	// std::recursive_mutex _mutex;
	// bool _locked;
	// HANDLE _thread;

public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
};