#include "StdAfx.h"

#include "Pickup.h"

Pickup::Pickup() {}

Pickup::~Pickup() {}

void Pickup::reset() {
	lock();
	_pickupId = 0;
	_model = 0;
	_type = 0;

	for (int i = 0; i < 3; i++)
		_position[i] = 0.f;

	unlock();
}

void Pickup::setPickupId(int pickupId) {
	lock();
	_pickupId = pickupId;
	unlock();
}

int Pickup::getPickupId() {
	return _pickupId;
}

void Pickup::setModel(int model) {
	lock();
	_model = model;
	unlock();
}

int Pickup::getModel() {
	return _model;
}

void Pickup::setType(int type) {
	lock();
	_type = type;
	unlock();
}

int Pickup::getType() {
	return _type;
}

void Pickup::setPosition(int n, float val) {
	if (n < 0 || n > 3)
		return;

	lock();
	_position[n] = val;
	unlock();
}

float Pickup::getPosition(int n) {
	if (n < 0 || n > 3)
		return 0.f;

	return _position[n];
}