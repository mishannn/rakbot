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
	uint32_t tickCount = GetTickCount();

	if (_timer > tickCount)
		return 0;

	return (tickCount - _timer);
}

bool Timer::isElapsed(uint32_t ms, bool resetIfTrue) {
	uint32_t tickCount = GetTickCount();

	if (_timer >= tickCount)
		return false;

	if ((tickCount - _timer) < ms)
		return false;

	if (resetIfTrue)
		_timer = tickCount;

	return true;
}

void Timer::setTimerFromCurrentTime() {
	_timer = GetTickCount();
}