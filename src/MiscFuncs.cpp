#include "RakBot.h"

#include "RakBot.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "RakNet.h"
#include "Settings.h"
#include "Pickup.h"
#include "Vehicle.h"

#include "VehicleStuff.h"
#include "MiscFuncs.h"

// following functions

int FindNearestVehicle() {
	Bot *bot = RakBot::app()->getBot();

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	if (rakClient == nullptr)
		return VEHICLE_ID_NONE;

	if (!bot->isSpawned())
		return VEHICLE_ID_NONE;

	int iVehicleId = VEHICLE_ID_NONE;
	float fDist = -1.0f;

	for (uint16_t i = 1; i < MAX_VEHICLES; i++) {
		Vehicle *vehicle = RakBot::app()->getVehicle(i);
		if (vehicle == nullptr)
			continue;

		float fTempDist = bot->distanceTo(vehicle);

		if (fTempDist < fDist || fDist < 0.0f) {
			iVehicleId = i;
			fDist = fTempDist;
		}
	}

	return iVehicleId;
}

int FindNearestPickup(int model) {
	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return PICKUP_ID_NONE;

	int iPickupID = PICKUP_ID_NONE;
	float fDist = -1.0f;

	for (int i = 1; i < MAX_PICKUPS; i++) {
		Pickup *pickup = RakBot::app()->getPickup(i);

		if (pickup == nullptr)
			continue;

		if (pickup->getModel() != model)
			continue;

		float fTempDist = bot->distanceTo(pickup);

		if (fTempDist < fDist || fDist < 0.0f) {
			iPickupID = i;
			fDist = fTempDist;
		}
	}

	return iPickupID;
}

int FindNearestPlayer() {
	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return PLAYER_ID_NONE;

	int playerId = PLAYER_ID_NONE;
	float fDist = -1.0f;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		Player *player = RakBot::app()->getPlayer(i);
		if (player == nullptr)
			continue;

		if (!player->isInStream())
			continue;

		float fTempDist = bot->distanceTo(player);

		if (fTempDist < fDist || fDist < 0.0f) {
			playerId = i;
			fDist = fTempDist;
		}
	}

	return playerId;
}

int FindNearestVehicleByModel(int model) {
	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return VEHICLE_ID_NONE;

	int vehicleId = VEHICLE_ID_NONE;
	float fDist = -1.0f;

	for (int i = 1; i < MAX_VEHICLES; i++) {
		Vehicle *vehicle = RakBot::app()->getVehicle(i);
		if (vehicle == nullptr)
			continue;

		if (vehicle->getModel() != model)
			continue;

		float fTempDist = bot->distanceTo(vehicle);

		if (fTempDist < fDist || fDist < 0.0f) {
			vehicleId = i;
			fDist = fTempDist;
		}
	}

	return vehicleId;
}

int GetPlayerID(char *name) {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		Player *player = RakBot::app()->getPlayer(i);
		if (player == nullptr)
			continue;

		if (player->getName() == name)
			return i;
	}

	return -1;
}


char *GenRandom(char *s, const int len) {
	static const char alphanum[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	for (int i = 0; i < len; ++i)
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

	s[len] = 0;
	return s;
}

char *GetRakBotPath() {
	static char path[MAX_PATH];
	GetModuleFileName(NULL, path, sizeof(path));
	*(strrchr(path, '\\')) = 0;
	return path;
}

char *GetRakBotPath(char *append) {
	static char path[MAX_PATH];
	GetModuleFileName(NULL, path, sizeof(path));

	*(strrchr(path, '\\') + 1) = 0;
	strcat(path, append);

	return path;
}

std::string UrlEncode(const std::string &s) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = s.begin(), n = s.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}

void LTrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

void RTrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

void Trim(std::string &s) {
	LTrim(s);
	RTrim(s);
}

std::vector<std::string> Split(const std::string &s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
		// elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
	}
	return elems;
}