#include "StdAfx.h"

#include "RakBot.h"
#include "Player.h"
#include "RakNet.h"
#include "Settings.h"
#include "Pickup.h"
#include "Script.h"
#include "Events.h"
#include "Server.h"
#include "Vehicle.h"

#include "Funcs.h"
#include "MathStuff.h"
#include "MiscFuncs.h"
#include "SampRpFuncs.h"

#include "main.h"
#include "netgame.h"
#include "netrpc.h"

#include "Bot.h"

Bot::Bot() : PlayerBase() {
	//reset();
}

Bot::~Bot() {

}

void Bot::setConnectRequested(bool connectRequested) {
	Lock lock(&_botMutex);

	_connectRequested = connectRequested;
}

bool Bot::isConnectRequested() {
	Lock lock(&_botMutex);

	return _connectRequested;
}

void Bot::reset(bool disconnect) {
	Lock lock(&_botMutex);

	uint16_t playerId;
	bool connected;

	if (!disconnect) {
		playerId = getPlayerId();
		connected = isConnected();
	}

	_connected = false;
	_spawned = false;
	_money = 0;

	PlayerBase::reset();

	if (!disconnect) {
		setPlayerId(playerId);
		setConnected(connected);
	}

	FuncsOff();
}

void Bot::reconnect(int reconnectDelay) {
	Lock lock(&_botMutex);

	if (isConnected()) {
		disconnect(false);
	}
	
	setConnectRequested(false);
	ReconnectTimer.setTimer(GetTickCount() + reconnectDelay);
	RakBot::app()->getEvents()->onReconnect(reconnectDelay);
}

// Connected
void Bot::setConnected(bool connected) {
	Lock lock(&_botMutex);

	_connected = connected;
}

bool Bot::isConnected() {
	Lock lock(&_botMutex);

	return _connected;
}

// Spawned
void Bot::setSpawned(bool spawned) {
	Lock lock(&_botMutex);

	_spawned = spawned;
}

bool Bot::isSpawned() {
	Lock lock(&_botMutex);

	return _spawned;
}

// Money
void Bot::setMoney(int money) {
	Lock lock(&_botMutex);

	_money = money;
}

int Bot::getMoney() {
	Lock lock(&_botMutex);

	return _money;
}

// FUNCS
void Bot::enterVehicle(Vehicle *vehicle, uint8_t seatId) {
	static bool enterVehicleReady = true;

	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Посадка в транспорт: бот должен быть подключен к серверу!");
		return;
	}

	if (!isSpawned()) {
		RakBot::app()->log("[ERROR] Посадка в транспорт: бот должен быть заспавнен!");
		return;
	}

	if (getPlayerState() != PLAYER_STATE_ONFOOT) {
		RakBot::app()->log("[ERROR] Посадка в транспорт: бот должен быть пешеходом!");
		return;
	}

	if (vehicle == nullptr) {
		RakBot::app()->log("[ERROR] Посадка в транспорт: транспорт недоступен!");
		return;
	}

	if (seatId < 0 || seatId >= vehicle->getSeatAmount()) {
		RakBot::app()->log("[ERROR] Посадка в транспорт: неверное посадочное место!");
		return;
	}

	if (!enterVehicleReady) {
		RakBot::app()->log("[ERROR] Посадка в транспорт: бот уже садится в транспорт!");
		return;
	}

	enterVehicleReady = false;

	teleport(vehicle->getPosition(0), vehicle->getPosition(1), vehicle->getPosition(2));

	RakBot::app()->getEvents()->defCallAdd(vars.enterVehicleDelay, false, [this, vehicle, seatId](DefCall *) {
		RakClientInterface *rakClient = RakBot::app()->getRakClient();
		RakNet::BitStream bsSend;
		bsSend.Write<uint16_t>(vehicle->getVehicleId());
		bsSend.Write(seatId);
		rakClient->RPC(&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

		setVehicleSeat(seatId);
		setVehicle(vehicle);

		if (seatId == 0) {
			setPlayerState(PLAYER_STATE_DRIVER);
		} else {
			setPlayerState(PLAYER_STATE_PASSENGER);
		}
		sync();

		RakBot::app()->getEvents()->onPutInVehicle(vehicle, seatId);
		RakBot::app()->log("[RAKBOT] Бот посажен в транспорт с ID %d на место %d!", vehicle->getVehicleId(), seatId);
		enterVehicleReady = true;
	});
}

void Bot::exitVehicle() {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Выход из транспорта: бот должен быть подключен к серверу!");
		return;
	}

	if (!isSpawned()) {
		RakBot::app()->log("[ERROR] Выход из транспорта: бот должен быть заспавнен!");
		return;
	}

	if (getPlayerState() != PLAYER_STATE_PASSENGER && getPlayerState() != PLAYER_STATE_DRIVER) {
		RakBot::app()->log("[ERROR] Выход из транспорта: бот должен быть водителем или пассажиром!");
		return;
	}

	Vehicle *vehicle = getVehicle();
	if (vehicle == nullptr) {
		RakBot::app()->log("[ERROR] Выход из транспорта: транспорт недоступен!");
		return;
	}

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	RakNet::BitStream bsSend;
	bsSend.Write(vehicle->getVehicleId());
	rakClient->RPC(&RPC_ExitVehicle, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	setPlayerState(PLAYER_STATE_ONFOOT);
	setVehicle(nullptr);
	setVehicleSeat(0);
	sync();

	RakBot::app()->getEvents()->onEjectFromVehicle();
	RakBot::app()->log("[RAKBOT] Бот вышел из транспорта!");
}

void Bot::sync() {
	Lock lock(&_botMutex);

	if (RakBot::app()->getEvents()->onSync())
		return;

	switch (getPlayerState()) {
		case PLAYER_STATE_ONFOOT:
			onfootSync();
			break;

		case PLAYER_STATE_SPECTATE:
			spectateSync();
			break;

		case PLAYER_STATE_DRIVER:
			driverSync();
			break;

		case PLAYER_STATE_PASSENGER:
			passengerSync();
			break;

		default:
			break;
	}
}

void Bot::aimSync() {
	if (!isSpawned())
		return;

	static uint32_t dwTick = 0;
	if (GetTickCount() - dwTick > 1000) {
		RakClientInterface *rakClient = RakBot::app()->getRakClient();

		RakNet::BitStream bsAimSync;
		AimData aimData;

		for (int i = 0; i < 3; i++)
			aimData.aimPos[i] = getPosition(i);

		Player *player = FindNearestPlayer();
		if (player == nullptr) {
			aimData.aimf1[0] = 0.f;
			aimData.aimf1[1] = 1.f;
		} else {
			aimData.aimf1[0] = cosf(player->getPosition(0) - getPosition(0));
			aimData.aimf1[1] = -sinf(player->getPosition(1) - getPosition(0));
		}

		aimData.camMode = 4;
		aimData.unknown = 85;
		aimData.weaponState = 63;
		aimData.aimf1[2] = 0.f;
		aimData.aimZ = 0;

		bsAimSync.Write((uint8_t)ID_AIM_SYNC);
		bsAimSync.Write((PCHAR)&aimData, sizeof(AimData));
		rakClient->Send(&bsAimSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		dwTick = GetTickCount();
	}
}

void Bot::onfootSync() {
	if (getPlayerState() != PLAYER_STATE_ONFOOT)
		return;

	if (!isSpawned())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	if (vars.smartInvis) {
		SpectatorData specSync;
		memset(&specSync, 0, sizeof(SpectatorData));

		for (int i = 0; i < 3; i++)
			specSync.fPosition[i] = getPosition(i);

		RakNet::BitStream bsSpecSync;
		bsSpecSync.Write((uint8_t)ID_SPECTATOR_SYNC);
		bsSpecSync.Write((PCHAR)&specSync, sizeof(SpectatorData));
		rakClient->Send(&bsSpecSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
		return;
	}

	RakNet::BitStream bsOnfootSync;
	bsOnfootSync.Write((uint8_t)ID_PLAYER_SYNC);

	OnfootData onfootSync;
	ZeroMemory(&onfootSync, sizeof(OnfootData));
	onfootSync.keys = getKeys()->getKeys();
	onfootSync.leftRightKey = getKeys()->getLeftRightKey();
	onfootSync.upDownKey = getKeys()->getUpDownKey();
	onfootSync.weapon = getWeapon();
	onfootSync.health = getHealth();
	onfootSync.armour = getArmour();
	onfootSync.animId = getAnimation()->getAnimId();
	onfootSync.animFlags = getAnimation()->getAnimFlags();
	onfootSync.surfVehicleId = getSurfing()->getVehicleId();

	for (int i = 0; i < 3; i++)
		onfootSync.surfOffsets[i] = getSurfing()->getOffset(i);

	for (int i = 0; i < 3; i++)
		onfootSync.position[i] = getPosition(i);

	for (int i = 0; i < 3; i++)
		onfootSync.speed[i] = getSpeed(i);

	for (int i = 0; i < 4; i++)
		onfootSync.quaternion[i] = getQuaternion(i);

	if (vars.sendBadSync) {
		Vehicle *vehicle = FindNearestVehicle();
		onfootSync.surfVehicleId = (vehicle != nullptr) ? vehicle->getVehicleId() : 1;
		onfootSync.surfOffsets[0] = NAN;
		onfootSync.surfOffsets[1] = NAN;
		onfootSync.surfOffsets[2] = NAN;
	}

	bsOnfootSync.Write((PCHAR)&onfootSync, sizeof(OnfootData));

	rakClient->Send(&bsOnfootSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
	aimSync();
}

void Bot::teleport(float positionX, float positionY, float positionZ) {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Телепорт: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Телепорт: игра должна быть инициализирована");
		return;
	}

	if (RakBot::app()->getEvents()->onTeleport(positionX, positionY, positionZ))
		return;

	setPosition(0, positionX);
	setPosition(1, positionY);
	setPosition(2, positionZ);
	sync();
}

void Bot::follow(uint16_t playerId) {
	Lock lock(&_botMutex);

	if (!isSpawned())
		return;

	if (getPlayerState() != PLAYER_STATE_ONFOOT)
		return;

	Player *player = RakBot::app()->getPlayer(playerId);
	if (player == nullptr)
		return;

	if (player->getPlayerState() != PLAYER_STATE_ONFOOT)
		return;

	for (int i = 0; i < 3; i++)
		setPosition(i, player->getPosition(i));

	for (int i = 0; i < 3; i++)
		setSpeed(i, player->getSpeed(i));

	for (int i = 0; i < 4; i++)
		setQuaternion(i, player->getQuaternion(i));

	setSpecialAction(player->getSpecialAction());
	if (player->getAnimation()->getAnimId() != 0
		&& player->getAnimation()->getAnimFlags() != 0) {
		getAnimation()->setAnimFlags(player->getAnimation()->getAnimFlags());
		getAnimation()->setAnimId(player->getAnimation()->getAnimId());
	}

	getKeys()->setKeys(player->getKeys()->getKeys());
	getKeys()->setLeftRightKey(player->getKeys()->getLeftRightKey());
	getKeys()->setUpDownKey(player->getKeys()->getUpDownKey());

	onfootSync();
}

void Bot::connect(std::string address, uint16_t port) {
	Lock lock(&_botMutex);

	if (isConnected()) {
		RakBot::app()->log("[ERROR] Подключение к серверу: бот уже подключен к серверу");
		return;
	}

	if (RakBot::app()->getEvents()->onRequestConnect())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	std::string serverPassword = RakBot::app()->getSettings()->getServerPassword();

	if (!serverPassword.empty()) {
		rakClient->SetPassword(serverPassword.c_str());
		RakBot::app()->log("[RAKBOT] Установлен пароль сервера: %s", serverPassword.c_str());
	}

	RakBot::app()->log("[RAKBOT] Подключение к %s:%d...", address.c_str(), port);
	rakClient->Connect(address.c_str(), port, 0, 0, 5);
	setConnectRequested(true);
}

void Bot::disconnect(bool timeOut) {
	Lock lock(&_botMutex);

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	if (!isConnected()) {
		RakBot::app()->log("[RAKBOT] Бот не подключен к серверу");
		return;
	}

	if (timeOut)
		rakClient->Disconnect(0);
	else
		rakClient->Disconnect(500);

	reset(true);

	for (int i = 0; i < MAX_PLAYERS; i++)
		RakBot::app()->deletePlayer(i);

	for (int i = 0; i < MAX_PICKUPS; i++)
		RakBot::app()->deletePickup(i);

	for (int i = 1; i < MAX_VEHICLES; i++)
		RakBot::app()->deleteVehicle(i);

	ZeroMemory(Objects, sizeof(GTAObject) * MAX_OBJECTS);
	ZeroMemory(&checkpoint, sizeof(Checkpoint));
	ZeroMemory(&raceCheckpoint, sizeof(RaceCheckpoint));
	ZeroMemory(&spawnInfo, sizeof(SpawnInfo));
	ZeroMemory(&gtaMenu, sizeof(GTAMenu));

	setConnectRequested(true);
	BotConnectedTimer.setTimer(UINT32_MAX);
	BotSpawnedTimer.setTimer(UINT32_MAX);
	GameInitedTimer.setTimer(UINT32_MAX);

	RakBot::app()->getServer()->reset();
	RakBot::app()->log("[RAKBOT] Бот отключен от сервера");
	RakBot::app()->getEvents()->onDisconnect(DISCONNECT_REASON_SELF);
}

void Bot::sendInput(std::string input) {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Отправка ввода: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Отправка ввода: игра должна быть инициализирована");
		return;
	}

	if (RakBot::app()->getEvents()->onSendInput(input))
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	RakNet::BitStream bsSend;

	if (input[0] == '/') {
		int length = input.length();
		bsSend.Write(length);
		bsSend.Write(input.c_str(), length);
		rakClient->RPC(&RPC_ServerCommand, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	} else {
		uint8_t length = static_cast<uint8_t>(input.length());
		bsSend.Write(length);
		bsSend.Write(input.c_str(), length);
		rakClient->RPC(&RPC_Chat, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	}
}

void Bot::requestClass(int classId) {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Запрос класса: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Запрос класса: игра должна быть инициализирована");
		return;
	}

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakNet::BitStream bsSend;
	bsSend.Write(classId);
	rakClient->RPC(&RPC_RequestClass, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	RakBot::app()->log("[RAKBOT] Запрошен класс с ID %d", classId);
}

void Bot::requestSpawn() {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Запрос спавна: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Запрос спавна: игра должна быть инициализирована");
		return;
	}

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	BitStream bsData;
	rakClient->RPC(&RPC_RequestSpawn, &bsData, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	RakBot::app()->log("[RAKBOT] Запрошен спавн");
}

bool Bot::pickUpPickup(Pickup *pickup, bool checkDist) {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Поднятие пикапа: бот должен быть подключен к серверу");
		return false;
	}

	if (!isSpawned()) {
		RakBot::app()->log("[ERROR] Поднятие пикапа: бот должен быть заспавнен");
		return false;
	}

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	if (getPlayerState() != PLAYER_STATE_ONFOOT)
		return false;

	if (!isSpawned())
		return false;

	if (pickup == nullptr) {
		RakBot::app()->log("[ERROR] Поднятие пикапа: пикап %d неактивен", pickup->getPickupId());
		return false;
	}

	if (distanceTo(pickup) >= 50.0f && checkDist) {
		RakBot::app()->log("[ERROR] Поднятие пикапа: расстояние до пикапа %d (модель: %d) больше 50 метров!", pickup->getPickupId(), pickup->getModel());
		return false;
	}

	teleport(pickup->getPosition(0), pickup->getPosition(1), pickup->getPosition(2));

	RakNet::BitStream bsSend;
	bsSend.Write<int>(pickup->getPickupId());
	rakClient->RPC(&RPC_PickedUpPickup, &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	RakBot::app()->log("[RAKBOT] Поднят пикап с ID %d (модель: %d)", pickup->getPickupId(), pickup->getModel());
	return true;
}

bool Bot::takeCheckpoint() {
	Lock lock(&_botMutex);

	if (!checkpoint.active && !raceCheckpoint.active)
		return false;

	float *position = checkpoint.active ? checkpoint.position : raceCheckpoint.position;

	if (RakBot::app()->getEvents()->onTakeCheckpoint(position[0], position[1], position[2]))
		return false;

	teleport(position[0], position[1], position[2]);

	// RakBot::app()->log("[RAKBOT] Бот телепортирован на чекпоинт");
	return true;
}

void Bot::passengerSync() {
	if (getPlayerState() != PLAYER_STATE_PASSENGER)
		return;

	if (!isSpawned())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakNet::BitStream bsVehicleSync;
	PassengerData passengerSync;
	ZeroMemory(&passengerSync, sizeof(PassengerData));

	passengerSync.byteArmor = getArmour();
	passengerSync.sUpDownKeys = getKeys()->getUpDownKey();
	passengerSync.sLeftRightKeys = getKeys()->getLeftRightKey();
	passengerSync.sKeys = getKeys()->getKeys();
	passengerSync.byteCurrentWeapon = getWeapon();
	passengerSync.byteHealth = getHealth();
	passengerSync.byteSeatID = getVehicleSeat();

	Vehicle *vehicle = getVehicle();
	if (vehicle == nullptr)
		return;

	for (int i = 0; i < 3; i++)
		setPosition(i, vehicle->getPosition(i));

	for (int i = 0; i < 3; i++)
		passengerSync.fPosition[i] = getPosition(i);

	passengerSync.sVehicleID = vehicle->getVehicleId();

	bsVehicleSync.Write((uint8_t)ID_PASSENGER_SYNC);
	bsVehicleSync.Write((PCHAR)&passengerSync, sizeof(PassengerData));
	rakClient->Send(&bsVehicleSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

	aimSync();
}

void Bot::driverSync() {
	if (getPlayerState() != PLAYER_STATE_DRIVER)
		return;

	if (!isSpawned())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakNet::BitStream bsDriverSync;
	IncarData driverSync;
	ZeroMemory(&driverSync, sizeof(IncarData));

	Vehicle *vehicle = getVehicle();
	if (vehicle == nullptr)
		return;

	driverSync.sVehicleId = vehicle->getVehicleId();
	driverSync.byteLandingGearState = vehicle->getGearState();
	driverSync.byteSirenOn = vehicle->isSirenEnabled();
	driverSync.TrailerID_or_ThrustAngle = vehicle->getTrailerId();
	driverSync.fCarHealth = vehicle->getCarHealth();
	driverSync.fTrainSpeed = vehicle->getTrainSpeed();

	for (int i = 0; i < 3; i++)
		driverSync.position[i] = getPosition(i);

	for (int i = 0; i < 3; i++)
		driverSync.vecMoveSpeed[i] = getSpeed(i);

	for (int i = 0; i < 4; i++)
		driverSync.quaternion[i] = getQuaternion(i);

	driverSync.bytePlayerHealth = getHealth();
	driverSync.bytePlayerArmour = getArmour();
	driverSync.weapon = getWeapon();
	driverSync.sKeys = getKeys()->getKeys();
	driverSync.lrAnalog = getKeys()->getLeftRightKey();
	driverSync.udAnalog = getKeys()->getUpDownKey();
	bsDriverSync.Write((uint8_t)ID_VEHICLE_SYNC);
	bsDriverSync.Write((PCHAR)&driverSync, sizeof(IncarData));
	rakClient->Send(&bsDriverSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

	aimSync();
}

void Bot::spectateSync() {
	if (getPlayerState() != PLAYER_STATE_SPECTATE)
		return;

	if (isSpawned())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	RakNet::BitStream bsSpecSync;
	SpectatorData specSync;
	ZeroMemory(&specSync, sizeof(SpectatorData));

	for (int i = 0; i < 3; i++)
		specSync.fPosition[i] = getPosition(i);

	specSync.sKeys = getKeys()->getKeys();
	specSync.sLeftRightKeys = getKeys()->getLeftRightKey();
	specSync.sUpDownKeys = getKeys()->getUpDownKey();
	bsSpecSync.Write((uint8_t)ID_SPECTATOR_SYNC);
	bsSpecSync.Write((PCHAR)&specSync, sizeof(SpectatorData));
	rakClient->Send(&bsSpecSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
}

void Bot::dialogResponse(uint16_t dialogId, uint8_t button, uint16_t item, std::string input) {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Отправка диалога: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Отправка диалога: игра должна быть инициализирована");
		return;
	}

	if (RakBot::app()->getEvents()->onDialogResponse(dialogId, button, item, input))
		return;

	static bool dialogResponseReady = true;
	while (!dialogResponseReady)
		Sleep(10);
	dialogResponseReady = false;

	RakBot::app()->getEvents()->defCallAdd(vars.dialogResponseDelay, false, [this, dialogId, button, item, input](DefCall *) {
		RakClientInterface *rakClient = RakBot::app()->getRakClient();

		uint8_t length = static_cast<uint8_t>(input.length());
		RakNet::BitStream bsSend;
		bsSend.Write(dialogId);
		bsSend.Write(button);
		bsSend.Write(item);
		bsSend.Write(length);
		bsSend.Write(input.c_str(), length);

		rakClient->RPC(&RPC_DialogResponse, const_cast<BitStream *>(&bsSend), HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
		dialogResponseReady = true;
	});
}

void Bot::spawn() {
	static bool spawnReady = true;

	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Спавн бота: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Спавн бота: игра должна быть инициализирована");
		return;
	}

	if (RakBot::app()->getEvents()->onSpawn())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	if (!spawnReady) {
		RakBot::app()->log("[RAKBOT] Спавн бота...");
	}

	spawnReady = false;
	vars.syncAllowed = false;
	reset(false);
	vars.routeIndex = 0;

	RakNet::BitStream bsSendSpawn;
	rakClient->RPC(&RPC_Spawn, &bsSendSpawn, HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	vars.waitForRequestSpawnReply = false;

	RakBot::app()->getEvents()->defCallAdd(vars.spawnDelay, false, [this](DefCall *) {
		setPlayerState(PLAYER_STATE_ONFOOT);
		for (int i = 0; i < 3; i++)
			setPosition(i, spawnInfo.position[i]);
		setHealth(100);
		setQuaternion(0, -cosf((spawnInfo.rotation / 2.f) * M_PI / 180.f));
		setQuaternion(1, 0.f);
		setQuaternion(2, 0.f);
		setQuaternion(3, 1.f * sinf((spawnInfo.rotation / 2.f) * M_PI / 180.f));

		float position[3];
		for (int i = 0; i < 3; i++)
			position[i] = getPosition(i);

		setSpawned(true);
		RakBot::app()->log("[RAKBOT] Бот заспавнен");
		sync();
		vars.syncAllowed = true;
		BotSpawnedTimer.setTimerFromCurrentTime();
		RakBot::app()->getEvents()->onSpawned();
		spawnReady = true;
	});
}

void Bot::kill() {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Убийство бота: бот должен быть подключен к серверу");
		return;
	}

	if (!isSpawned()) {
		RakBot::app()->log("[ERROR] Убийство бота: бот должен быть заспавнен");
		return;
	}

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	setHealth(0);
	sync();

	RakNet::BitStream bsSend;
	bsSend.Write("\xFF\xFF\xFF", 3);
	rakClient->RPC(&RPC_Death, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	spawn();
	RakBot::app()->getEvents()->onDeath();
	RakBot::app()->log("[RAKBOT] Бот убит");
}

void Bot::clickTextdraw(uint16_t textDrawId) {
	Lock lock(&_botMutex);

	if (!isConnected()) {
		RakBot::app()->log("[ERROR] Клик по текстдраву: бот должен быть подключен к серверу");
		return;
	}

	if (!RakBot::app()->getServer()->isGameInited()) {
		RakBot::app()->log("[ERROR] Клик по текстдраву: игра должна быть инициализирована");
		return;
	}

	if (RakBot::app()->getEvents()->onTextDrawClick(textDrawId))
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakNet::BitStream bsSend;
	bsSend.Write<uint16_t>(textDrawId);
	rakClient->RPC(&RPC_ClickTextDraw, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	RakBot::app()->log("[RAKBOT] Отправлен клик по текстдраву с ID %d", textDrawId);
}