#pragma once

#include "Includes.h"
#include "Defines.h"
#include "Structs.h"

#include "Mutex.h"

class RakClientInterface;
class Bot;
class Player;
class Settings;
class Pickup;
class Server;
class Events;
class SAMPDialog;
class Vehicle;

class RakBot : private Mutex {
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

	void log(std::string format, ...);
	void logToFile(std::string line);
};