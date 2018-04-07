#include "StdAfx.h"

#include "Server.h"

Server::Server() {}

Server::~Server() {}

void Server::reset() {
}

void Server::setServerName(std::string serverName) {
	Lock lock(&_serverMutex);

	_serverName = serverName;
}

std::string Server::getServerName() {
	Lock lock(&_serverMutex);

	return _serverName;
}