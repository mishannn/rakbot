#include "StdAfx.h"

#include "Settings.h"

Vars vars;

Settings::Settings() {}

Settings::~Settings() {}

void Settings::reset() {
	getAddress()->reset();
}

void Settings::setName(std::string name) {
	_name = name;
}

std::string Settings::getName() {
	return _name;
}

void Settings::setLoginPassword(std::string loginPassword) {
	_loginPassword = loginPassword;
}

std::string Settings::getLoginPassword() {
	return _loginPassword;
}

void Settings::setServerPassword(std::string serverPassword) {
	_serverPassword = serverPassword;
}

std::string Settings::getServerPassword() {
	return _serverPassword;
}