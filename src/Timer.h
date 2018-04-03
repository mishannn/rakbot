#pragma once

#include <Windows.h>

#include "Mutex.h"

class Timer : private Mutex {
private:
	uint32_t _timer;

public:
	Timer();
	Timer(uint32_t timer);
	~Timer();

	void setTimerFromCurrentTime();
	void setTimer(uint32_t timer);
	uint32_t getTimer() { return _timer; }

	uint32_t getElapsed(uint32_t fromTime = 0);
	bool isElapsed(uint32_t ms, bool resetIfTrue);

	static uint32_t getCurrentTime() { return GetTickCount(); }
};

