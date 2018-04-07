#include "StdAfx.h"

#include "MathStuff.h"
#include "Pickup.h"
#include "Vehicle.h"

#include "PlayerBase.h"

PlayerBase::PlayerBase() {}

PlayerBase::~PlayerBase() {}

float PlayerBase::distanceTo(float position[3]) {
	Lock lock(&_playerBaseMutex);

	return vect3_dist(_position, position);
}

float PlayerBase::distanceTo(PlayerBase * player) {
	Lock lock(&_playerBaseMutex);

	if (player == nullptr)
		return 0.f;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = player->getPosition(i);

	return vect3_dist(_position, position);
}

float PlayerBase::distanceTo(Pickup * pickup) {
	Lock lock(&_playerBaseMutex);

	if (pickup == nullptr)
		return 0.f;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = pickup->getPosition(i);

	return vect3_dist(_position, position);
}

float PlayerBase::distanceTo(Vehicle * vehicle) {
	Lock lock(&_playerBaseMutex);

	if (vehicle == nullptr)
		return 0.f;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = vehicle->getPosition(i);

	return vect3_dist(_position, position);
}

std::string PlayerBase::getPlayerStateName() {
	Lock lock(&_playerBaseMutex);

	std::string playerStateName;

	switch (_playerState) {
		case PLAYER_STATE_SPECTATE:
			playerStateName = "наблюдатель";
			break;

		case PLAYER_STATE_DRIVER:
			playerStateName = "водитель";
			break;

		case PLAYER_STATE_ONFOOT:
			playerStateName = "пешеход";
			break;

		case PLAYER_STATE_PASSENGER:
			playerStateName = "пассажир";
			break;

		default:
			playerStateName = "n/a";
			break;
	}

	return playerStateName;
}

void PlayerBase::reset() {
	Lock lock(&_playerBaseMutex);

	_playerState = PLAYER_STATE_NONE;
	_playerId = PLAYER_ID_NONE;
	_health = 0;
	_armour = 0;
	_weapon = 0;
	_specialAction = 0;
	_skin = 0;

	for (int i = 0; i < 3; i++)
		_position[i] = 0.f;

	for (int i = 0; i < 4; i++)
		_quaternion[i] = 0.f;

	for (int i = 0; i < 3; i++)
		_speed[i] = 0.f;

	_vehicleSeat = 0;
	_vehicle = nullptr;

	getInfo()->reset();
	getAnimation()->reset();
	getKeys()->reset();
	getSurfing()->reset();
}


// PLAYER STATE
void PlayerBase::setPlayerState(uint8_t playerState) {
	Lock lock(&_playerBaseMutex);

	_playerState = playerState;
}

uint8_t PlayerBase::getPlayerState() {
	Lock lock(&_playerBaseMutex);

	return _playerState;
}

// HEALTH
void PlayerBase::setHealth(uint8_t playerState) {
	Lock lock(&_playerBaseMutex);

	_health = playerState;
}

uint8_t PlayerBase::getHealth() {
	Lock lock(&_playerBaseMutex);

	return _health;
}

// ARMOUR
void PlayerBase::setArmour(uint8_t armour) {
	Lock lock(&_playerBaseMutex);

	_armour = armour;
}

uint8_t PlayerBase::getArmour() {
	Lock lock(&_playerBaseMutex);

	return _armour;
}

// WEAPON
void PlayerBase::setWeapon(uint8_t weapon) {
	Lock lock(&_playerBaseMutex);

	_weapon = weapon;
}

uint8_t PlayerBase::getWeapon() {
	Lock lock(&_playerBaseMutex);

	return _weapon;
}

// SPECIAL ACTION
void PlayerBase::setSpecialAction(uint8_t specialAction) {
	Lock lock(&_playerBaseMutex);

	_specialAction = specialAction;
}

uint8_t PlayerBase::getSpecialAction() {
	Lock lock(&_playerBaseMutex);

	return _specialAction;
}

// PLAYER ID
void PlayerBase::setPlayerId(uint16_t playerId) {
	Lock lock(&_playerBaseMutex);

	_playerId = playerId;
}

uint16_t PlayerBase::getPlayerId() {
	Lock lock(&_playerBaseMutex);

	return _playerId;
}

// SKIN
void PlayerBase::setSkin(int skin) {
	Lock lock(&_playerBaseMutex);

	_skin = skin;
}

int PlayerBase::getSkin() {
	Lock lock(&_playerBaseMutex);

	return _skin;
}

// POSITION
void PlayerBase::setPosition(int n, float val) {
	Lock lock(&_playerBaseMutex);

	if (n < 0 || n > 2)
		return;

	_position[n] = val;
}

float PlayerBase::getPosition(int n) {
	Lock lock(&_playerBaseMutex);

	if (n < 0 || n > 2)
		return 0.f;

	return _position[n];
}

// QUATERNION
void PlayerBase::setQuaternion(int n, float val) {
	Lock lock(&_playerBaseMutex);

	if (n < 0 || n > 3)
		return;

	_quaternion[n] = val;
}

float PlayerBase::getQuaternion(int n) {
	Lock lock(&_playerBaseMutex);

	if (n < 0 || n > 3)
		return 0.f;

	return _quaternion[n];
}

// SPEED
void PlayerBase::setSpeed(int n, float val) {
	Lock lock(&_playerBaseMutex);

	if (n < 0 || n > 2)
		return;

	_speed[n] = val;
}

float PlayerBase::getSpeed(int n) {
	Lock lock(&_playerBaseMutex);

	if (n < 0 || n > 2)
		return 0.f;

	return _speed[n];
}

// NAME
void PlayerBase::setName(std::string name) {
	Lock lock(&_playerBaseMutex);

	_name = name;
}

std::string PlayerBase::getName() {
	Lock lock(&_playerBaseMutex);

	return _name;
}

// VEHICLE SEAT
void PlayerBase::setVehicleSeat(uint8_t vehicleSeat) {
	Lock lock(&_playerBaseMutex);

	_vehicleSeat = vehicleSeat;
}

uint8_t PlayerBase::getVehicleSeat() {
	Lock lock(&_playerBaseMutex);

	return _vehicleSeat;
}

// VEHICLE
void PlayerBase::setVehicle(Vehicle *vehicle) {
	Lock lock(&_playerBaseMutex);

	_vehicle = vehicle;
}

Vehicle *PlayerBase::getVehicle() {
	Lock lock(&_playerBaseMutex);

	return _vehicle;
}