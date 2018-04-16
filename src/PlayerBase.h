#pragma once

#include "Defines.h"

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

public:
	PlayerKeys() {
		// reset();
	}

	void reset() {
		_leftRightKey = 0;
		_upDownKey = 0;
		_keys = 0;
	}

	void setLeftRightKey(uint16_t leftRightKey) {
		_leftRightKey = leftRightKey;
	}

	uint16_t getLeftRightKey() {
		return _leftRightKey;
	}

	void setUpDownKey(uint16_t upDownKey) {
		_upDownKey = upDownKey;
	}

	uint16_t getUpDownKey() {
		return _upDownKey;
	}

	void setKeyId(uint16_t keys) {
		_keys = keys;
	}

	uint16_t getKeyId() {
		return _keys;
	}
};

class PlayerAnimation {
private:
	uint16_t _animId;
	uint16_t _animFlags;

public:
	PlayerAnimation() {
		// reset();
	}

	void reset() {
		_animId = 1189;
		_animFlags = 33000;
	}

	void setAnimId(uint16_t animId) {
		_animId = animId;
	}

	uint16_t getAnimId() {
		return _animId;
	}

	void setAnimFlags(uint16_t animFlags) {
		_animFlags = animFlags;
	}

	uint16_t getAnimFlags() {
		return _animFlags;
	}
};

struct PlayerInfo {
private:
	int _score;
	int _ping;

public:
	PlayerInfo() {
		// reset();
	}

	void reset() {
		_score = 0;
		_ping = 0;
	}

	void setScore(int score) {
		_score = score;
	}

	int getScore() {
		return _score;
	}

	void setPing(int ping) {
		_ping = ping;
	}

	int getPing() {
		return _ping;
	}
};

struct PlayerSurfing {
private:
	uint16_t _vehicleId;
	float _surfOffsets[3];

public:
	PlayerSurfing() {
		// reset();
	}

	void reset() {
		_vehicleId = 0;

		for (int i = 0; i < 3; i++)
			_surfOffsets[i] = 0.f;
	}

	void setVehicleId(uint16_t vehicleId) {
		_vehicleId = vehicleId;
	}

	uint16_t getVehicleId() {
		return _vehicleId;
	}

	void setOffsets(int n, float val) {
		if (n < 0 || n > 2)
			return;

		_surfOffsets[n] = val;
	}

	float getOffset(int n) {
		if (n < 0 || n > 2)
			return 0.f;

		return _surfOffsets[n];
	}
};

class PlayerPassenger {
private:
	uint8_t	_seatId;
	uint16_t _vehicleId;

public:
	PlayerPassenger() {
		// reset();
	}

	void reset() {
		_seatId = 0;
		_vehicleId = VEHICLE_ID_NONE;
	}

	void setSeatId(uint8_t seatId) {
		_seatId = seatId;
	}

	uint8_t getSeatId() {
		return _seatId;
	}

	void setVehicleId(uint16_t vehicleId) {
		_vehicleId = vehicleId;
	}

	uint16_t getVehicleId() {
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