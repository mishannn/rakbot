#pragma once

#include "Mutex.h"

class Server {
private:
	std::string _serverName;

	Mutex _serverMutex;

public:
	Server();
	~Server();

	void reset();

	void setServerName(std::string serverName);
	std::string getServerName();
};
