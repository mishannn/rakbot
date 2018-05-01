#pragma once

#include "PlayerBase.h"

class Player : public PlayerBase {
private:
	bool _active;
	bool _inStream;
	bool _admin;

public:
	Player();
	~Player();

	void reset();

	void setActive(bool active);
	bool isActive();

	void setInStream(bool admin);
	bool isInStream();

	void setAdmin(bool admin);
	bool isAdmin();
};