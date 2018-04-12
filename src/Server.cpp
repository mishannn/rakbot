#include "StdAfx.h"

#include "Server.h"

Server::Server() {}

Server::~Server() {}

void Server::reset() {
	_gameInited = false;
}

void Server::setGameInited(bool gameInited) {
	Lock lock(&_serverMutex);
	_gameInited = gameInited;
}

bool Server::isGameInited() {
	Lock lock(&_serverMutex);
	return _gameInited;
}

void Server::setServerName(std::string serverName) {
	Lock lock(&_serverMutex);
	_serverName = serverName;
}

std::string Server::getServerName() {
	Lock lock(&_serverMutex);
	return _serverName;
}