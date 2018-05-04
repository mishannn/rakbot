#include "StdAfx.h"

#include "Server.h"

Server::Server() {}

Server::~Server() {}

void Server::reset() {
	std::lock_guard<std::mutex> lock(_serverMutex);
	_gameInited = false;
}

void Server::setGameInited(bool gameInited) {
	std::lock_guard<std::mutex> lock(_serverMutex);
	_gameInited = gameInited;
}

bool Server::isGameInited() {
	std::lock_guard<std::mutex> lock(_serverMutex);
	return _gameInited;
}

void Server::setServerName(std::string serverName) {
	std::lock_guard<std::mutex> lock(_serverMutex);
	_serverName = serverName;
}

std::string Server::getServerName() {
	std::lock_guard<std::mutex> lock(_serverMutex);
	return _serverName;
}