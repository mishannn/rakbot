#pragma once

#include "Mutex.h"

class Server {
private:
	bool _gameInited;
	std::string _serverName;

	Mutex _serverMutex;

public:
	Server();
	~Server();

	void reset();

	void setGameInited(bool gameInited);
	bool isGameInited();

	void setServerName(std::string serverName);
	std::string getServerName();
};
