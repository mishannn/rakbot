#include "StdAfx.h"

#include "Timer.h"

Timer::Timer() {
	_timer = GetTickCount();
}

Timer::Timer(uint32_t timer) {
	_timer = timer;
}

Timer::~Timer() {
}

void Timer::setTimer(uint32_t timer) {
	Lock lock(&_timerMutex);

	_timer = timer;
}

uint32_t Timer::getElapsed() {
	Lock lock(&_timerMutex);

	if (_timer > GetTickCount())
		return 0;

	return (GetTickCount() - _timer);
}

bool Timer::isElapsed(uint32_t ms, bool resetIfTrue) {
	Lock lock(&_timerMutex);

	if (_timer >= GetTickCount())
		return false;

	if ((GetTickCount() - _timer) < ms)
		return false;

	if (resetIfTrue)
		_timer = GetTickCount();

	return true;
}

void Timer::setTimerFromCurrentTime() {
	Lock lock(&_timerMutex);

	_timer = GetTickCount();
}