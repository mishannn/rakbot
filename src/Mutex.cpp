#include "StdAfx.h"

#include "RakBot.h"

#include "Mutex.h"

Mutex::Mutex() {
	_mutex = CreateMutex(NULL, FALSE, NULL);
}

Mutex::~Mutex() {
	unlock();
	CloseHandle(_mutex);
}

void Mutex::lock() {
	WaitForSingleObject(_mutex, INFINITE);
}

void Mutex::unlock() {
	ReleaseMutex(_mutex);
}
