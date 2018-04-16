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
	_timer = timer;
}

uint32_t Timer::getElapsed() {
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
		_timer = GetTickCount();

	return true;
}

void Timer::setTimerFromCurrentTime() {
	_timer = GetTickCount();
}