#include "RakBot.h"
#include "MathStuff.h"
#include "Pickup.h"
#include "Vehicle.h"

#include "PlayerBase.h"

PlayerBase::PlayerBase() : Mutex() {
}

PlayerBase::~PlayerBase() {
}

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
	lock();
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
	unlock();

	getInfo()->reset();
	getAnimation()->reset();
	getKeys()->reset();
	getSurfing()->reset();
}


// PLAYER STATE
void PlayerBase::setPlayerState(uint8_t playerState) {
	lock();
	_playerState = playerState;
	unlock();
}

uint8_t PlayerBase::getPlayerState() {
	return _playerState;
}

// HEALTH
void PlayerBase::setHealth(uint8_t playerState) {
	lock();
	_health = playerState;
	unlock();
}

uint8_t PlayerBase::getHealth() {
	return _health;
}

// ARMOUR
void PlayerBase::setArmour(uint8_t armour) {
	lock();
	_armour = armour;
	unlock();
}

uint8_t PlayerBase::getArmour() {
	return _armour;
}

// WEAPON
void PlayerBase::setWeapon(uint8_t weapon) {
	lock();
	_weapon = weapon;
	unlock();
}

uint8_t PlayerBase::getWeapon() {
	return _weapon;
}

// SPECIAL ACTION
void PlayerBase::setSpecialAction(uint8_t specialAction) {
	lock();
	_specialAction = specialAction;
	unlock();
}

uint8_t PlayerBase::getSpecialAction() {
	return _specialAction;
}

// PLAYER ID
void PlayerBase::setPlayerId(uint16_t playerId) {
	lock();
	_playerId = playerId;
	unlock();
}

uint16_t PlayerBase::getPlayerId() {
	return _playerId;
}

// SKIN
void PlayerBase::setSkin(int skin) {
	lock();
	_skin = skin;
	unlock();
}

int PlayerBase::getSkin() {
	return _skin;
}

// POSITION
void PlayerBase::setPosition(int n, float val) {
	if (n < 0 || n > 2)
		return;

	lock();
	_position[n] = val;
	unlock();
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

	lock();
	_quaternion[n] = val;
	unlock();
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

	lock();
	_speed[n] = val;
	unlock();
}

float PlayerBase::getSpeed(int n) {
	if (n < 0 || n > 2)
		return 0.f;

	return _speed[n];
}

// NAME
void PlayerBase::setName(std::string name) {
	lock();
	_name = name;
	unlock();
}

std::string PlayerBase::getName() {
	return _name;
}

// VEHICLE SEAT
void PlayerBase::setVehicleSeat(uint8_t vehicleSeat) {
	lock();
	_vehicleSeat = vehicleSeat;
	unlock();
}

uint8_t PlayerBase::getVehicleSeat() {
	return _vehicleSeat;
}

// VEHICLE
void PlayerBase::setVehicle(Vehicle *vehicle) {
	lock();
	_vehicle = vehicle;
	unlock();
}

Vehicle *PlayerBase::getVehicle() {
	return _vehicle;
}