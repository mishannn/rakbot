#include "StdAfx.h"

#include "VehicleStuff.h"

#include "Vehicle.h"

void Vehicle::loadModelData() {
	for (int i = 0; i < VEHICLE_LIST_SIZE; i++) {
		if (VehicleList[i].id != _model)
			continue;

		lock();
		_vehicleClass = VehicleList[i].classId;
		_seatAmount = VehicleList[i].passengers + 1;
		_name = std::string(VehicleList[i].name);
		_passenger = new Player *[_seatAmount];
		for (int i = 0; i < _seatAmount; i++)
			_passenger[i] = nullptr;
		unlock();
	}
}

Vehicle::Vehicle() { }

Vehicle::~Vehicle() {
	if (_passenger != nullptr)
		delete[] _passenger;
}

void Vehicle::reset() {
	lock();
	_doorsOpened = false;
	_engineEnabled = false;
	_lightsEnabled = false;
	_sirenEnabled = false;
	_firstColor = 0;
	_secondColor = 0;
	_seatAmount = 0;
	_vehicleId = 0;
	_model = 0;
	_vehicleClass = VEHICLE_CLASS_NONE;
	for (int i = 0; i < 3; i++)
		_position[i] = 0.f;
	for (int i = 0; i < 3; i++)
		_speed[i] = 0.f;
	for (int i = 0; i < 4; i++)
		_quaternion[i] = 0.f;
	_carHealth = 0.f;
	_driver = nullptr;
	_passenger = nullptr;
	unlock();
}

void Vehicle::setDoorsOpened(bool doorsOpened) {
	lock();
	_doorsOpened = doorsOpened;
	unlock();
}

bool Vehicle::isDoorsOpened() {
	return _doorsOpened;
}

void Vehicle::setEngineEnabled(bool engineEnabled) {
	lock();
	_engineEnabled = engineEnabled;
	unlock();
}

bool Vehicle::isEngineEnabled() {
	return _engineEnabled;
}

void Vehicle::setLightsEnabled(bool lightsEnabled) {
	lock();
	_lightsEnabled = lightsEnabled;
	unlock();
}

bool Vehicle::isLightsEnabled() {
	return _lightsEnabled;
}

void Vehicle::setSirenEnabled(bool sirenEnabled) {
	lock();
	_sirenEnabled = sirenEnabled;
	unlock();
}

bool Vehicle::isSirenEnabled() {
	return _sirenEnabled;
}

void Vehicle::setFirstColor(uint8_t firstColor) {
	lock();
	_firstColor = firstColor;
	unlock();
}

uint8_t Vehicle::getFirstColor() {
	return _firstColor;
}

void Vehicle::setSecondColor(uint8_t secondColor) {
	lock();
	_secondColor = secondColor;
	unlock();
}

uint8_t Vehicle::getSecondColor() {
	return _secondColor;
}

uint8_t Vehicle::getSeatAmount() {
	return _seatAmount;
}

void Vehicle::setVehicleId(uint16_t vehicleId) {
	lock();
	_vehicleId = vehicleId;
	unlock();
}

uint16_t Vehicle::getVehicleId() {
	return _vehicleId;
}

void Vehicle::setModel(int model) {
	lock();
	_model = model;
	unlock();

	loadModelData();
}

int Vehicle::getModel() {
	return _model;
}

int Vehicle::getVehicleClass() {
	return _vehicleClass;
}

int Vehicle::getPassengerAmount() {
	int passengerCount = 0;
	for (int i = 0; i < _seatAmount; i++) {
		if (_passenger[i] != nullptr)
			passengerCount++;
	}
	return passengerCount;
}

void Vehicle::setPosition(int n, float position) {
	if (n < 0 || n >= 3)
		return;

	lock();
	_position[n] = position;
	unlock();
}

float Vehicle::getPosition(int n) {
	if (n < 0 || n >= 3)
		return 0.f;

	return _position[n];
}

void Vehicle::setSpeed(int n, float speed) {
	if (n < 0 || n >= 3)
		return;

	lock();
	_speed[n] = speed;
	unlock();
}

float Vehicle::getSpeed(int n) {
	if (n < 0 || n >= 3)
		return 0.f;

	return _speed[n];
}

void Vehicle::setQuaternion(int n, float quaternion) {
	if (n < 0 || n >= 4)
		return;

	lock();
	_quaternion[n] = quaternion;
	unlock();
}

float Vehicle::getQuaternion(int n) {
	if (n < 0 || n >= 4)
		return 0.f;

	return _quaternion[n];
}

void Vehicle::setCarHealth(float carHealth) {
	lock();
	_carHealth = carHealth;
	unlock();
}

float Vehicle::getCarHealth() {
	return _carHealth;
}

std::string Vehicle::getName() {
	return _name;
}

void Vehicle::setDriver(Player *driver) {
	lock();
	_driver = driver;
	unlock();
}

Player *Vehicle::getDriver() {
	return _driver;
}

void Vehicle::setPassenger(int n, Player *passenger) {
	if (n < 0 || n > 5)
		return;

	lock();
	_passenger[n] = passenger;
	unlock();
}

Player *Vehicle::getPassenger(int n) {
	if (n < 0 || n >= _seatAmount)
		return nullptr;

	return _passenger[n];
}

// GEAR STATE
void Vehicle::setGearState(uint8_t gearState) {
	lock();
	_gearState = gearState;
	unlock();
}

uint8_t Vehicle::getGearState() {
	return _gearState;
}

// TRAILER ID
void Vehicle::setTrailerId(uint16_t trailerId) {
	lock();
	_trailerId = trailerId;
	unlock();
}

uint16_t Vehicle::getTrailerId() {
	return _trailerId;
}

// TRAIN SPEED
void Vehicle::setTrainSpeed(float trainSpeed) {
	lock();
	_trainSpeed = trainSpeed;
	unlock();
}

float Vehicle::getTrainSpeed() {
	return _trainSpeed;
}