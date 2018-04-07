#include "StdAfx.h"

#include "Pickup.h"

Pickup::Pickup() {}

Pickup::~Pickup() {}

void Pickup::reset() {
	Lock lock(&_pickupMutex);

	_pickupId = 0;
	_model = 0;
	_type = 0;

	for (int i = 0; i < 3; i++)
		_position[i] = 0.f;
}

void Pickup::setPickupId(int pickupId) {
	Lock lock(&_pickupMutex);

	_pickupId = pickupId;
}

int Pickup::getPickupId() {
	Lock lock(&_pickupMutex);

	return _pickupId;
}

void Pickup::setModel(int model) {
	Lock lock(&_pickupMutex);

	_model = model;
}

int Pickup::getModel() {
	Lock lock(&_pickupMutex);

	return _model;
}

void Pickup::setType(int type) {
	Lock lock(&_pickupMutex);

	_type = type;
}

int Pickup::getType() {
	Lock lock(&_pickupMutex);

	return _type;
}

void Pickup::setPosition(int n, float val) {
	Lock lock(&_pickupMutex);

	if (n < 0 || n > 3)
		return;

	_position[n] = val;
}

float Pickup::getPosition(int n) {
	Lock lock(&_pickupMutex);

	if (n < 0 || n > 3)
		return 0.f;

	return _position[n];
}