#pragma once

#include "Defines.h"

#include "Mutex.h"

#include <string>

class Pickup;
class Vehicle;

enum PlayerState {
	PLAYER_STATE_NONE,
	PLAYER_STATE_ONFOOT,
	PLAYER_STATE_DRIVER,
	PLAYER_STATE_PASSENGER,
	PLAYER_STATE_SPECTATE
};

class PlayerKeys {
private:
	uint16_t _leftRightKey;
	uint16_t _upDownKey;
	uint16_t _keys;

	Mutex _playerKeysMutex;

public:
	PlayerKeys() {
		// reset();
	}

	void reset() {
		Lock lock(&_playerKeysMutex);
		_leftRightKey = 0;
		_upDownKey = 0;
		_keys = 0;
	}

	void setLeftRightKey(uint16_t leftRightKey) {
		Lock lock(&_playerKeysMutex);
		_leftRightKey = leftRightKey;
	}

	uint16_t getLeftRightKey() {
		Lock lock(&_playerKeysMutex);
		return _leftRightKey;
	}

	void setUpDownKey(uint16_t upDownKey) {
		Lock lock(&_playerKeysMutex);
		_upDownKey = upDownKey;
	}

	uint16_t getUpDownKey() {
		Lock lock(&_playerKeysMutex);
		return _upDownKey;
	}

	void setKeyId(uint16_t keys) {
		Lock lock(&_playerKeysMutex);
		_keys = keys;
	}

	uint16_t getKeyId() {
		Lock lock(&_playerKeysMutex);
		return _keys;
	}
};

class PlayerAnimation {
private:
	uint16_t _animId;
	uint16_t _animFlags;

	Mutex _playerAnimMutex;

public:
	PlayerAnimation() {
		// reset();
	}

	void reset() {
		Lock lock(&_playerAnimMutex);
		_animId = 1189;
		_animFlags = 33000;
	}

	void setAnimId(uint16_t animId) {
		Lock lock(&_playerAnimMutex);
		_animId = animId;
	}

	uint16_t getAnimId() {
		Lock lock(&_playerAnimMutex);
		return _animId;
	}

	void setAnimFlags(uint16_t animFlags) {
		Lock lock(&_playerAnimMutex);
		_animFlags = animFlags;
	}

	uint16_t getAnimFlags() {
		Lock lock(&_playerAnimMutex);
		return _animFlags;
	}
};

struct PlayerInfo {
private:
	int _score;
	int _ping;

	Mutex _playerInfoMutex;

public:
	PlayerInfo() {
		// reset();
	}

	void reset() {
		Lock lock(&_playerInfoMutex);
		_score = 0;
		_ping = 0;
	}

	void setScore(int score) {
		Lock lock(&_playerInfoMutex);
		_score = score;
	}

	int getScore() {
		Lock lock(&_playerInfoMutex);
		return _score;
	}

	void setPing(int ping) {
		Lock lock(&_playerInfoMutex);
		_ping = ping;
	}

	int getPing() {
		Lock lock(&_playerInfoMutex);
		return _ping;
	}
};

struct PlayerSurfing {
private:
	uint16_t _vehicleId;
	float _surfOffsets[3];

	Mutex _playerSurfingMutex;

public:
	PlayerSurfing() {
		// reset();
	}

	void reset() {
		Lock lock(&_playerSurfingMutex);

		_vehicleId = 0;

		for (int i = 0; i < 3; i++)
			_surfOffsets[i] = 0.f;
	}

	void setVehicleId(uint16_t vehicleId) {
		Lock lock(&_playerSurfingMutex);
		_vehicleId = vehicleId;
	}

	uint16_t getVehicleId() {
		Lock lock(&_playerSurfingMutex);
		return _vehicleId;
	}

	void setOffsets(int n, float val) {
		Lock lock(&_playerSurfingMutex);

		if (n < 0 || n > 2)
			return;

		_surfOffsets[n] = val;
	}

	float getOffset(int n) {
		Lock lock(&_playerSurfingMutex);

		if (n < 0 || n > 2)
			return 0.f;

		return _surfOffsets[n];
	}
};

class PlayerPassenger {
private:
	uint8_t	_seatId;
	uint16_t _vehicleId;
	
	Mutex _playerPassengerMutex;

public:
	PlayerPassenger() {
		// reset();
	}

	void reset() {
		Lock lock(&_playerPassengerMutex);
		_seatId = 0;
		_vehicleId = VEHICLE_ID_NONE;
	}

	void setSeatId(uint8_t seatId) {
		Lock lock(&_playerPassengerMutex);
		_seatId = seatId;
	}

	uint8_t getSeatId() {
		Lock lock(&_playerPassengerMutex);
		return _seatId;
	}

	void setVehicleId(uint16_t vehicleId) {
		Lock lock(&_playerPassengerMutex);
		_vehicleId = vehicleId;
	}

	uint16_t getVehicleId() {
		Lock lock(&_playerPassengerMutex);
		return _vehicleId;
	}
};

class PlayerBase {
private:
	uint8_t _playerState;
	uint8_t _health;
	uint8_t _armour;
	uint8_t _weapon;
	uint8_t _specialAction;
	uint16_t _playerId;
	int _skin;
	float _position[3];
	float _quaternion[4];
	float _speed[3];
	std::string _name;

	uint8_t _vehicleSeat;
	Vehicle *_vehicle;

	PlayerKeys _keys;
	PlayerAnimation _anim;
	PlayerInfo _info;
	PlayerSurfing _surfing;

	Mutex _playerBaseMutex;

	PlayerBase(PlayerBase const&);
	PlayerBase& operator= (PlayerBase const&);

protected:
	PlayerBase();
	~PlayerBase();

public:

	void reset();

	PlayerKeys *getKeys() { return &_keys; }
	PlayerAnimation *getAnimation() { return &_anim; }
	PlayerInfo *getInfo() { return &_info; }
	PlayerSurfing *getSurfing() { return &_surfing; }

	void setPlayerState(uint8_t playerState);
	uint8_t getPlayerState();

	void setHealth(uint8_t health);
	uint8_t getHealth();

	void setArmour(uint8_t armour);
	uint8_t getArmour();

	void setWeapon(uint8_t weapon);
	uint8_t getWeapon();

	void setSpecialAction(uint8_t specialAction);
	uint8_t getSpecialAction();

	void setPlayerId(uint16_t playerId);
	uint16_t getPlayerId();

	void setSkin(int skin);
	int getSkin();

	void setPosition(int n, float val);
	float getPosition(int n);

	void setQuaternion(int n, float val);
	float getQuaternion(int n);

	void setSpeed(int n, float val);
	float getSpeed(int n);

	void setName(std::string name);
	std::string getName();

	void setVehicleSeat(uint8_t vehicleSeat);
	uint8_t getVehicleSeat();

	void setVehicle(Vehicle *vehicle);
	Vehicle *getVehicle();

	float distanceTo(float position[3]);
	float distanceTo(PlayerBase *player);
	float distanceTo(Pickup *pickup);
	float distanceTo(Vehicle *vehicle);

	std::string getPlayerStateName();
};