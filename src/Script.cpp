#include "RakBot.h"

#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "RakNet.h"
#include "Settings.h"
#include "Funcs.h"
#include "Server.h"
#include "Pickup.h"
#include "Lock.h"
#include "Vehicle.h"

#include "MiscFuncs.h"

#include "cmds.h"
#include "keycheck.h"
#include "netgame.h"
#include "netrpc.h"
#include "window.h"

#include "Script.h"

std::vector<Script *> scripts;

void LuaPanic(sol::optional<std::string> maybe_msg) {
	RakBot::app()->log("[ERROR] ������ Lua!");
	if (maybe_msg) {
		const std::string& msg = maybe_msg.value();
		std::vector<std::string> lines = Split(msg, '\n');
		for each (std::string line in lines) {
			RakBot::app()->log("[ERROR] %s", line.c_str());
		}
	}
}

Script::Script(std::string scriptName) : _scriptName(scriptName) {
	try {
		std::string scriptPath = std::string(GetRakBotPath("scripts")) + "\\" + _scriptName;

		// LOAD FILE
		std::ifstream scriptFile(scriptPath);
		if (!scriptFile.is_open()) {
			luaLog("[ERROR] ������ �������� ������� \"" + _scriptName + "\": ���� �� ������");
			return;
		}
		scriptFile.close();

		// LOAD LUA LIBRARIES
		_scriptState.open_libraries();
		_scriptState.set_panic(sol::c_call<decltype(&LuaPanic), &LuaPanic>);

		// REGISTER LUA FUNCTIONS
		luaRegisterFunctions();

		// LOAD AND EXECUTE SCRIPT
		sol::load_result script = _scriptState.load_file(scriptPath);
		if (!script.valid())
			throw script.get<sol::error>();

		sol::protected_function_result result = script.get<sol::protected_function>()();
		if (!result.valid())
			throw result.get<sol::error>();
		luaLog("[LUA] ������ \"" + _scriptName + "\" ������� ��������");
	} catch (const char *e) {
		std::string message = "������ �������� ������� \"" + _scriptName + "\": " + e;
		luaError(message);
	} catch (const std::exception &e) {
		std::string message = "������ �������� ������� \"" + _scriptName + "\": " + e.what();
		luaError(message);
	} catch (...) {
		std::string message = "����������� ������ ��� �������� ������� \"" + _scriptName + "\"";
		luaError(message);
	}

	// CALL ON_SCRIPT_START CALBACK
	luaOnScriptStart();
}

Script::~Script() {}

void Script::luaLog(std::string log) {
	/* Lock lock(vars.logMutex);

	if (log.empty())
		return;

	SYSTEMTIME		time;
	char			path[MAX_PATH], name[MAX_PATH];

	GetLocalTime(&time);

	if (LogFile == nullptr) {
		if (RakBot::app()->getSettings()->getAddress()->getIp().empty()
			|| RakBot::app()->getSettings()->getName().empty()
			|| RakBot::app()->getSettings()->getAddress()->getPort() == 0) {
			return;
		}

		GetModuleFileName(NULL, path, sizeof(path));
		strcpy(strrchr(path, '\\') + 1, "logs");
		CreateDirectory(path, NULL);
		snprintf(name, sizeof(name), "%s\\%s;%d",
			path,
			RakBot::app()->getSettings()->getAddress()->getIp().c_str(),
			RakBot::app()->getSettings()->getAddress()->getPort());
		CreateDirectory(name, NULL);
		strcat(name, "\\");
		strcat(name, RakBot::app()->getSettings()->getName().c_str());
		strcat(name, ".log");

		LogFile = fopen(name, "a");
		if (LogFile == NULL) {
			return;
		}
	}

	fprintf(LogFile, "[%02d:%02d:%02d] %s\n", time.wHour, time.wMinute, time.wSecond, log.c_str());
	fflush(LogFile);

	int bufSize = 512;
	char *buf = new char[bufSize + 1];
	_snprintf(buf, bufSize, "%s", log.c_str());

	if (vars.timeStamp) {
		char *bufTemp = new char[bufSize + 20];
		_snprintf(bufTemp, bufSize, "[%02d:%02d:%02d] %s", time.wHour, time.wMinute, time.wSecond, buf);
		delete[] buf;
		buf = bufTemp;
	}

	int lbCount = SendMessage(g_hWndLog, LB_GETCOUNT, 0, 0);
	if (lbCount >= MAX_LOGLINES)
		SendMessage(g_hWndLog, LB_DELETESTRING, 0, 0);

	WPARAM idx = SendMessage(g_hWndLog, LB_ADDSTRING, 0, (LPARAM)buf);
	SendMessage(g_hWndLog, LB_SETTOPINDEX, idx, 0); */
}

Script *Script::load(std::string scriptName) {
	try {
		if (scriptName.empty())
			return nullptr;

		return new Script(scriptName);
	} catch (const char *e) {
		std::string message = "[ERROR] ���������� ��� �������� ������� �������: " + std::string(e);
		RakBot::app()->log(message.c_str());
		return nullptr;
	} catch (const std::exception &e) {
		std::string message = "[ERROR] ���������� ��� �������� ������� �������: " + std::string(e.what());
		RakBot::app()->log(message.c_str());
		return nullptr;
	} catch (...) {
		std::string message = "[ERROR] �������������� ���������� ��� �������� ������� �������";
		RakBot::app()->log(message.c_str());
		return nullptr;
	}
}

void Script::unload(Script *script) {
	try {
		delete script;
	} catch (const char *e) {
		std::string message = "[ERROR] ���������� ��� �������� �������: " + std::string(e);
		RakBot::app()->log(message.c_str());
	} catch (const std::exception &e) {
		std::string message = "[ERROR] ���������� ��� �������� �������: " + std::string(e.what());
		RakBot::app()->log(message.c_str());
	} catch (...) {
		std::string message = "[ERROR] �������������� ���������� ��� �������� �������";
		RakBot::app()->log(message.c_str());
	}
}

// CALLBACKS

void Script::luaOnReset(uint8_t restart, uint32_t reconnectTime) {
	luaCallback("onReset", restart, reconnectTime);
}

bool Script::luaOnRecvPacket(uint8_t packetId, uint8_t *packetData, int packetSize) {
	return luaCallback("onRecvPacket", packetId, reinterpret_cast<int>(packetData), packetSize);
}

bool Script::luaOnSendPacket(uint8_t packetId, uint8_t *packetData, int packetSize) {
	return luaCallback("onSendPacket", packetId, reinterpret_cast<int>(packetData), packetSize);
}

bool Script::luaOnRecvRPC(uint8_t rpcId, uint8_t *packetData, int packetSize) {
	return luaCallback("onRecvRpc", rpcId, reinterpret_cast<int>(packetData), packetSize);
}

bool Script::luaOnSendRPC(uint8_t rpcId, uint8_t *packetData, int packetSize) {
	return luaCallback("onSendRpc", rpcId, reinterpret_cast<int>(packetData), packetSize);
}

void Script::luaOnGameText(std::string gameText) {
	luaCallback("onGameText", gameText);
}

void Script::luaOnSetSpawnPos(float positionX, float positionY, float positionZ) {
	luaCallback("onSetSpawnPos", positionX, positionY, positionZ);
}

bool Script::luaOnServerMessage(std::string message) {
	return luaCallback("onServerMessage", message);
}

bool Script::luaOnChatMessage(uint16_t playerId, std::string message) {
	return luaCallback("onChatMessage", playerId, message);
}

void Script::luaOnSetSkin(uint16_t playerid, uint16_t skinId) {
	luaCallback("onSetSkin", playerid, skinId);
}

void Script::luaOnCreateVehicle(uint16_t vehicleId) {
	luaCallback("onCreateVehicle", vehicleId);
}

void Script::luaOnDestroyVehicle(uint16_t vehicleId) {
	luaCallback("onDestroyVehicle", vehicleId);
}

void Script::luaOnSetVehicleParams(uint16_t vehicleId) {
	luaCallback("onSetVehicleParams", vehicleId);
}

void Script::luaOnPlayerAddInWorld(uint16_t playerId) {
	luaCallback("onPlayerAddInWorld", playerId);
}

void Script::luaOnPlayerRemoveFromWorld(uint16_t playerId) {
	luaCallback("onPlayerRemoveFromWorld", playerId);
}

void Script::luaOnPlayerDeath(uint16_t playerId) {
	luaCallback("onPlayerDeath", playerId);
}

void Script::luaOnTextDrawHide(uint16_t textDrawId) {
	luaCallback("onTextDrawHide", textDrawId);
}

void Script::luaOnTextDrawSetString(uint16_t textDrawId, std::string textDrawString) {
	luaCallback("onTextDrawSetString", textDrawId, textDrawString);
}

void Script::luaOnTextDrawShow(uint16_t textDrawId, float positionX, float positionY, std::string textDrawString) {
	luaCallback("onTextDrawShow", textDrawId, positionX, positionY, textDrawString);
}

void Script::luaOnToggleSpectating(bool state) {
	luaCallback("onToggleSpectating", state);
}

bool Script::luaOnPutInVehicle(uint16_t vehicleId, uint16_t seatId) {
	return luaCallback("onPlayerPutInVehicle", vehicleId, seatId);
}

bool Script::luaOnEjectFromVehicle() {
	return luaCallback("onPlayerEjectFromVehicle");
}

void Script::luaOnCreateCheckpoint() {
	luaCallback("onCreateCheckpoint");
}

void Script::luaOnDestroyCheckpoint() {
	luaCallback("onDestroyCheckpoint");
}

void Script::luaOnCreateRaceCheckpoint() {
	luaCallback("onCreateRaceCheckpoint");
}

void Script::luaOnDestroyRaceCheckpoint() {
	luaCallback("onDestroyRaceCheckpoint");
}

void Script::luaOnCreatePickup(uint16_t pickupId) {
	luaCallback("onCreatePickup", pickupId);
}

void Script::luaOnDestroyPickup(uint16_t pickupId) {
	luaCallback("onDestroyPickup", pickupId);
}

void Script::luaOnCreateObject(uint16_t objectId) {
	luaCallback("onCreateObject", objectId);
}

void Script::luaOnDestroyObject(uint16_t objectId) {
	luaCallback("onDestroyObject", objectId);
}

void Script::luaOnAttachObjectToPlayer(uint16_t playerId, uint32_t slotId, bool attach) {
	luaCallback("onAttachObjectToPlayer", playerId, slotId, attach);
}

bool Script::luaOnSetHealth(uint8_t health) {
	return luaCallback("onSetHealth", health);
}

void Script::luaOnSetMoney(int money) {
	luaCallback("onSetMoney", money);
}

void Script::luaOnPlayerQuit(uint16_t playerId, uint8_t reasonId) {
	luaCallback("onPlayerQuit", playerId, reasonId);
}

void Script::luaOnPlayerJoin(uint16_t playerId, std::string playerName) {
	luaCallback("onPlayerJoin", playerId, playerName);
}

void Script::luaOnGameInited() {
	luaCallback("onGameInited");
}

void Script::luaOnConnect() {
	luaCallback("onConnect");
}

bool Script::luaOnSetPosition(float positionX, float positionY, float positionZ) {
	return luaCallback("onSetPosition", positionX, positionY, positionZ);
}

void Script::luaOnSpawn(float positionX, float positionY, float positionZ) {
	luaCallback("onSpawn", positionX, positionY, positionZ);
}

bool Script::luaOnDialogShow(uint16_t dialogId, uint8_t dialogStyle, std::string dialogTitle, std::string okButtonText, std::string cancelButtonText, std::string dialogText) {
	return luaCallback("onDialogShow", dialogId, dialogStyle, dialogTitle, okButtonText, cancelButtonText, dialogText);
}

bool Script::luaOnPrintLog(std::string msg) {
	return luaCallback("onPrintLog", msg);
}

bool Script::luaOnRunCommand(std::string cmd) {
	return luaCallback("onRunCommand", cmd);
}

void Script::luaOnScriptStart() {
	luaCallback("onScriptStart");
}

void Script::luaOnScriptExit() {
	luaCallback("onScriptExit");
}

void Script::luaOnScriptUpdate() {
	luaCallback("onScriptUpdate");
}

void Script::luaRegisterFunctions() {
	_scriptState.set_function("connect", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->connect(
			RakBot::app()->getSettings()->getAddress()->getIp(),
			RakBot::app()->getSettings()->getAddress()->getPort()
		);
	});
	_scriptState.set_function("disconnect", [this](bool timeOut) {
		Bot *bot = RakBot::app()->getBot();
		return bot->disconnect(timeOut);
	});
	_scriptState.set_function("reset", [this](bool reconnect) {
		Bot *bot = RakBot::app()->getBot();
		return bot->reset(reconnect);
	});
	_scriptState.set_function("reconnect", [this](int reconnectDelay) {
		Bot *bot = RakBot::app()->getBot();
		return bot->reset(reconnectDelay);
	});
	_scriptState.set_function("getMoney", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getMoney();
	});
	_scriptState.set_function("enterVehicle", [this](int vehicleId, int seatId) {
		Bot *bot = RakBot::app()->getBot();
		return bot->enterVehicle(vehicleId, seatId);
	});
	_scriptState.set_function("exitVehicle", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->exitVehicle();
	});
	_scriptState.set_function("sendSync", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->sync();
	});
	_scriptState.set_function("sendInput", [this](std::string input) {
		Bot *bot = RakBot::app()->getBot();
		return bot->sendInput(input);
	});
	_scriptState.set_function("requestClass", [this](int classId) {
		Bot *bot = RakBot::app()->getBot();
		return bot->requestClass(classId);
	});
	_scriptState.set_function("requestSpawn", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->requestSpawn();
	});
	_scriptState.set_function("sendPickup", [this](int pickupId, bool checkDist) {
		Bot *bot = RakBot::app()->getBot();
		return bot->pickUpPickup(pickupId, checkDist);
	});
	_scriptState.set_function("sendDialog", [this](int dialogId, int button, int item, std::string input) {
		Bot *bot = RakBot::app()->getBot();
		return bot->dialogResponse(dialogId, button, item, input);
	});
	_scriptState.set_function("sendSpawn", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->spawn();
	});
	_scriptState.set_function("getHealth", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getHealth();
	});
	_scriptState.set_function("setHealth", [this](int health) {
		Bot *bot = RakBot::app()->getBot();
		return bot->setHealth(health);
	});
	_scriptState.set_function("getArmour", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getArmour();
	});
	_scriptState.set_function("setArmour", [this](int armour) {
		Bot *bot = RakBot::app()->getBot();
		return bot->setArmour(armour);
	});
	_scriptState.set_function("getWeapon", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getWeapon();
	});
	_scriptState.set_function("setWeapon", [this](int weapon) {
		Bot *bot = RakBot::app()->getBot();
		return bot->setWeapon(weapon);
	});
	_scriptState.set_function("getBotId", [this]() {
		Bot *bot = RakBot::app()->getBot();
		uint16_t playerId = bot->getPlayerId();
		if (playerId == PLAYER_ID_NONE)
			return sol::make_object(_scriptState, false);
		return sol::make_object(_scriptState, playerId);
	});
	_scriptState.set_function("getSkin", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getSkin();
	});
	_scriptState.set_function("getBotState", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getPlayerState();
	});
	_scriptState.set_function("getPosition", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return std::make_tuple(bot->getPosition(0), bot->getPosition(1), bot->getPosition(2));
	});
	_scriptState.set_function("setPosition", [this](float x, float y, float z) {
		Bot *bot = RakBot::app()->getBot();
		bot->setPosition(0, x);
		bot->setPosition(1, y);
		bot->setPosition(2, z);
	});
	_scriptState.set_function("getSpeed", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return std::make_tuple(bot->getSpeed(0), bot->getSpeed(1), bot->getSpeed(2));
	});
	_scriptState.set_function("setSpeed", [this](float x, float y, float z) {
		Bot *bot = RakBot::app()->getBot();
		bot->setSpeed(0, x);
		bot->setSpeed(1, y);
		bot->setSpeed(2, z);
	});
	_scriptState.set_function("getQuaternion", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return std::make_tuple(bot->getQuaternion(0), bot->getQuaternion(1), bot->getQuaternion(2), bot->getQuaternion(3));
	});
	_scriptState.set_function("setQuaternion", [this](float w, float x, float y, float z) {
		Bot *bot = RakBot::app()->getBot();
		bot->setQuaternion(0, w);
		bot->setQuaternion(1, x);
		bot->setQuaternion(2, y);
		bot->setQuaternion(3, z);
	});
	_scriptState.set_function("getBotVehicle", [this]() {
		Bot *bot = RakBot::app()->getBot();
		Vehicle *vehicle = bot->getVehicle();
		if (vehicle == nullptr)
			return sol::make_object(_scriptState, false);
		return sol::make_object(_scriptState, vehicle->getVehicleId());
	});
	_scriptState.set_function("getAnimation", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return std::make_tuple(bot->getAnimation()->getAnimId(), bot->getAnimation()->getAnimFlags());
	});
	_scriptState.set_function("setAnimation", [this](int animId, int animFlags) {
		Bot *bot = RakBot::app()->getBot();
		bot->getAnimation()->setAnimId(animId);
		bot->getAnimation()->setAnimFlags(animFlags);
	});
	_scriptState.set_function("getKeys", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return std::make_tuple(bot->getKeys()->getKeys(), bot->getKeys()->getLeftRightKey(), bot->getKeys()->getUpDownKey());
	});
	_scriptState.set_function("setKeys", [this](int keys, int lrAnalog, int udAnalog) {
		Bot *bot = RakBot::app()->getBot();
		bot->getKeys()->setKeys(keys);
		bot->getKeys()->setLeftRightKey(lrAnalog);
		bot->getKeys()->setUpDownKey(udAnalog);
	});
	_scriptState.set_function("getScore", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getInfo()->getScore();
	});
	_scriptState.set_function("getPing", [this]() {
		Bot *bot = RakBot::app()->getBot();
		return bot->getInfo()->getPing();
	});
	_scriptState.set_function("clickTextdraw", [this](int textdrawId) {
		Bot *bot = RakBot::app()->getBot();
		bot->clickTextdraw(textdrawId);
	});

	// RAKNET
	_scriptState.set_function("sendPacket", [this](int bsPtr) {
		BitStream *bsData = reinterpret_cast<BitStream *>(bsPtr);
		if (bsData == nullptr)
			return;

		RakBot::app()->getRakClient()->Send(bsData, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
	});
	_scriptState.set_function("sendRpc", [this](int rpcId, int bsPtr) {
		if (rpcId < 0 || rpcId > 255)
			return;

		BitStream *bsData = reinterpret_cast<BitStream *>(bsPtr);
		if (bsData == nullptr)
			return;

		RakBot::app()->getRakClient()->RPC(&rpcId, bsData, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	});
	_scriptState.set_function("bitStreamNew", [this]() {
		BitStream *bitStream = new BitStream();
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		return sol::make_object(_scriptState, reinterpret_cast<int>(bitStream));
	});
	_scriptState.set_function("bitStreamInit", [this](int dataPtr, int dataSize) {
		BitStream *bitStream = new BitStream(reinterpret_cast<uint8_t *>(dataPtr), dataSize, false);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		return sol::make_object(_scriptState, reinterpret_cast<int>(bitStream));
	});
	_scriptState.set_function("bitStreamDelete", [this](int bsPtr) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		delete bitStream;
		return true;
	});
	_scriptState.set_function("bitStreamData", [this](int bsPtr) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		return sol::make_object(_scriptState, reinterpret_cast<int>(bitStream->GetData()));
	});
	_scriptState.set_function("bitStreamSize", [this](int bsPtr) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		return sol::make_object(_scriptState, bitStream->GetNumberOfBytesUsed());
	});
	_scriptState.set_function("bitStreamIgnore", [this](int bsPtr, int amount) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		bitStream->IgnoreBits(BYTES_TO_BITS(amount));
		return true;
	});
	_scriptState.set_function("bitStreamReset", [this](int bsPtr) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		bitStream->Reset();
		return true;
	});
	_scriptState.set_function("bitStreamSetWriteOffset", [this](int bsPtr, int offset) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		bitStream->SetWriteOffset(BYTES_TO_BITS(offset));
		return true;
	});
	_scriptState.set_function("bitStreamSetReadOffset", [this](int bsPtr, int offset) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		bitStream->SetReadOffset(BYTES_TO_BITS(offset));
		return true;
	});
	_scriptState.set_function("bitStreamReadByte", [this](int bsPtr, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		uint8_t value;
		if (compressed)
			bitStream->ReadCompressed(value);
		else
			bitStream->Read(value);
		return sol::make_object(_scriptState, static_cast<int>(value));
	});
	_scriptState.set_function("bitStreamReadWord", [this](int bsPtr, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		uint16_t value;
		if (compressed)
			bitStream->ReadCompressed(value);
		else
			bitStream->Read(value);
		return sol::make_object(_scriptState, static_cast<int>(value));
	});
	_scriptState.set_function("bitStreamReadDWord", [this](int bsPtr, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		uint32_t value;
		if (compressed)
			bitStream->ReadCompressed(value);
		else
			bitStream->Read(value);
		return sol::make_object(_scriptState, static_cast<int>(value));
	});
	_scriptState.set_function("bitStreamReadFloat", [this](int bsPtr, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return sol::make_object(_scriptState, false);
		float value;
		if (compressed)
			bitStream->ReadCompressed(value);
		else
			bitStream->Read(value);
		return sol::make_object(_scriptState, value);
	});
	_scriptState.set_function("bitStreamReadString", [this](int bsPtr, int strSize, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr || strSize < 1)
			return sol::make_object(_scriptState, false);
		char *strBuf = new char[strSize + 1];
		if (compressed)
			stringCompressor->DecodeString(strBuf, strSize, bitStream);
		else
			bitStream->Read(strBuf, strSize);
		strBuf[strSize] = 0;
		return sol::make_object(_scriptState, std::string(strBuf));
	});
	_scriptState.set_function("bitStreamWriteByte", [this](int bsPtr, int val, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		uint8_t value = static_cast<uint8_t>(val);
		if (compressed)
			bitStream->WriteCompressed(value);
		else
			bitStream->Write(value);
		return true;
	});
	_scriptState.set_function("bitStreamWriteWord", [this](int bsPtr, int val, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		uint16_t value = static_cast<uint16_t>(val);
		if (compressed)
			bitStream->WriteCompressed(value);
		else
			bitStream->Write(value);
		return true;
	});
	_scriptState.set_function("bitStreamWriteDWord", [this](int bsPtr, int val, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		uint32_t value = static_cast<uint32_t>(val);
		if (compressed)
			bitStream->WriteCompressed(value);
		else
			bitStream->Write(value);
		return true;
	});
	_scriptState.set_function("bitStreamWriteFloat", [this](int bsPtr, float val, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		float value = val;
		if (compressed)
			bitStream->WriteCompressed(value);
		else
			bitStream->Write(value);
		return true;
	});
	_scriptState.set_function("bitStreamWriteString", [this](int bsPtr, std::string str, int strSize, sol::optional<bool> compressed) {
		BitStream *bitStream = reinterpret_cast<BitStream *>(bsPtr);
		if (bitStream == nullptr)
			return false;
		if (compressed)
			stringCompressor->EncodeString(str.c_str(), strSize, bitStream);
		else
			bitStream->Write(str.c_str(), strSize);
		return true;
	});

	// MISC
	_scriptState.set_function("printLog", [this](std::string log) {
		std::vector<std::string> lines = Split(log, '\n');
		for each (std::string line in lines) {
			luaLog("[LUA] " + line);
		}
	});
	_scriptState.set_function("runCommand", [this](std::string cmd) {
		std::vector<std::string> lines = Split(cmd, '|');
		for each (std::string line in lines) {
			RunCommand(line.c_str(), true);
		}
	});
	_scriptState.set_function("sleep", [this](int ms) {
		Sleep(ms);
	});
	_scriptState.set_function("exit", [this]() {
		RakBot::app()->exit();
	});
	_scriptState.set_function("messageBox", [this](std::string message) {
		std::thread msgBoxThread([message] {
			MessageBox(NULL, message.c_str(), "��������� Lua!", MB_ICONASTERISK);
		});
		msgBoxThread.detach();
	});
	_scriptState.set_function("dumpMem", [this](int address, int size) {
		return std::string(DumpMem(reinterpret_cast<uint8_t *>(address), size));
	});
	_scriptState.set_function("openUrl", [this](std::string url) {
		char safeUrlBuf[1024];
		DWORD safeUrlLength = sizeof(safeUrlBuf);
		InternetCanonicalizeUrl(url.c_str(), safeUrlBuf, &safeUrlLength, NULL);
		std::string safeUrl = safeUrlBuf;
		CURLcode code = OpenURL(safeUrl);
		return std::make_tuple(CurlBuffer, static_cast<int>(code));
	});

	// CHECKPOINT
	_scriptState.set_function("getCheckpoint", [this]() {
		sol::table cpInfo = _scriptState.create_table();
		Bot *bot = RakBot::app()->getBot();
		if (!checkpoint.active)
			return sol::make_object(_scriptState, false);
		cpInfo["size"] = checkpoint.size;
		cpInfo["distance"] = bot->distanceTo(checkpoint.position);
		cpInfo["position"] = _scriptState.create_table();
		cpInfo["position"]["x"] = checkpoint.position[0];
		cpInfo["position"]["y"] = checkpoint.position[1];
		cpInfo["position"]["z"] = checkpoint.position[2];
		return sol::make_object(_scriptState, cpInfo);
	});

	// CHECKPOINT
	_scriptState.set_function("getRaceCheckpoint", [this]() {
		sol::table cpInfo = _scriptState.create_table();
		Bot *bot = RakBot::app()->getBot();
		if (!raceCheckpoint.active)
			return sol::make_object(_scriptState, false);
		cpInfo["size"] = raceCheckpoint.size;
		cpInfo["type"] = raceCheckpoint.type;
		cpInfo["distance"] = bot->distanceTo(raceCheckpoint.position);
		cpInfo["position"] = _scriptState.create_table();
		cpInfo["position"]["x"] = raceCheckpoint.position[0];
		cpInfo["position"]["y"] = raceCheckpoint.position[1];
		cpInfo["position"]["z"] = raceCheckpoint.position[2];
		cpInfo["nextPosition"] = _scriptState.create_table();
		cpInfo["nextPosition"]["x"] = raceCheckpoint.fNextPos[0];
		cpInfo["nextPosition"]["y"] = raceCheckpoint.fNextPos[1];
		cpInfo["nextPosition"]["z"] = raceCheckpoint.fNextPos[2];
		return sol::make_object(_scriptState, cpInfo);
	});

	// PICKUPS
	_scriptState.set_function("getPickup", [this](int pickupId) {
		sol::table pickupInfo = _scriptState.create_table();
		Bot *bot = RakBot::app()->getBot();
		Pickup *pickup = RakBot::app()->getPickup(pickupId);
		if (pickup == nullptr)
			return sol::make_object(_scriptState, false);
		pickupInfo["id"] = pickup->getPickupId();
		pickupInfo["model"] = pickup->getModel();
		pickupInfo["type"] = pickup->getType();
		pickupInfo["distance"] = bot->distanceTo(pickup);
		pickupInfo["position"] = _scriptState.create_table();
		pickupInfo["position"]["x"] = pickup->getPosition(0);
		pickupInfo["position"]["y"] = pickup->getPosition(1);
		pickupInfo["position"]["z"] = pickup->getPosition(2);
		return sol::make_object(_scriptState, pickupInfo);
	});

	// VEHICLES
	/* _scriptState.set_function("getVehicle", [this](int vehicleId) {
		sol::table vehicleInfo = _scriptState.create_table();
		Bot *bot = RakBot::app()->getBot();
		stVehicle vehicle = Vehicles[vehicleId];
		if (!vehicle.isExists)
			return sol::make_object(_scriptState, false);
		vehicleInfo["id"] = vehicleId;
		vehicleInfo["model"] = vehicle.model;
		vehicleInfo["firstColor"] = vehicle.firstColor;
		vehicleInfo["secondColor"] = vehicle.secondColor;
		vehicleInfo["lights"] = vehicle.lights;
		vehicleInfo["engine"] = vehicle.engine;
		vehicleInfo["distance"] = bot->distanceTo(vehicle.position);
		vehicleInfo["position"] = _scriptState.create_table();
		vehicleInfo["position"]["x"] = vehicle.position[0];
		vehicleInfo["position"]["y"] = vehicle.position[1];
		vehicleInfo["position"]["z"] = vehicle.position[2];
		return sol::make_object(_scriptState, vehicleInfo);
	}); */

	// OBJECTS
	_scriptState.set_function("getObject", [this](int objectId) {
		sol::table objectInfo = _scriptState.create_table();
		Bot *bot = RakBot::app()->getBot();
		GTAObject object = Objects[objectId];
		if (!object.active)
			return sol::make_object(_scriptState, false);
		objectInfo["id"] = objectId;
		objectInfo["model"] = object.ulModelId;
		objectInfo["objectId"] = object.usObjectId;
		objectInfo["drawDistance"] = object.fDrawDistance;
		objectInfo["distance"] = bot->distanceTo(object.position);
		objectInfo["rotation"] = _scriptState.create_table();
		objectInfo["rotation"]["x"] = object.fRotation[0];
		objectInfo["rotation"]["y"] = object.fRotation[1];
		objectInfo["rotation"]["z"] = object.fRotation[2];
		objectInfo["position"] = _scriptState.create_table();
		objectInfo["position"]["x"] = object.position[0];
		objectInfo["position"]["y"] = object.position[1];
		objectInfo["position"]["z"] = object.position[2];
		return sol::make_object(_scriptState, objectInfo);
	});

	// PLAYERS
	_scriptState.set_function("getPlayer", [this](int playerId) {
		sol::table playerInfo = _scriptState.create_table();
		Bot *bot = RakBot::app()->getBot();
		Player *player = RakBot::app()->getPlayer(playerId);
		if (player == nullptr)
			return sol::make_object(_scriptState, false);
		playerInfo["id"] = player->getPlayerId();
		playerInfo["state"] = player->getPlayerState();
		playerInfo["health"] = player->getHealth();
		playerInfo["isAdmin"] = player->isAdmin();
		playerInfo["inStream"] = player->isInStream();
		playerInfo["inCar"] = ((player->getPlayerState() == PLAYER_STATE_DRIVER) || (player->getPlayerState() == PLAYER_STATE_PASSENGER));
		playerInfo["armour"] = player->getArmour();
		playerInfo["weapon"] = player->getWeapon();
		playerInfo["name"] = player->getName();
		playerInfo["distance"] = bot->distanceTo(player);
		playerInfo["score"] = player->getInfo()->getScore();
		playerInfo["ping"] = player->getInfo()->getPing();
		playerInfo["keys"] = _scriptState.create_table();
		playerInfo["keys"]["keys"] = player->getKeys()->getKeys();
		playerInfo["keys"]["lrAnalog"] = player->getKeys()->getLeftRightKey();
		playerInfo["keys"]["udAnalog"] = player->getKeys()->getUpDownKey();
		playerInfo["anim"] = _scriptState.create_table();
		playerInfo["anim"]["animId"] = player->getAnimation()->getAnimId();
		playerInfo["anim"]["animFlags"] = player->getAnimation()->getAnimFlags();
		playerInfo["position"] = _scriptState.create_table();
		playerInfo["position"]["x"] = player->getPosition(0);
		playerInfo["position"]["y"] = player->getPosition(1);
		playerInfo["position"]["z"] = player->getPosition(2);
		playerInfo["speed"] = _scriptState.create_table();
		playerInfo["speed"]["x"] = player->getSpeed(0);
		playerInfo["speed"]["y"] = player->getSpeed(1);
		playerInfo["speed"]["z"] = player->getSpeed(2);
		playerInfo["quaternion"] = _scriptState.create_table();
		playerInfo["quaternion"]["w"] = player->getQuaternion(0);
		playerInfo["quaternion"]["x"] = player->getQuaternion(1);
		playerInfo["quaternion"]["y"] = player->getQuaternion(2);
		playerInfo["quaternion"]["z"] = player->getQuaternion(3);
		Vehicle *vehicle = player->getVehicle();
		if (vehicle != nullptr) {
			playerInfo["vehicleId"] = vehicle->getVehicleId();
			playerInfo["vehicleSeat"] = player->getVehicleSeat();
		}
		return sol::make_object(_scriptState, playerInfo);
	});
	_scriptState.set_function("getPlayersCount", [this]() {
		return RakBot::app()->getPlayersCount();
	});

	// SERVER
	_scriptState.set_function("getServerName", [this]() {
		Server *server = RakBot::app()->getServer();
		return server->getServerName();
	});

	// ADMINS
	_scriptState.set_function("add", [this](std::string admin) {
		vars.admins.push_back(admin);
	});
	_scriptState.set_function("clear", [this]() {
		vars.admins.clear();
	});

	// AUTO REG
	_scriptState.set_function("setOwnAutoReg", [this](bool state) {
		vars.autoRegEnabled = (state == false);
	});
	_scriptState.set_function("setMail", [this](std::string mail) {
		vars.autoRegMail = mail;
	});
	_scriptState.set_function("getMail", [this]() {
		return vars.autoRegMail;
	});
	_scriptState.set_function("setReferer", [this](std::string referer) {
		vars.autoRegReferer = referer;
	});
	_scriptState.set_function("getReferer", [this]() {
		return vars.autoRegReferer;
	});
	_scriptState.set_function("setSex", [this](int sex) {
		vars.autoRegSex = sex;
	});
	_scriptState.set_function("getSex", [this]() {
		return vars.autoRegSex;
	});

	// SETTINGS
	_scriptState.set_function("getNickName", [this]() {
		Settings *settings = RakBot::app()->getSettings();
		return settings->getName();
	});
	_scriptState.set_function("setNickName", [this](std::string name) {
		Settings *settings = RakBot::app()->getSettings();
		settings->setName(name);
	});
	_scriptState.set_function("getPassword", [this]() {
		Settings *settings = RakBot::app()->getSettings();
		return settings->getLoginPassword();
	});
	_scriptState.set_function("setPassword", [this](std::string password) {
		Settings *settings = RakBot::app()->getSettings();
		settings->setLoginPassword(password);
	});
	_scriptState.set_function("getServerPassword", [this]() {
		Settings *settings = RakBot::app()->getSettings();
		return settings->getServerPassword();
	});
	_scriptState.set_function("setServerPassword", [this](std::string password) {
		Settings *settings = RakBot::app()->getSettings();
		settings->setServerPassword(password);
	});
	_scriptState.set_function("getServerAddress", [this]() {
		std::stringstream ss;
		ss << RakBot::app()->getSettings()->getAddress()->getIp() << ":" << RakBot::app()->getSettings()->getAddress()->getPort();
		return ss.str();
	});
	_scriptState.set_function("setServerAddress", [this](std::string address) {
		std::vector<std::string> parts = Split(address, ':');
		if (parts.size() != 2)
			return;
		std::string ip = parts[0];
		uint16_t port = static_cast<uint16_t>(std::stoi(parts[1]));
		if (port < 1 || port > 65535)
			return;
		RakBot::app()->getSettings()->getAddress()->setIp(ip);
		RakBot::app()->getSettings()->getAddress()->setPort(port);
	});
}

void Script::luaError(std::string error) {
	std::vector<std::string> lines = Split(error, '\n');

	for each (std::string line in lines) {
		luaLog("[ERROR] " + line);
	}
}

void LoadScripts() {
	CreateDirectory(GetRakBotPath("scripts"), NULL);
	CreateDirectory(GetRakBotPath("scripts\\libs"), NULL);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(GetRakBotPath("scripts\\*.lua"), &wfd);
	setlocale(LC_ALL, "");

	bool scriptsExists = false;
	if (INVALID_HANDLE_VALUE != hFind) {
		do {
			scripts.push_back(Script::load(wfd.cFileName));
			scriptsExists = true;
		} while (NULL != FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
}

void UnloadScripts() {
	while (scripts.size() > 0) {
		Script *script = scripts.back();
		script->luaOnScriptExit();
		Script::unload(script);
		scripts.pop_back();
	}
}

template<typename ...Args>
bool Script::luaCallback(std::string funcName, Args && ...args) {
	try {
		Lock lock(*this);

		sol::protected_function func = _scriptState[funcName];
		if (!func.valid())
			return false;

		sol::protected_function_result result = func(std::forward<Args>(args)...);
		if (!result.valid())
			throw result.get<sol::error>();

		if (result.get_type() != sol::type::boolean)
			return false;

		return result.get<bool>();
	} catch (const char *e) {
		std::string message = "������ � ������� \"" + funcName + "\" ������� \"" + _scriptName + "\": " + e;
		luaError(message);
		return false;
	} catch (const std::exception &e) {
		std::string message = "������ � ������� \"" + funcName + "\" ������� \"" + _scriptName + "\": " + e.what();
		luaError(message);
		return false;
	} catch (...) {
		std::string message = "����������� ������ � ������� \"" + funcName + "\" ������� \"" + _scriptName + "\"";
		luaError(message);
		return false;
	}
}