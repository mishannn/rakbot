#include "StdAfx.h"

#include "RakBot.h"
#include "RakNet.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "Settings.h"
#include "Pickup.h"
#include "Script.h"
#include "Server.h"
#include "SampRpFuncs.h"
#include "Funcs.h"
#include "MathStuff.h"
#include "cmds.h"
#include "netrpc.h"
#include "Vehicle.h"

#include "Events.h"

Events::Events() {}

Events::~Events() {}

void Events::reset() {

}

bool Events::onRunCommand(std::string command) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnRunCommand(command))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onPrintLog(std::string text) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnPrintLog(text))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onSendInput(std::string input) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnSendInput(input))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onSync() {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnSync())
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onGameText(std::string gameText) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnGameText(gameText);
	}

	if (vars.botFarmerAutomated) {
		if (gameText.find("count") != std::string::npos) {
			FarmCount++;
			if (FarmCount >= 20) {
				FarmGetPay = 1;
				vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
				vars.coordMasterEnabled = 1;
				FarmWork = 0;
				FarmCount = 0;
			}
		}
	}

	return false;
}

void Events::onSpawned() {
	Bot *bot = RakBot::app()->getBot();

	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnSpawned(bot->getPosition(0), bot->getPosition(1), bot->getPosition(2));
	}

	if (vars.virtualWorld) {
		bot->requestClass(rand() % 299);
		return;
	}

	if (SampRpFuncs::isSampRpServer()) {
		if (vars.bQuestEnabled && !vars.coordMasterEnabled) {
			if (!vars.bQuestSpawn) {
				vars.bQuestEnabled = 1;
				bot->sendInput("/quest");
				return;
			} else
				vars.bQuestSpawn = 0;

			if (vars.iQuestStep == 4) {
				if (vars.iBankPutMoney == 1500) {
					vars.botFarmerEnabled = 0;
					vars.coordMasterTarget[0] = 1414.69f;
					vars.coordMasterTarget[1] = -1700.48f;
					vars.coordMasterTarget[2] = 13.54f;
					vars.coordMasterEnabled = 1;
					RakBot::app()->log("[RAKBOT] Автоматическое пополнение баланса на 1500 вирт", vars.iBankPutMoney);
				} else {
					vars.coordMasterTarget[0] = 459.09f;
					vars.coordMasterTarget[1] = -1500.35f;
					vars.coordMasterTarget[2] = 31.04f;
					vars.coordMasterEnabled = 1;
					vars.bBuySkin = 1;
					RakBot::app()->log("[RAKBOT] Покупка скина. Телепорт к магазину...");
				}
			}
			if (vars.iQuestStep == 5) {
				vars.iSetWorkIndex = 1;
				RakBot::app()->log("[RAKBOT] Устройство на работу водителя автобуса", vars.iSetWorkIndex);
				vars.coordMasterTarget[0] = 1480.00f;
				vars.coordMasterTarget[1] = -1771.00f;
				vars.coordMasterTarget[2] = 18.00f;
				vars.coordMasterEnabled = 1;
			}
		}
		if (vars.botAutoSchoolEnabled) {
			vars.coordMasterTarget[0] = -2026.00f;
			vars.coordMasterTarget[1] = -101.00f;
			vars.coordMasterTarget[2] = 35.00f;
			vars.coordMasterEnabled = 1;
			RakBot::app()->log("[RAKBOT] Начало сдачи экзамена на права. Телепорт к автошколе...");
		}
		if (vars.botFarmerEnabled) {
			vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
			vars.coordMasterEnabled = 1;
			vars.botFarmerEnabled = 1;
			RakBot::app()->log("[RAKBOT] Телепорт на ферму %d", FarmIndex);
			return;
		}
		if (vars.botLoaderEnabled) {
			vars.coordMasterTarget[0] = 2126.78f;
			vars.coordMasterTarget[1] = -2281.03f;
			vars.coordMasterTarget[2] = 24.88f;
			vars.coordMasterEnabled = 1;
			RakBot::app()->log("[RAKBOT] Бот грузчика включен");
			return;
		}
		switch (vars.busWorkerRoute) {
			case 1:
				vars.coordMasterTarget[0] = 1258.83f;
				vars.coordMasterTarget[1] = -1810.54f;
				vars.coordMasterTarget[2] = 10.07f;
				vars.busWorkerBusModel = 437;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: городской ЛС");
				vars.coordMasterEnabled = 1;
				return;

			case 2:
				vars.coordMasterTarget[0] = -1985.03f;
				vars.coordMasterTarget[1] = 96.93f;
				vars.coordMasterTarget[2] = 23.82f;
				vars.busWorkerBusModel = 437;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: городской СФ");
				vars.coordMasterEnabled = 1;
				return;

			case 3:
				vars.coordMasterTarget[0] = 2778.17f;
				vars.coordMasterTarget[1] = 1290.91f;
				vars.coordMasterTarget[2] = 6.73f;
				vars.busWorkerBusModel = 437;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: городской ЛВ");
				vars.coordMasterEnabled = 1;
				return;

			case 4:
				vars.busWorkerRouteItem = 0;
				vars.coordMasterTarget[0] = 1654.91f;
				vars.coordMasterTarget[1] = -1050.12f;
				vars.coordMasterTarget[2] = 21.13f;
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: междугородний ЛС-СФ");
				vars.coordMasterEnabled = 1;
				return;

			case 5:
				vars.busWorkerRouteItem = 1;
				vars.coordMasterTarget[0] = 1654.91f;
				vars.coordMasterTarget[1] = -1050.12f;
				vars.coordMasterTarget[2] = 21.13f;
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: междугородний ЛС-ЛВ");
				vars.coordMasterEnabled = 1;
				return;

			case 6:
				vars.busWorkerRouteItem = 2;
				vars.coordMasterTarget[0] = 1654.91f;
				vars.coordMasterTarget[1] = -1050.12f;
				vars.coordMasterTarget[2] = 21.13f;
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: междугородний СФ-ЛВ");
				vars.coordMasterEnabled = 1;
				return;

			case 7:
				vars.busWorkerRouteItem = 3;
				vars.coordMasterTarget[0] = 1654.91f;
				vars.coordMasterTarget[1] = -1050.12f;
				vars.coordMasterTarget[2] = 21.13f;
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: пригородный ЛС-ФК");
				vars.coordMasterEnabled = 1;
				return;

			case 8:
				vars.busWorkerRouteItem = 4;
				vars.coordMasterTarget[0] = 1654.91f;
				vars.coordMasterTarget[1] = -1050.12f;
				vars.coordMasterTarget[2] = 21.13f;
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] Выбран маршрут автобуса: пригородный ЛС-ЗАВОД");
				vars.coordMasterEnabled = false;
				return;
		}
	}
	if (vars.savedTeleportEnabled && !vars.checkPointMaster) {
		vect3_copy(vars.savedCoords, vars.coordMasterTarget);
		vars.coordMasterEnabled = false;
		RakBot::app()->log("[RAKBOT] Телепорт на сохраненные координаты");
		return;
	}
}

bool Events::onSpawn() {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnSpawn())
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

void Events::onSetSpawnPos(float positionX, float positionY, float positionZ) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnSetSpawnPos(positionX, positionY, positionZ);
	}
}

bool Events::onSetPosition(float positionX, float positionY, float positionZ) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnSetPosition(positionX, positionY, positionZ))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onSetHealth(uint8_t health) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnSetHealth(health))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onSetArmour(uint8_t armour) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnSetArmour(armour))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onServerMessage(std::string message) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnServerMessage(message))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	return SampRpFuncs::onServerMessage(message);
}

bool Events::onChatMessage(uint16_t playerId, std::string message) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnChatMessage(playerId, message))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onDialogShow(uint16_t dialogId, uint8_t dialogStyle, std::string dialogTitle, std::string okButtonText, std::string cancelButtonText, std::string dialogText) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr) {
			if (script->luaOnDialogShow(dialogId, dialogStyle, dialogTitle, okButtonText, cancelButtonText, dialogText))
				luaResult = true;
		}
	}
	if (luaResult)
		return true;

	Bot *bot = RakBot::app()->getBot();

	if (dialogId == vars.dialogIdBan) {
		RakBot::app()->log("[RAKBOT] Аккаунт заблокирован!");
		bot->disconnect(false);
		RakBot::app()->exit();
		return true;
	}

	if (dialogId == vars.dialogIdPassword) {
		RakBot::app()->log("[RAKBOT] Ввод пароля...");
		bot->dialogResponse(1, 1, 0, RakBot::app()->getSettings()->getLoginPassword());
		return true;
	}

	if (SampRpFuncs::isSampRpServer()) {
		if (vars.botFarmerEnabled) {
			if (dialogId == 135) {
				bot->dialogResponse(dialogId, FarmGetPay ? 0 : 1, 0, std::string());
				if (FarmGetPay)
					FarmGetPay = 0;
				return true;
			}
			if (dialogId == 126)
				return true;
		}

		if (vars.iSetWorkIndex) {
			if (dialogId == 6) {
				RakBot::app()->log("[RAKBOT] Устройство на работу...");
				bot->dialogResponse(dialogId, 1, 0, std::string());
				return true;
			}
			if (dialogId == 7) {
				RakBot::app()->log("[RAKBOT] Выбор работы...");
				bot->dialogResponse(dialogId, 1, vars.iSetWorkIndex - 1);
				vars.iSetWorkIndex = 0;
				Sleep(1500);
				if (vars.bQuestEnabled) {
					vars.bQuestEnabled = 0;
					vars.iQuestStep = 0;
					vars.busWorkerRoute = 6;
				}
				bot->spawn();
				return true;
			}
		}

		if (vars.botAutoSchoolEnabled) {
			if (dialogId == 96 || dialogId == 97) {
				bot->dialogResponse(dialogId);
				vars.botAutoSchoolActive = 1;
				return true;
			}
		}

		if (vars.busWorkerRoute) {
			switch (dialogId) {
				case 276:
					bot->dialogResponse(dialogId);
					return true;

				case 169:
					bot->dialogResponse(dialogId);
					return true;

				case 17:
					bot->dialogResponse(dialogId);
					return true;

				case 18:
					bot->dialogResponse(dialogId, 1, vars.busWorkerRouteItem);
					return true;
			}
		}

		if (vars.botLoaderEnabled && dialogId == 128) {
			Sleep(500);
			bot->dialogResponse(dialogId);
			return true;
		}

		if (vars.autoRegEnabled) {
			switch (dialogId) {
				case 2:
					RakBot::app()->log("[RAKBOT] Регистрация аккаунта...");
					bot->dialogResponse(dialogId, 1, 0, RakBot::app()->getSettings()->getLoginPassword());
					return true;

				case 3:
					bot->dialogResponse(dialogId);
					return true;

				case 21:
					bot->dialogResponse(dialogId, 1, 0, vars.autoRegMail);
					return true;

				case 109:
					bot->dialogResponse(dialogId, 1, 0, vars.autoRegReferer);
					return true;

				case 4:
					bot->dialogResponse(dialogId, vars.autoRegSex);
					return true;
			}
		}
	}

	if (SampRpFuncs::isSampRpServer()) {
		if (vars.bQuestEnabled) {
			if (dialogId == 259) {
				bot->dialogResponse(dialogId);
				return true;
			}
			if (dialogId == 225 || dialogId == 228) {
				bot->dialogResponse(dialogId);
				return true;
			}
			if (dialogId == 229) {
				if (dialogText.find("скин") != std::string::npos) {
					vars.botFarmerEnabled = 0;
					vars.bQuestSpawn = 1;
					vars.iQuestStep = 4;
					vars.iBankPutMoney = 1500;
					bot->spawn();
					return true;
				}
				if (dialogText.find("20 мешков") != std::string::npos) {
					vars.iQuestStep = 1;
					RunCommand("!botloader");
					return true;
				}
				if (dialogText.find("50 единиц") != std::string::npos) {
					vars.botAutoSchoolEnabled = 0;
					vars.bQuestSpawn = 1;
					vars.botFarmerAutomated = 1;
					vars.iQuestStep = 3;
					vars.botFarmerEnabled = 1;
					bot->spawn();
					return true;
				}
				if (dialogText.find("права") != std::string::npos) {
					vars.botLoaderEnabled = 0;
					vars.iQuestStep = 2;
					vars.bQuestSpawn = 1;
					vars.botAutoSchoolEnabled = 1;
					bot->spawn();
					return true;
				}
				if (dialogText.find("в мэрию") != std::string::npos) {
					vars.iQuestStep = 5;
					vars.bQuestSpawn = 1;
					bot->spawn();
					return true;
				}
				bot->dialogResponse(dialogId);
				return true;
			}
		}

		if (vars.getBalanceEnabled) {
			switch (dialogId) {
				case 22:
				{
					if (dialogText.find("ATM") != std::string::npos) {
						if (!vars.getBalanceFinished) {
							bot->dialogResponse(dialogId);
							vars.getBalanceFinished = true;
						} else {
							bot->dialogResponse(dialogId);
							vars.getBalanceFinished = false;
							vars.getBalanceEnabled = false;
						}
						return true;
					}
				}
			}
		}
	}

	if (vars.parseStatistic && SampRpFuncs::isSampRpServer()) {
		switch (dialogId) {
			case 22:
			{
				if (dialogTitle.find("MainMenu") != std::string::npos) {
					bot->dialogResponse(dialogId, 1, 3);
					return true;
				}

				if (dialogTitle.find("Информация") != std::string::npos) {
					bot->dialogResponse(dialogId);
					return true;
				}
				if (dialogTitle.find("Статистика персонажа") != std::string::npos) {
					char path[MAX_PATH], buf[256];

					Settings *settings = RakBot::app()->getSettings();
					GetModuleFileName(NULL, buf, sizeof(buf));
					strcpy(strrchr(buf, '\\') + 1, "stats");
					CreateDirectoryA(buf, NULL);
					snprintf(path, MAX_PATH, "%s\\%s;%s;%d.log",
						buf, settings->getName().c_str(), settings->getAddress()->getIp().c_str(), settings->getAddress()->getPort());

					FILE *f = fopen(path, "w");
					std::stringstream ss;
					ss << dialogText << "\nПароль: " << RakBot::app()->getSettings()->getLoginPassword() << std::endl;
					::fwrite(ss.str().c_str(), ss.str().length(), 1, f);
					::fclose(f);

					RakBot::app()->log("[RAKBOT] Информация аккаунта записана в файл");
					bot->dialogResponse(dialogId);
					return true;
				}
			}
		}
	}

	switch (vars.skipDialog) {
		case 1:
			RakBot::app()->log("[RAKBOT] Пропуск диалога %d с нижатием 1й кнопки(ENTER)", dialogId);
			bot->dialogResponse(dialogId);
			return true;

		case 2:
			RakBot::app()->log("[RAKBOT] Пропуск диалога %d с нижатием 2й кнопки(ESC)", dialogId);
			bot->dialogResponse(dialogId, 0);
			return true;

		case 3:
			RakBot::app()->log("[RAKBOT] Пропуск диалога %d", dialogId);
			return true;
	}

	return false;
}

bool Events::onDialogResponse(uint16_t dialogId, uint8_t dialogButton, uint16_t dialogItem, std::string dialogInput) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnDialogResponse(dialogId, dialogButton, dialogItem, dialogInput))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

void Events::onSetSkin(uint16_t playerid, uint16_t skinId) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnSetSkin(playerid, skinId);
	}
}

void Events::onApplyAnimation(uint16_t playerId, uint16_t animId) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnApplyAnimation(playerId, animId);
	}
}

void Events::onConnect(uint16_t playerId) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnConnect(playerId);
	}
}

void Events::onDisconnect(uint8_t reason) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnDisconnect(reason);
	}
}

void Events::onSetMoney(int money) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnSetMoney(money);
	}
}

void Events::onGameInited(std::string serverName) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnGameInited();
	}
}

void Events::onCreateVehicle(Vehicle * vehicle) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnCreateVehicle(vehicle->getVehicleId());
	}
}

void Events::onDestroyVehicle(Vehicle *vehicle) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnDestroyVehicle(vehicle->getVehicleId());
	}
}

void Events::onSetVehicleParams(Vehicle * vehicle) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnSetVehicleParams(vehicle->getVehicleId());
	}
}

void Events::onPlayerJoin(Player *player) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnPlayerJoin(player->getPlayerId(), player->getName());
	}
}

void Events::onPlayerQuit(Player *player, uint8_t reason) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnPlayerQuit(player->getPlayerId(), reason);
	}
}

void Events::onPlayerRemoveFromWorld(Player * player) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnPlayerRemoveFromWorld(player->getPlayerId());
	}
}

void Events::onPlayerDeath(Player * player) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnPlayerDeath(player->getPlayerId());
	}
}

void Events::onDeath() {
	Bot *bot = RakBot::app()->getBot();

	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnPlayerDeath(bot->getPlayerId());
	}
}

void Events::onPlayerAddInWorld(Player * player) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnPlayerAddInWorld(player->getPlayerId());
	}
}

void Events::onTextDrawHide(uint16_t textDrawId) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnTextDrawHide(textDrawId);
	}
}

void Events::onTextDrawShow(uint16_t textDrawId, float positionX, float positionY, std::string textDrawString) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnTextDrawShow(textDrawId, positionX, positionY, textDrawString);
	}
}

void Events::onTextDrawSetString(uint16_t textDrawId, std::string string) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnTextDrawSetString(textDrawId, string);
	}
}

bool Events::onTextDrawClick(uint16_t textDrawId) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnTextDrawClick(textDrawId))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

void Events::onTextLabelShow(uint16_t labelId, float positionX, float positionY, float positionZ, std::string labelString) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnTextLabelShow(labelId, positionX, positionY, positionZ, labelString);
	}
}

void Events::onToggleSpectating(bool state) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnToggleSpectating(state);
	}
}

bool Events::onPutInVehicle(Vehicle *vehicle, uint8_t seatId) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnPutInVehicle(vehicle->getVehicleId(), seatId))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onEjectFromVehicle() {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnEjectFromVehicle())
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

void Events::onCreatePickup(Pickup * pickup) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnCreatePickup(pickup->getPickupId());
	}
}

void Events::onDestroyPickup(Pickup * pickup) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnDestroyPickup(pickup->getPickupId());
	}
}

bool Events::onPickUpPickup(Pickup * pickup) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnPickUpPickup(pickup->getPickupId()))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

void Events::onCreateCheckpoint(Checkpoint * checkpoint) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnCreateCheckpoint();
	}
}

void Events::onDestroyCheckpoint(Checkpoint * checkpoint) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnDestroyCheckpoint();
	}
}

void Events::onCreateRaceCheckpoint(RaceCheckpoint * raceCheckpoint) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnCreateRaceCheckpoint();
	}
}

void Events::onDestroyRaceCheckpoint(RaceCheckpoint * raceCheckpoint) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnDestroyRaceCheckpoint();
	}
}

void Events::onCreateObject(GTAObject * object) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnCreateObject(object->usObjectId);
	}

	if (object->ulModelId == 1317 && SampRpFuncs::isSampRpServer()) {
		vect3_copy(object->position, checkpoint.position);
		checkpoint.position[2] += 1.5f;
		checkpoint.size = 1.0f;
		checkpoint.active = 1;
	}
}

void Events::onDestroyObject(GTAObject * object) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnDestroyObject(object->usObjectId);
	}

	if (object->ulModelId == 1317 && SampRpFuncs::isSampRpServer()) {
		checkpoint.active = 0;
	}
}

void Events::onAttachObjectToPlayer(uint16_t playerId, uint32_t slotId, bool attach) {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnAttachObjectToPlayer(playerId, slotId, attach);
	}

	Bot *bot = RakBot::app()->getBot();

	if (vars.botLoaderEnabled && playerId == bot->getPlayerId()) {
		if (!attach) {
			RakBot::app()->log("[RAKBOT] Бот грузчика: мешок сдан!");
			BotWithBag = false;
		} else {
			BotWithBag = true;
		}
	}
}

bool Events::onTakeCheckpoint(float positionX, float positionY, float positionZ) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnTakeCheckpoint(positionX, positionY, positionZ))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onTeleport(float positionX, float positionY, float positionZ) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnTeleport(positionX, positionY, positionZ))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onCoordMasterStart(float targetX, float targetY, float targetZ) {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnCoordMasterStart(targetX, targetY, targetZ))
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

bool Events::onCoordMasterStop() {
	bool luaResult = false;
	for each (Script *script in scripts) {
		if (script != nullptr)
			if (script->luaOnCoordMasterStop())
				luaResult = true;
	}
	if (luaResult)
		return true;

	return false;
}

void Events::onCoordMasterComplete() {
	for each (Script *script in scripts) {
		if (script != nullptr)
			script->luaOnCoordMasterComplete();
	}
}
