#include "StdAfx.h"

#include "Player.h"

Player::Player() : PlayerBase() {
	// reset();
}

Player::~Player() {

}

void Player::reset() {
	lock();
	_inStream = false;
	_admin = false;
	unlock();

	PlayerBase::reset();
}

// IN STREAM
void Player::setInStream(bool inStream) {
	lock();
	_inStream = inStream;
	unlock();
}

bool Player::isInStream() {
	return _inStream;
}

// ADMIN
void Player::setAdmin(bool admin) {
	lock();
	_admin = admin;
	unlock();
}

bool Player::isAdmin() {
	return _admin;
}
