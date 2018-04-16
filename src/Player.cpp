#include "StdAfx.h"

#include "Player.h"

Player::Player() : PlayerBase() {
	// reset();
}

Player::~Player() {

}

void Player::reset() {
	_inStream = false;
	_admin = false;

	PlayerBase::reset();
}

// IN STREAM
void Player::setInStream(bool inStream) {
	_inStream = inStream;
}

bool Player::isInStream() {
	return _inStream;
}

// ADMIN
void Player::setAdmin(bool admin) {
	_admin = admin;
}

bool Player::isAdmin() {
	return _admin;
}
