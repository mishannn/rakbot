#include "StdAfx.h"

#include "Pickup.h"

Pickup::Pickup() {}

Pickup::~Pickup() {}

void Pickup::reset() {
	_pickupId = 0;
	_model = 0;
	_type = 0;

	for (int i = 0; i < 3; i++)
		_position[i] = 0.f;
}

void Pickup::setPickupId(int pickupId) {
	_pickupId = pickupId;
}

int Pickup::getPickupId() {
	return _pickupId;
}

void Pickup::setModel(int model) {
	_model = model;
}

int Pickup::getModel() {
	return _model;
}

void Pickup::setType(int type) {
	_type = type;
}

int Pickup::getType() {
	return _type;
}

void Pickup::setPosition(int n, float val) {
	if (n < 0 || n > 3)
		return;

	_position[n] = val;
}

float Pickup::getPosition(int n) {
	if (n < 0 || n > 3)
		return 0.f;

	return _position[n];
}