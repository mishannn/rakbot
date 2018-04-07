#pragma once


class Server {
private:
	std::string _serverName;

public:
	Server();
	~Server();

	void reset();

	void setServerName(std::string serverName);
	std::string getServerName();
};
