#include "StdAfx.h"

#include "Settings.h"

Vars vars;

Settings::Settings() {}

Settings::~Settings() {}

void Settings::reset() {
	Lock lock(&_settingsMutex);

	getAddress()->reset();
}

void Settings::setName(std::string name) {
	Lock lock(&_settingsMutex);

	_name = name;
}

std::string Settings::getName() {
	Lock lock(&_settingsMutex);

	return _name;
}

void Settings::setLoginPassword(std::string loginPassword) {
	Lock lock(&_settingsMutex);

	_loginPassword = loginPassword;
}

std::string Settings::getLoginPassword() {
	Lock lock(&_settingsMutex);

	return _loginPassword;
}

void Settings::setServerPassword(std::string serverPassword) {
	Lock lock(&_settingsMutex);

	_serverPassword = serverPassword;
}

std::string Settings::getServerPassword() {
	Lock lock(&_settingsMutex);

	return _serverPassword;
}