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
#include "Lock.h"

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

void Bot::reset(bool reconnect) {
	uint16_t playerId;
	bool connected;

	if (!reconnect) {
		playerId = getPlayerId();
		connected = isConnected();
	}

	lock();
	_connected = false;
	_spawned = false;
	_money = 0;
	unlock();

	PlayerBase::reset();

	if (!reconnect) {
		setPlayerId(playerId);
		setConnected(connected);
	}

	FuncsOff();
}

void Bot::reconnect(int reconnectDelay) {
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

	GameInited = false;
	ConnectRequested = false;
	BotConnectedTimer.setTimer(UINT32_MAX);
	BotSpawnedTimer.setTimer(UINT32_MAX);
	GameInitedTimer.setTimer(UINT32_MAX);

	RakBot::app()->getServer()->reset();
	ReconnectTimer.setTimer(GetTickCount() + reconnectDelay);
}

// Connected
void Bot::setConnected(bool connected) {
	lock();
	_connected = connected;
	unlock();
}

bool Bot::isConnected() {
	return _connected;
}

// Spawned
void Bot::setSpawned(bool spawned) {
	lock();
	_spawned = spawned;
	unlock();
}

bool Bot::isSpawned() {
	return _spawned;
}

// Money
void Bot::setMoney(int money) {
	lock();
	_money = money;
	unlock();
}

int Bot::getMoney() {
	return _money;
}

// FUNCS
void Bot::enterVehicle(Vehicle *vehicle, uint8_t seatId) {
	static bool enterVehicleReady = true;

	if (getPlayerState() != PLAYER_STATE_ONFOOT) {
		RakBot::app()->log("[ERROR] ������� � ���������: ��� ������ ���� ���������!");
		return;
	}

	if (vehicle == nullptr) {
		RakBot::app()->log("[ERROR] ������� � ���������: ��������� ����������!");
		return;
	}

	if (seatId < 0 || seatId >= vehicle->getSeatAmount()) {
		RakBot::app()->log("[ERROR] ������� � ���������: �������� ���������� �����!");
		return;
	}

	while (!enterVehicleReady)
		return;
	enterVehicleReady = false;

	for (int i = 0; i < 3; i++)
		setPosition(i, vehicle->getPosition(i));
	sync();

	std::thread enterVehicleThread([this, vehicle, seatId]() {
		Sleep(vars.enterVehicleDelay);

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
		RakBot::app()->log("[RAKBOT] ��� ������� � ��������� � ID %d �� ����� %d!", vehicle->getVehicleId(), seatId);
		enterVehicleReady = true;
	});
	enterVehicleThread.detach();
}

void Bot::exitVehicle() {
	if (getPlayerState() != PLAYER_STATE_PASSENGER && getPlayerState() != PLAYER_STATE_DRIVER) {
		RakBot::app()->log("[ERROR] ����� �� ����������: ��� ������ ���� ��������� ��� ����������!");
		return;
	}

	Vehicle *vehicle = getVehicle();
	if (vehicle == nullptr) {
		RakBot::app()->log("[ERROR] ����� �� ����������: ��������� ����������!");
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
	RakBot::app()->log("[RAKBOT] ��� ����� �� ����������!");
}

void Bot::sync() {
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
	if (RakBot::app()->getEvents()->onTeleport(positionX, positionY, positionZ))
		return;

	setPosition(0, positionX);
	setPosition(1, positionY);
	setPosition(2, positionZ);
	sync();
}

void Bot::follow(uint16_t playerId) {
	if (getPlayerState() != PLAYER_STATE_ONFOOT)
		return;

	if (!isSpawned())
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
	if (isConnected())
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	std::string serverPassword = RakBot::app()->getSettings()->getServerPassword();

	if (!serverPassword.empty()) {
		rakClient->SetPassword(serverPassword.c_str());
		RakBot::app()->log("[RAKBOT] ���������� ������ �������: %s", serverPassword.c_str());
	}

	RakBot::app()->log("[RAKBOT] ����������� � %s:%d...", address.c_str(), port);
	rakClient->Connect(address.c_str(), port, 0, 0, 5);
}

void Bot::disconnect(bool timeOut) {
	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	if (!isConnected()) {
		RakBot::app()->log("[RAKBOT] ��� �������� �� �������");
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

	GameInited = false;
	ConnectRequested = true;
	BotConnectedTimer = UINT32_MAX;
	BotSpawnedTimer = UINT32_MAX;
	GameInitedTimer = UINT32_MAX;

	RakBot::app()->getServer()->reset();
	RakBot::app()->log("[RAKBOT] ��� �������� �� �������");
	RakBot::app()->getEvents()->onDisconnect(DISCONNECT_REASON_SELF);
}

void Bot::sendInput(std::string input) {
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
	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakNet::BitStream bsSend;
	bsSend.Write(classId);
	rakClient->RPC(&RPC_RequestClass, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	RakBot::app()->log("[RAKBOT] �������� ����� � ID %d", classId);
}

void Bot::requestSpawn() {
	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	BitStream bsData;
	rakClient->RPC(&RPC_RequestSpawn, &bsData, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	RakBot::app()->log("[RAKBOT] �������� �����");
}

bool Bot::pickUpPickup(Pickup *pickup, bool checkDist) {
	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	if (getPlayerState() != PLAYER_STATE_ONFOOT)
		return false;

	if (!isSpawned())
		return false;

	if (pickup == nullptr) {
		RakBot::app()->log("[ERROR] �������� ������: ����� %d ���������", pickup->getPickupId());
		return false;
	}

	if (distanceTo(pickup) >= 50.0f && checkDist) {
		RakBot::app()->log("[ERROR] �������� ������: ���������� �� ������ %d (������: %d) ������ 50 ������!", pickup->getPickupId(), pickup->getModel());
		return false;
	}

	for (int i = 0; i < 3; i++)
		setPosition(i, pickup->getPosition(i));

	sync();

	RakNet::BitStream bsSend;
	bsSend.Write<int>(pickup->getPickupId());
	rakClient->RPC(&RPC_PickedUpPickup, &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	RakBot::app()->log("[RAKBOT] ������ ����� � ID %d (������: %d)", pickup->getPickupId(), pickup->getModel());
	return true;
}

bool Bot::takeCheckpoint() {
	if (!checkpoint.active && !raceCheckpoint.active)
		return false;

	float *position = checkpoint.active ? checkpoint.position : raceCheckpoint.position;

	if (RakBot::app()->getEvents()->onTakeCheckpoint(position[0], position[1], position[2]))
		return false;

	for (int i = 0; i < 3; i++)
		setPosition(i, position[i]);
	sync();

	// RakBot::app()->log("[RAKBOT] ��� �������������� �� ��������");
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
	if (RakBot::app()->getEvents()->onDialogResponse(dialogId, button, item, input))
		return;

	static bool dialogResponseReady = true;
	while (!dialogResponseReady)
		Sleep(10);
	dialogResponseReady = false;

	std::thread sendThread([this, dialogId, button, item, input]() {
		RakClientInterface *rakClient = RakBot::app()->getRakClient();

		uint8_t length = static_cast<uint8_t>(input.length());
		RakNet::BitStream bsSend;
		bsSend.Write(dialogId);
		bsSend.Write(button);
		bsSend.Write(item);
		bsSend.Write(length);
		bsSend.Write(input.c_str(), length);

		Sleep(vars.dialogResponseDelay);
		rakClient->RPC(&RPC_DialogResponse, const_cast<BitStream *>(&bsSend), HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
		dialogResponseReady = true;
	});
	sendThread.detach();
}

void Bot::spawn() {
	if (RakBot::app()->getEvents()->onSpawn())
		return;

	static bool spawnReady = true;
	while (!spawnReady)
		Sleep(10);
	spawnReady = false;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakBot::app()->log("[RAKBOT] ����� ����...");

	reset(false);
	vars.routeIndex = 0;

	RakNet::BitStream bsSendSpawn;
	rakClient->RPC(&RPC_Spawn, &bsSendSpawn, HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	vars.waitForRequestSpawnReply = false;

	RakBot::app()->log("[RAKBOT] ��� ���������");

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

	std::thread spawnSyncThread([this]() {
		Sleep(vars.spawnDelay);
		setSpawned(true);
		sync();
		vars.syncAllowed = true;
		BotSpawnedTimer = GetTickCount();
		RakBot::app()->getEvents()->onSpawned();
		spawnReady = true;
	});
	spawnSyncThread.detach();
}

void Bot::kill() {
	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	setHealth(0);
	sync();

	RakNet::BitStream bsSend;
	bsSend.Write("\xFF\xFF\xFF", 3);
	rakClient->RPC(&RPC_Death, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	spawn();
	RakBot::app()->getEvents()->onDeath();
	RakBot::app()->log("[RAKBOT] ��� ����");
}

void Bot::clickTextdraw(uint16_t textDrawId) {
	if (RakBot::app()->getEvents()->onTextDrawClick(textDrawId))
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();

	RakNet::BitStream bsSend;
	bsSend.Write<uint16_t>(textDrawId);
	rakClient->RPC(&RPC_ClickTextDraw, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	RakBot::app()->log("[RAKBOT] ��������� ���� �� ���������� � ID %d", textDrawId);
}