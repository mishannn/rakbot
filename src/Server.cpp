#include "RakBot.h"

#include "Server.h"

Server::Server() {}

Server::~Server() {}

void Server::reset() {
	lock();
	unlock();
}

void Server::setServerName(std::string serverName) {
	lock();
	_serverName = serverName;
	unlock();
}

std::string Server::getServerName() {
	return _serverName;
}