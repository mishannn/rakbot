#include "StdAfx.h"

#include "VehicleStuff.h"

#include "Vehicle.h"

void Vehicle::loadModelData() {
	for (int i = 0; i < VEHICLE_LIST_SIZE; i++) {
		if (VehicleList[i].id != _model)
			continue;

			_vehicleClass = VehicleList[i].classId;
		_seatAmount = VehicleList[i].passengers + 1;
		_name = std::string(VehicleList[i].name);
		_passenger = new Player *[_seatAmount];
		for (int i = 0; i < _seatAmount; i++)
			_passenger[i] = nullptr;
		}
}

Vehicle::Vehicle() {}

Vehicle::~Vehicle() {
	if (_passenger != nullptr)
		delete[] _passenger;
}

void Vehicle::reset() {
	Lock lock(&_vehicleMutex);

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
}

void Vehicle::setDoorsOpened(bool doorsOpened) {
	Lock lock(&_vehicleMutex);

	_doorsOpened = doorsOpened;
}

bool Vehicle::isDoorsOpened() {
	Lock lock(&_vehicleMutex);

	return _doorsOpened;
}

void Vehicle::setEngineEnabled(bool engineEnabled) {
	Lock lock(&_vehicleMutex);

	_engineEnabled = engineEnabled;
}

bool Vehicle::isEngineEnabled() {
	Lock lock(&_vehicleMutex);

	return _engineEnabled;
}

void Vehicle::setLightsEnabled(bool lightsEnabled) {
	Lock lock(&_vehicleMutex);

	_lightsEnabled = lightsEnabled;
}

bool Vehicle::isLightsEnabled() {
	Lock lock(&_vehicleMutex);

	return _lightsEnabled;
}

void Vehicle::setSirenEnabled(bool sirenEnabled) {
	Lock lock(&_vehicleMutex);

	_sirenEnabled = sirenEnabled;
}

bool Vehicle::isSirenEnabled() {
	Lock lock(&_vehicleMutex);

	return _sirenEnabled;
}

void Vehicle::setFirstColor(uint8_t firstColor) {
	Lock lock(&_vehicleMutex);

	_firstColor = firstColor;
}

uint8_t Vehicle::getFirstColor() {
	Lock lock(&_vehicleMutex);

	return _firstColor;
}

void Vehicle::setSecondColor(uint8_t secondColor) {
	Lock lock(&_vehicleMutex);

	_secondColor = secondColor;
}

uint8_t Vehicle::getSecondColor() {
	Lock lock(&_vehicleMutex);

	return _secondColor;
}

uint8_t Vehicle::getSeatAmount() {
	Lock lock(&_vehicleMutex);

	return _seatAmount;
}

void Vehicle::setVehicleId(uint16_t vehicleId) {
	Lock lock(&_vehicleMutex);

	_vehicleId = vehicleId;
}

uint16_t Vehicle::getVehicleId() {
	Lock lock(&_vehicleMutex);

	return _vehicleId;
}

void Vehicle::setModel(int model) {
	Lock lock(&_vehicleMutex);

	_model = model;

	loadModelData();
}

int Vehicle::getModel() {
	Lock lock(&_vehicleMutex);

	return _model;
}

int Vehicle::getVehicleClass() {
	Lock lock(&_vehicleMutex);

	return _vehicleClass;
}

int Vehicle::getPassengerAmount() {
	Lock lock(&_vehicleMutex);

	int passengerCount = 0;
	for (int i = 0; i < _seatAmount; i++) {
		if (_passenger[i] != nullptr)
			passengerCount++;
	}
	return passengerCount;
}

void Vehicle::setPosition(int n, float position) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= 3)
		return;

	_position[n] = position;
}

float Vehicle::getPosition(int n) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= 3)
		return 0.f;

	return _position[n];
}

void Vehicle::setSpeed(int n, float speed) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= 3)
		return;

	_speed[n] = speed;
}

float Vehicle::getSpeed(int n) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= 3)
		return 0.f;

	return _speed[n];
}

void Vehicle::setQuaternion(int n, float quaternion) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= 4)
		return;

	_quaternion[n] = quaternion;
}

float Vehicle::getQuaternion(int n) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= 4)
		return 0.f;

	return _quaternion[n];
}

void Vehicle::setCarHealth(float carHealth) {
	Lock lock(&_vehicleMutex);

	_carHealth = carHealth;
}

float Vehicle::getCarHealth() {
	Lock lock(&_vehicleMutex);

	return _carHealth;
}

std::string Vehicle::getName() {
	Lock lock(&_vehicleMutex);

	return _name;
}

void Vehicle::setDriver(Player *driver) {
	Lock lock(&_vehicleMutex);

	_driver = driver;
}

Player *Vehicle::getDriver() {
	Lock lock(&_vehicleMutex);

	return _driver;
}

void Vehicle::setPassenger(int n, Player *passenger) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n > 5)
		return;

	_passenger[n] = passenger;
}

Player *Vehicle::getPassenger(int n) {
	Lock lock(&_vehicleMutex);

	if (n < 0 || n >= _seatAmount)
		return nullptr;

	return _passenger[n];
}

// GEAR STATE
void Vehicle::setGearState(uint8_t gearState) {
	Lock lock(&_vehicleMutex);

	_gearState = gearState;
}

uint8_t Vehicle::getGearState() {
	Lock lock(&_vehicleMutex);

	return _gearState;
}

// TRAILER ID
void Vehicle::setTrailerId(uint16_t trailerId) {
	Lock lock(&_vehicleMutex);

	_trailerId = trailerId;
}

uint16_t Vehicle::getTrailerId() {
	Lock lock(&_vehicleMutex);

	return _trailerId;
}

// TRAIN SPEED
void Vehicle::setTrainSpeed(float trainSpeed) {
	Lock lock(&_vehicleMutex);

	_trainSpeed = trainSpeed;
}

float Vehicle::getTrainSpeed() {
	Lock lock(&_vehicleMutex);

	return _trainSpeed;
}