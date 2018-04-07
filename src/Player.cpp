#include "StdAfx.h"

#include "Player.h"

Player::Player() : PlayerBase() {
	// reset();
}

Player::~Player() {

}

void Player::reset() {
	Lock lock(&_playerMutex);

	_inStream = false;
	_admin = false;

	PlayerBase::reset();
}

// IN STREAM
void Player::setInStream(bool inStream) {
	Lock lock(&_playerMutex);

	_inStream = inStream;
}

bool Player::isInStream() {
	Lock lock(&_playerMutex);

	return _inStream;
}

// ADMIN
void Player::setAdmin(bool admin) {
	Lock lock(&_playerMutex);

	_admin = admin;
}

bool Player::isAdmin() {
	Lock lock(&_playerMutex);

	return _admin;
}
