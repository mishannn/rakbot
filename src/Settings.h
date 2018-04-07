#pragma once

#include "Includes.h"
#include "Defines.h"
#include "Structs.h"

#include "Mutex.h"
#include "Timer.h"

struct Vars {
	bool windowOpened;
	bool waitForRequestSpawnReply;
	Timer lastChangePos;

	std::vector<std::string> admins;
	std::string adminsUrl;

	bool farChatEnabled;

	bool botAutoSchoolEnabled;
	bool botAutoSchoolActive;
	bool botAutoSchoolFinished;

	bool ignoreServerMessages;
	bool floodEnabled;
	int floodMode;
	int floodDelay;
	std::string floodText;

	int reconnectDelay;
	int adminReconnectDelay;

	int iBankPutMoney;
	bool bBuySkin;

	bool getBalanceEnabled;
	bool getBalanceFinished;

	int busWorkerRoute;
	int busWorkerRouteItem;
	int busWorkerBusModel;
	bool checkPointMaster;
	bool timeStamp;

	int iSetWorkIndex;

	bool parseStatistic;

	bool botFarmerEnabled;
	bool botFarmerAutomated;

	bool virtualWorld;
	int adminActionNear;
	int adminActionOnline;
	int skipDialog;

	int dialogIdBan;
	int dialogIdPassword;

	float coordMasterHeight;
	float coordMasterDist;
	int coordMasterDelay;

	int maxId;
	int minId;
	bool checkIdEnabled;

	int iInteriorID;

	bool bQuestEnabled;
	int iQuestStep;
	bool bQuestSpawn;

	int maxOnline;
	int minOnline;
	bool checkOnlineEnabled;

	// Follow & Stick
	int followPlayerID;
	bool stickEnabled;
	bool followEnabled;

	// AutoReger
	int autoRegEnabled;
	std::string autoRegMail;
	std::string autoRegReferer;
	int autoRegSex;

	bool autoPickEnabled;
	bool antiDeath;
	bool bAntiSat;

	// CoordMaster
	bool coordMasterEnabled;
	float coordMasterTarget[3];

	// Register
	std::string regKey;
	bool keyAccepted;

	float savedCoords[3];
	bool savedTeleportEnabled;

	// AntiAFK
	bool noAfk;
	bool syncAllowed;
	bool antiAfkEnabled;
	uint32_t antiAfkDelay;
	float antiAfkOffset;

	bool botLoaderEnabled;
	int botLoaderDelay;
	int botLoaderCount;
	int botLoaderGetPay;
	bool botLoaderCheckVans;

	unsigned int uiChallenge;

	bool sleepEnabled;

	bool keepOnlineEnabled;
	bool keepOnlineWait;
	bool keepOnlineBeginAfterEnd;
	int keepOnlineBegin;
	int keepOnlineEnd;

	bool botOff;
	uint32_t dwLicenseCheckResult;

	std::string resourcePath;

	bool routeEnabled;
	bool routeLoop;
	float routeSpeed;
	std::vector<OnfootData> routeData;
	uint32_t routeIndex;
	std::thread routeThread;

	bool textDrawCreateLogging;
	bool textDrawHideLogging;
	bool textDrawSetStringLogging;

	bool textLabelCreateLogging;

	bool sendBadSync;
	bool smartInvis;

	float faceAngle;
	std::string adapterAddress;

	int mainDelay;
	int luaUpdateDelay;
	int updateDelay;
	int spawnDelay;
	int dialogResponseDelay;
	int noAfkDelay;
	int afterSpawnDelay;
	int enterVehicleDelay;

	FILE *logFile;
	std::string logFileMode;

	bool mapWindowOpened;
	std::thread mapWindowThread;
};

extern Vars vars;

class Address {
private:
	std::string _ip;
	uint16_t _port;

	Mutex _addressMutex;

public:
	Address() {

	}

	void reset() {
		Lock lock(&_addressMutex);
		_ip = "127.0.0.1";
		_port = 7777;
	}

	void setIp(std::string ip) {
		Lock lock(&_addressMutex);
		_ip = ip;
	}

	std::string getIp() {
		Lock lock(&_addressMutex);
		return _ip;
	}

	void setPort(uint16_t port) {
		Lock lock(&_addressMutex);
		_port = port;
	}

	uint16_t getPort() {
		Lock lock(&_addressMutex);
		return _port;
	}
};

class Settings {
private:
	Address _address;
	std::string _name;
	std::string _serverPassword;
	std::string _loginPassword;

	Mutex _settingsMutex;

public:
	Settings();
	~Settings();

	void reset();

	Address *getAddress() { return &_address; }

	void setName(std::string name);
	std::string getName();

	void setLoginPassword(std::string loginPassword);
	std::string getLoginPassword();

	void setServerPassword(std::string serverPassword);
	std::string getServerPassword();
};