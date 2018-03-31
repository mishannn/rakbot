#pragma once

#include "Mutex.h"

class Timer : private Mutex {
private:
	uint32_t _timer;

public:
	Timer();
	Timer(uint32_t timer);
	~Timer();

	void reset();
	void setTimer(uint32_t timer);
	uint32_t getTimer() { return _timer; }

	uint32_t getElapsed();
	bool isElapsed(uint32_t ms, bool resetIfTrue);
};

