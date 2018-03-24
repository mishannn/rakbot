#pragma once

class Mutex;

class Lock {
private:
	Mutex *_mutex;

public:
	Lock(Mutex &mutex);
	~Lock();
};

