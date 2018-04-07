#pragma once

#include "PlayerBase.h"

class Bot : public PlayerBase {
private:
	bool _connected;
	bool _spawned;
	int _money;

	void aimSync();
	void onfootSync();
	void passengerSync();
	void driverSync();
	void spectateSync();

	Mutex _botMutex;

public:
	Bot();
	~Bot();

	void reset(bool reconnect);
	void reconnect(int reconnectDelay);

	void setConnected(bool connected);
	bool isConnected();

	void setSpawned(bool spawned);
	bool isSpawned();

	void setMoney(int money);
	int getMoney();

	void enterVehicle(Vehicle *vehicle, uint8_t seatId);
	void exitVehicle();

	void sync();
	void teleport(float fPosX, float fPosY, float fPosZ);
	void follow(uint16_t playerId);

	void connect(std::string address, uint16_t port);
	void disconnect(bool timeOut);

	void sendInput(std::string input);
	void requestClass(int classId);
	void requestSpawn();
	bool pickUpPickup(Pickup *pickup, bool checkDist = true);
	bool takeCheckpoint();
	void dialogResponse(uint16_t dialogId, uint8_t button = 1, uint16_t item = 0, std::string input = std::string());
	void spawn();
	void kill();
	void wait(int ms) { Sleep(ms); }
	void clickTextdraw(uint16_t textDrawId);
};