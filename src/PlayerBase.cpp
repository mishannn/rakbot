#include "StdAfx.h"

#include "MathStuff.h"
#include "Pickup.h"
#include "Vehicle.h"

#include "PlayerBase.h"

PlayerBase::PlayerBase() {}

PlayerBase::~PlayerBase() {}

float PlayerBase::distanceTo(float position[3]) {
	return vect3_dist(_position, position);
}

float PlayerBase::distanceTo(PlayerBase * player) {
	if (player == nullptr)
		return 0.f;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = player->getPosition(i);

	return vect3_dist(_position, position);
}

float PlayerBase::distanceTo(Pickup * pickup) {
	if (pickup == nullptr)
		return 0.f;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = pickup->getPosition(i);

	return vect3_dist(_position, position);
}

float PlayerBase::distanceTo(Vehicle * vehicle) {
	if (vehicle == nullptr)
		return 0.f;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = vehicle->getPosition(i);

	return vect3_dist(_position, position);
}

std::string PlayerBase::getPlayerStateName() {
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
	_playerState = playerState;
}

uint8_t PlayerBase::getPlayerState() {
	return _playerState;
}

// HEALTH
void PlayerBase::setHealth(uint8_t playerState) {
	_health = playerState;
}

uint8_t PlayerBase::getHealth() {
	return _health;
}

// ARMOUR
void PlayerBase::setArmour(uint8_t armour) {
	_armour = armour;
}

uint8_t PlayerBase::getArmour() {
	return _armour;
}

// WEAPON
void PlayerBase::setWeapon(uint8_t weapon) {
	_weapon = weapon;
}

uint8_t PlayerBase::getWeapon() {
	return _weapon;
}

// SPECIAL ACTION
void PlayerBase::setSpecialAction(uint8_t specialAction) {
	_specialAction = specialAction;
}

uint8_t PlayerBase::getSpecialAction() {
	return _specialAction;
}

// PLAYER ID
void PlayerBase::setPlayerId(uint16_t playerId) {
	_playerId = playerId;
}

uint16_t PlayerBase::getPlayerId() {
	return _playerId;
}

// SKIN
void PlayerBase::setSkin(int skin) {
	_skin = skin;
}

int PlayerBase::getSkin() {
	return _skin;
}

// POSITION
void PlayerBase::setPosition(int n, float val) {
	if (n < 0 || n > 2)
		return;

	_position[n] = val;
}

float PlayerBase::getPosition(int n) {
	if (n < 0 || n > 2)
		return 0.f;

	return _position[n];
}

// QUATERNION
void PlayerBase::setQuaternion(int n, float val) {
	if (n < 0 || n > 3)
		return;

	_quaternion[n] = val;
}

float PlayerBase::getQuaternion(int n) {
	if (n < 0 || n > 3)
		return 0.f;

	return _quaternion[n];
}

// SPEED
void PlayerBase::setSpeed(int n, float val) {
	if (n < 0 || n > 2)
		return;

	_speed[n] = val;
}

float PlayerBase::getSpeed(int n) {
	if (n < 0 || n > 2)
		return 0.f;

	return _speed[n];
}

// NAME
void PlayerBase::setName(std::string name) {
	_name = name;
}

std::string PlayerBase::getName() {
	return _name;
}

// VEHICLE SEAT
void PlayerBase::setVehicleSeat(uint8_t vehicleSeat) {
	_vehicleSeat = vehicleSeat;
}

uint8_t PlayerBase::getVehicleSeat() {
	return _vehicleSeat;
}

// VEHICLE
void PlayerBase::setVehicle(Vehicle *vehicle) {
	_vehicle = vehicle;
}

Vehicle *PlayerBase::getVehicle() {
	return _vehicle;
}