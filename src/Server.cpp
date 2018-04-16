#include "StdAfx.h"

#include "Server.h"

Server::Server() {}

Server::~Server() {}

void Server::reset() {
	_gameInited = false;
}

void Server::setGameInited(bool gameInited) {
	_gameInited = gameInited;
}

bool Server::isGameInited() {
	return _gameInited;
}

void Server::setServerName(std::string serverName) {
	_serverName = serverName;
}

std::string Server::getServerName() {
	return _serverName;
}