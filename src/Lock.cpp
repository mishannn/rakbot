#include "StdAfx.h"

#include "Mutex.h"

#include "Lock.h"

Lock::Lock(Mutex &mutex) {
	_mutex = &mutex;
	_mutex->lock();
}

Lock::~Lock() {
	_mutex->unlock();
}
