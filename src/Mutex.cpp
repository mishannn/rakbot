#include "StdAfx.h"

#include "Mutex.h"

Mutex::Mutex() {
	_mutex = CreateMutex(NULL, FALSE, NULL);
	// _locked = false;
	// _thread = NULL;
}

Mutex::~Mutex() {}

void Mutex::lock() {
	WaitForSingleObject(_mutex, INFINITE);
	// printf("Try lock\n");

	/* while (_locked && _thread != NULL && _thread != GetCurrentThread()) {
		Sleep(1);
	}
	_locked = true;
	_thread = GetCurrentThread(); */

	// printf("Locked\n");
	// _mutex.lock();
}

void Mutex::unlock() {
	ReleaseMutex(_mutex);
	// printf("Unlocked\n");
	// _locked = false;
	// _thread = NULL;
	// _mutex.unlock();
}
