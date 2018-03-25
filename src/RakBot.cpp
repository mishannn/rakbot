#include "StdAfx.h"

#include "RakNet.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "Settings.h"
#include "Funcs.h"
#include "Pickup.h"
#include "Server.h"
#include "Events.h"
#include "SAMPDialog.h"
#include "Vehicle.h"
#include "Lock.h"

#include "window.h"

#include "RakBot.h"

RakBot::RakBot() : Mutex() {
	for (int i = 0; i < MAX_PLAYERS; i++)
		_players[i] = nullptr;

	_bot = new Bot();
	_bot->reset(true);

	_settings = new Settings();
	_settings->reset();

	_server = new Server();
	_server->reset();

	_events = new Events();
	_events->reset();

	_sampDialog = new SAMPDialog();
	_sampDialog->reset();

	_rakClient = RakNetworkFactory::GetRakClientInterface();
	_rakClient->SetMTUSize(576);
}

RakBot::~RakBot() {
	if (_rakClient != nullptr) {
		RakNetworkFactory::DestroyRakClientInterface(_rakClient);
		_rakClient = nullptr;
	}

	if (_sampDialog != nullptr) {
		delete _sampDialog;
		_sampDialog = nullptr;
	}

	if (_events != nullptr) {
		delete _events;
		_events = nullptr;
	}

	if (_settings != nullptr) {
		delete _settings;
		_settings = nullptr;
	}

	if (_server != nullptr) {
		delete _server;
		_server = nullptr;
	}

	if (_bot != nullptr) {
		delete _bot;
		_bot = nullptr;
	}

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (_players[i] != nullptr) {
			delete _players[i];
			_players[i] = nullptr;
		}
	}
}

RakBot *RakBot::app() {
	static RakBot rakBot;
	return &rakBot;
}

void RakBot::exit() {
	vars.botOff = true;
	log("[RAKBOT] Âûõîä èç áîòà...");
}

Player *RakBot::getPlayer(uint16_t playerId) {
	if (playerId < 0 || playerId >= MAX_PLAYERS)
		return nullptr;

	return _players[playerId];
}

Player *RakBot::addPlayer(uint16_t playerId) {
	if (playerId < 0 || playerId >= MAX_PLAYERS)
		return nullptr;

	if (_players[playerId] != nullptr)
		return _players[playerId];

	_players[playerId] = new Player;
	_players[playerId]->reset();
	_players[playerId]->setPlayerId(playerId);

	return _players[playerId];
}

void RakBot::deletePlayer(uint16_t playerId) {
	if (playerId < 0 || playerId >= MAX_PLAYERS)
		return;

	if (_players[playerId] == nullptr)
		return;

	delete _players[playerId];
	_players[playerId] = nullptr;
}

uint16_t RakBot::getPlayersCount() {
	if (!_bot->isConnected())
		return 0;

	uint16_t count = 1;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		Player *player = _players[i];
		if (player == nullptr)
			continue;
		count++;
	}

	return count;
}

Pickup *RakBot::getPickup(int pickupId) {
	if (pickupId < 0 || pickupId >= MAX_PICKUPS)
		return nullptr;

	return _pickups[pickupId];
}

Pickup *RakBot::addPickup(int pickupId) {
	if (pickupId < 0 || pickupId >= MAX_PICKUPS)
		return nullptr;

	if (_pickups[pickupId] != nullptr)
		return _pickups[pickupId];

	_pickups[pickupId] = new Pickup;
	_pickups[pickupId]->reset();
	_pickups[pickupId]->setPickupId(pickupId);
	return _pickups[pickupId];
}

void RakBot::deletePickup(int pickupId) {
	if (pickupId < 0 || pickupId >= MAX_PICKUPS)
		return;

	if (_pickups[pickupId] == nullptr)
		return;

	delete _pickups[pickupId];
	_pickups[pickupId] = nullptr;
}

Vehicle *RakBot::getVehicle(uint16_t vehicleId) {
	if (vehicleId < 1 || vehicleId >= MAX_VEHICLES)
		return nullptr;

	return _vehicles[vehicleId];
}

Vehicle *RakBot::addVehicle(uint16_t vehicleId) {
	if (vehicleId < 1 || vehicleId >= MAX_VEHICLES)
		return nullptr;

	if (_vehicles[vehicleId] != nullptr)
		return _vehicles[vehicleId];

	_vehicles[vehicleId] = new Vehicle;
	_vehicles[vehicleId]->reset();
	_vehicles[vehicleId]->setVehicleId(vehicleId);
	return _vehicles[vehicleId];
}

void RakBot::deleteVehicle(uint16_t vehicleId) {
	if (vehicleId < 1 || vehicleId >= MAX_VEHICLES)
		return;

	if (_vehicles[vehicleId] == nullptr)
		return;

	delete _vehicles[vehicleId];
	_vehicles[vehicleId] = nullptr;
}

Bot *RakBot::getBot() {
	return _bot;
}

Settings *RakBot::getSettings() {
	return _settings;
}

RakClientInterface *RakBot::getRakClient() {
	return _rakClient;
}

Server *RakBot::getServer() {
	return _server;
}

Events *RakBot::getEvents() {
	return _events;
}

SAMPDialog *RakBot::getSampDialog() {
	return _sampDialog;
}

void RakBot::log(const char *format, ...) {
	if (format == nullptr)
		return;

	if (strlen(format) < 1)
		return;

	Lock lock(vars.logMutex);

	char *buf = new char[MAX_LOGLEN + 1];

	std::va_list args;
	va_start(args, format);
	int bufLen = vsnprintf(buf, MAX_LOGLEN, format, args);
	buf[bufLen] = 0;
	va_end(args);

	if (RakBot::app()->getEvents()->onPrintLog(std::string(buf), false))
		return;

	logToFile(std::string(buf));

	if (vars.timeStamp) {
		SYSTEMTIME time;
		GetLocalTime(&time);
		
		char tempBuf[MAX_LOGLEN + 64];
		int bufLen = snprintf(tempBuf, MAX_LOGLEN, "[%02d:%02d:%02d] %s", time.wHour, time.wMinute, time.wSecond, buf);
		strncpy(buf, tempBuf, bufLen);
		buf[bufLen] = 0;
	}

	int lbCount = SendMessage(g_hWndLog, LB_GETCOUNT, 0, 0);
	if (lbCount >= MAX_LOGLINES)
		SendMessage(g_hWndLog, LB_DELETESTRING, 0, 0);

	WPARAM idx = SendMessage(g_hWndLog, LB_ADDSTRING, 0, (LPARAM)buf);
	SendMessage(g_hWndLog, LB_SETTOPINDEX, idx, 0);
}

void RakBot::logToFile(std::string line) {
	static Mutex logToFileMutex;
	Lock lock(logToFileMutex);

	if (vars.logFile == nullptr) {
		Settings *settings = RakBot::app()->getSettings();

		if (settings->getAddress()->getIp().empty())
			return;

		if (settings->getAddress()->getPort() < 0 || settings->getAddress()->getPort() > 65535)
			return;

		if (settings->getName().empty())
			return;

		char currentDirBuf[MAX_PATH];
		GetModuleFileName(NULL, currentDirBuf, sizeof(currentDirBuf));
		*strrchr(currentDirBuf, '\\') = 0;

		std::stringstream ss;
		ss << currentDirBuf << "\\logs";
		CreateDirectory(ss.str().c_str(), NULL);

		ss << "\\" << settings->getAddress()->getIp() << ";" << settings->getAddress()->getPort();
		CreateDirectory(ss.str().c_str(), NULL);

		ss << "\\" << settings->getName() << ".log";

		// MessageBox(NULL, ss.str().c_str(), "", NULL);

		vars.logFile = fopen(ss.str().c_str(), "a");
		if (vars.logFile == nullptr)
			return;

		fprintf(vars.logFile, "==================================[ÍÎÂÀß ÑÅÑÑÈß]==================================\n");
	}

	SYSTEMTIME time;
	GetLocalTime(&time);

	char timeBuf[64];
	_snprintf(timeBuf, sizeof(timeBuf), "[%02d:%02d:%02d]", time.wHour, time.wMinute, time.wSecond);

	fprintf(vars.logFile, "%s %s\n", timeBuf, line.c_str());
	fflush(vars.logFile);
}