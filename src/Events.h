#pragma once
#include "Mutex.h"
class Events : private Mutex {
public:
	Events();
	~Events();

	void reset();

	bool onRunCommand(std::string command);
	bool onPrintLog(std::string text);
	bool onSendInput(std::string input);
	bool onSync();

	bool onGameText(std::string gameText);
	void onSpawned();
	bool onSpawn();
	void onDeath();
	void onSetSpawnPos(float positionX, float positionY, float positionZ);
	bool onSetPosition(float positionX, float positionY, float positionZ);
	bool onChangePosition(float positionX, float positionY, float positionZ);
	bool onSetHealth(uint8_t health);
	bool onServerMessage(std::string message);
	bool onChatMessage(uint16_t playerId, std::string message);
	bool onDialogShow(uint16_t dialogId, uint8_t dialogStyle, std::string dialogTitle, std::string okButtonText, std::string cancelButtonText, std::string dialogText);
	bool onDialogResponse(uint16_t dialogId, uint8_t dialogButton, uint8_t dialogItem, std::string dialogInput);
	void onSetSkin(uint16_t playerid, uint16_t skinId);
	void onApplyAnimation(uint16_t playerId, uint16_t animId);
	void onConnect(uint16_t playerId);
	void onDisconnect(uint8_t reason);

	// MONEY
	void onSetMoney(int money);

	// GAME
	void onGameInited(std::string serverName);

	// VEHICLES
	void onCreateVehicle(Vehicle *vehicle);
	void onDestroyVehicle(Vehicle *vehicle);
	void onSetVehicleParams(Vehicle *vehicle);

	// PUT & EJECT VEHICLE
	bool onPutInVehicle(Vehicle *vehicleId, uint8_t seatId);
	bool onEjectFromVehicle();

	// PLAYERS
	void onPlayerJoin(Player *player);
	void onPlayerQuit(Player *player, uint8_t reason);
	void onPlayerRemoveFromWorld(Player *player);
	void onPlayerDeath(Player *player);
	void onPlayerAddInWorld(Player *player);

	// TEXTDRAWS
	void onTextDrawHide(uint16_t textDrawId);
	void onTextDrawShow(uint16_t textDrawId, float positionX, float positionY, std::string textDrawString);
	void onTextDrawSetString(uint16_t textDrawId, std::string string);
	bool onTextDrawClick(uint16_t textDrawId);

	// SPECTATING
	void onToggleSpectating(bool state);

	// PICKUPS
	void onCreatePickup(Pickup *pickup);
	void onDestroyPickup(Pickup *pickup);
	bool onPickUpPickup(Pickup *pickup);

	// CHECKPOINTS
	void onCreateCheckpoint(Checkpoint *checkpoint);
	void onDestroyCheckpoint(Checkpoint *checkpoint);
	void onCreateRaceCheckpoint(RaceCheckpoint *raceCheckpoint);
	void onDestroyRaceCheckpoint(RaceCheckpoint *raceCheckpoint);

	// OBJECTS
	void onCreateObject(GTAObject *object);
	void onDestroyObject(GTAObject *object);
	void onAttachObjectToPlayer(uint16_t playerId, uint32_t slotId, bool attach);

	// ...
	bool onTakeCheckpoint(float positionX, float positionY, float positionZ);
};