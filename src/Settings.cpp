#include "RakBot.h"

#include "Settings.h"

Vars vars;

Settings::Settings() : Mutex() {}

Settings::~Settings() {}

void Settings::reset() {
	getAddress()->reset();
}

void Settings::setName(std::string name) {
	lock();
	_name = name;
	unlock();
}

std::string Settings::getName() {
	return _name;
}

void Settings::setLoginPassword(std::string loginPassword) {
	lock();
	_loginPassword = loginPassword;
	unlock();
}

std::string Settings::getLoginPassword() {
	return _loginPassword;
}

void Settings::setServerPassword(std::string serverPassword) {
	lock();
	_serverPassword = serverPassword;
	unlock();
}

std::string Settings::getServerPassword() {
	return _serverPassword;
}