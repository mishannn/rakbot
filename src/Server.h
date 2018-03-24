#pragma once

#include "Mutex.h"

class Server : private Mutex {
private:
	std::string _serverName;

public:
	Server();
	~Server();

	void reset();

	void setServerName(std::string serverName);
	std::string getServerName();
};
