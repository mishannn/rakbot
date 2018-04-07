#pragma once

#include "PlayerBase.h"

class Player : public PlayerBase {
private:
	bool _inStream;
	bool _admin;

	Mutex _playerMutex;

public:
	Player();
	~Player();

	void reset();

	void setInStream(bool admin);
	bool isInStream();

	void setAdmin(bool admin);
	bool isAdmin();
};