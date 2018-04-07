#pragma once

#include "Includes.h"
#include "Defines.h"
#include "Structs.h"

class RakClientInterface;
class Bot;
class Player;
class Settings;
class Pickup;
class Server;
class Events;
class SAMPDialog;
class Vehicle;
class Mutex;

enum MutexEnum {
	MUTEX_RUNCOMMAND,
	MUTEX_LOG,
	MUTEX_LOGTOFILE,
	MutexesAmount
};

class RakBot {
private:
	RakClientInterface *_rakClient;
	Player *_players[MAX_PLAYERS];
	Pickup *_pickups[MAX_PICKUPS];
	Vehicle *_vehicles[MAX_VEHICLES];
	Bot *_bot;
	Settings *_settings;
	Server *_server;
	Events *_events;
	SAMPDialog *_sampDialog;
	Mutex *_mutexes[MutexesAmount];

	RakBot();
	~RakBot();

	RakBot(RakBot const&);
	RakBot& operator= (RakBot const&);

public:
	static RakBot *app();

	void exit();

	Player *getPlayer(uint16_t playerId);
	Player *addPlayer(uint16_t playerId);
	void deletePlayer(uint16_t playerId);
	uint16_t getPlayersCount();

	Pickup *getPickup(int pickupId);
	Pickup *addPickup(int pickupId);
	void deletePickup(int pickupId);

	Vehicle *getVehicle(uint16_t vehicleId);
	Vehicle *addVehicle(uint16_t vehicleId);
	void deleteVehicle(uint16_t vehicleId);

	Bot *getBot();
	Settings *getSettings();
	RakClientInterface *getRakClient();
	Server *getServer();
	Events *getEvents();
	SAMPDialog *getSampDialog();

	Mutex *getMutex(int mutexIndex);

	void log(const char *format, ...);
	void logToFile(std::string line);
};