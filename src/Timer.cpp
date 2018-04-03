#include "StdAfx.h"

#include "Timer.h"

Timer::Timer() : Mutex() {
	_timer = GetTickCount();
}

Timer::Timer(uint32_t timer) : Mutex() {
	_timer = timer;
}

Timer::~Timer() { }

void Timer::setTimer(uint32_t timer) {
	_timer = timer;
}

uint32_t Timer::getElapsed(uint32_t fromTime = 0) {
	if (_timer > GetTickCount())
		return 0;

	return (GetTickCount() - _timer);
}

bool Timer::isElapsed(uint32_t ms, bool resetIfTrue) {
	if (_timer >= GetTickCount())
		return false;

	if ((GetTickCount() - _timer) < ms)
		return false;

	if (resetIfTrue)
		setTimerFromCurrentTime();

	return true;
}

void Timer::setTimerFromCurrentTime() {
	lock();
	_timer = GetTickCount();
	unlock();
}