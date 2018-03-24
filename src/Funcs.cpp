#include "RakBot.h"

#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "RakNet.h"
#include "Settings.h"
#include "Pickup.h"
#include "Vehicle.h"

#include "MiscFuncs.h"
#include "MathStuff.h"
#include "SampRpFuncs.h"

#include "main.h"
#include "mapwnd.h"
#include "netgame.h"
#include "netrpc.h"
#include "window.h"

#include "Funcs.h"

Funcs::Funcs() {}

Funcs::~Funcs() {}

void NoAfk() {
	if (!vars.noAfk || !vars.syncAllowed)
		return;

	if (vars.stickEnabled || vars.followEnabled)
		return;

	if (static_cast<int>(GetTickCount() - BotSpawnedTime) < vars.afterSpawnDelay)
		return;

	static uint32_t timer = 0;
	if (static_cast<int>(GetTickCount() - timer) < vars.noAfkDelay)
		return;

	Bot *bot = RakBot::app()->getBot();
	bot->sync();
}

void RoutePlay() {
	/* Bot *bot = RakBot::getInstance()->getBot();

	BitStream data;
	vars.routeIndex = 0;

	while (vars.routeEnabled && !vars.bBotExit) {
		Sleep(static_cast<uint32_t>(vars.routeSpeed));

		if (bot->isSpawned()) {
			if (vars.routeIndex >= vars.routeData.size()) {
				if (!vars.routeLoop) {
					vars.routeEnabled = false;
					vars.noAfkEnabled = true;

					RakBot::app()->log("[RAKBOT] Сохраненный маршрут: остановлен");
					break;
				} else {
					RakBot::app()->log("[RAKBOT] Сохраненный маршрут: повтор");
					vars.routeIndex = 0;
				}
			}

			vect4_copy(vars.routeData[vars.routeIndex].quaternion, players[localPlayerID].onfootData.quaternion);
			vect3_copy(vars.routeData[vars.routeIndex].speed, players[localPlayerID].onfootData.speed);
			vect3_copy(vars.routeData[vars.routeIndex].position, players[localPlayerID].onfootData.position);
			players[localPlayerID].onfootData.animFlags = vars.routeData[vars.routeIndex].animFlags;
			players[localPlayerID].onfootData.animId = vars.routeData[vars.routeIndex].animId;
			players[localPlayerID].onfootData.keys = vars.routeData[vars.routeIndex].keys;
			players[localPlayerID].onfootData.leftRightKey = vars.routeData[vars.routeIndex].leftRightKey;
			players[localPlayerID].onfootData.upDownKey = vars.routeData[vars.routeIndex].upDownKey;

			data.Reset();
			data.Write<uint8_t>(ID_PLAYER_SYNC);
			data.Write(reinterpret_cast<char *>(&players[localPlayerID].onfootData), sizeof(OnfootData));
			rakClient->Send(&data, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

			vars.routeIndex++;
		}
	} */
}

void CheckChangePos() {
	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return;

	static float fOldPos[3] = { 0, 0, 0 };
	for (int i = 0; i < 3; i++) {
		if (fOldPos[i] != bot->getPosition(i)) {
			if (vars.mapWindowOpened)
				UpdateMapWindow();

			vars.lastChangePos = GetTickCount();
			for (int i = 0; i < 3; i++)
				fOldPos[i] = bot->getPosition(i);
			break;
		}
	}
	if (raceCheckpoint.active) {
		static float fOldCP[3] = { 0, 0, 0 };
		for (int i = 0; i < 3; i++) {
			if (fOldCP[i] != raceCheckpoint.position[i]) {
				if (vars.mapWindowOpened)
					UpdateMapWindow();

				vect3_copy(raceCheckpoint.position, fOldCP);
				break;
			}
		}
	}
}

void UpdateInfo() {
	Bot *bot = RakBot::app()->getBot();

	static uint32_t timer = 0;
	if ((GetTickCount() - timer) < 500)
		return;
	timer = GetTickCount();

	if (bot->isConnected()) {
		char botInfo[1024];
		snprintf(botInfo, sizeof(botInfo),
			"Игроки: %d\nПинг: %d\nКоординаты:\n X: %0.2f\n Y: %0.2f\n Z: %0.2f\nЗдоровье: %d\nДеньги: %d\nАнимация: %d/%d\nУровень: %d\nСтатус: %s",
			RakBot::app()->getPlayersCount(), bot->getInfo()->getPing(), bot->getPosition(0), bot->getPosition(1), bot->getPosition(2),
			bot->getHealth(), bot->getMoney(),
			bot->getAnimation()->getAnimId(), bot->getAnimation()->getAnimFlags(), bot->getInfo()->getScore(),
			bot->getPlayerStateName().c_str());

		if ((bot->getPlayerState() == PLAYER_STATE_DRIVER) || (bot->getPlayerState() == PLAYER_STATE_PASSENGER)) {
			std::stringstream ss;
			ss << "\nТранспорт: " << bot->getVehicle()->getVehicleId();
			strcat(botInfo, ss.str().c_str());
		}

		SetWindowText(g_hWndTitle, botInfo);
	} else {
		SetWindowText(g_hWndTitle, "RakBot " RAKBOT_VERSION "\nАвтор: MishaN");
	}
}

void CheckOnlineAndId() {
	Bot *bot = RakBot::app()->getBot();

	if (bot->isConnected()) {
		if (vars.checkIdEnabled) {
			uint16_t playerId = bot->getPlayerId();
			if (playerId > vars.maxId || playerId < vars.minId) {
				RakBot::app()->log("[RAKBOT] Неподходящий ID (%d). Переподключение...", playerId);
				bot->disconnect(false);
				bot->reconnect(vars.reconnectDelay);
			}
		}

		if (((GetTickCount() - GameInitedTime) > 5000)) {
			if (vars.checkOnlineEnabled) {
				int playersCount = RakBot::app()->getPlayersCount();
				if (playersCount > vars.maxOnline || playersCount < vars.minOnline) {
					RakBot::app()->log("[RAKBOT] Неподходящий онлайн (%d). Переподключение...", playersCount);
					bot->disconnect(false);
					bot->reconnect(vars.reconnectDelay);
				}
			}
		}
	}
}

void Follow() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.followEnabled || vars.coordMasterEnabled || vars.stickEnabled || !bot->isSpawned())
		return;

	Player *player = RakBot::app()->getPlayer(vars.followPlayerID);

	if (player->isInStream()) {
		bot->follow(player->getPlayerId());
		return;
	}
	bot->sync();
}

void AdminChecker() {
	static uint32_t timer = 0;
	if ((GetTickCount() - timer) < 500)
		return;
	timer = GetTickCount();

	Bot *bot = RakBot::app()->getBot();
	if (!bot->isConnected()) {
		SetWindowText(g_hWndAdmins, "Загрузка игроков...");
		SetWindowText(g_hWndAdminsArea, "Админы онлайн");
		return;
	}

	char szAdminList[2048];
	ZeroMemory(szAdminList, sizeof(szAdminList));
	int iCount = 0, iCountNear = 0;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		Player *player = RakBot::app()->getPlayer(i);
		if (player == nullptr)
			continue;

		if (player->getInfo()->getScore() < 1)
			continue;

		if (player->isAdmin()) {
			switch (vars.adminActionOnline) {
				case 1:
					RakBot::app()->log("[RAKBOT] Админ в сети, ПЕРЕПОДКЛЮЧЕНИЕ");
					bot->disconnect(false);
					bot->reconnect(vars.adminReconnectDelay);
					return;

				case 2:
					RakBot::app()->log("[RAKBOT] Админ в сети, ВЫХОД");
					bot->disconnect(false);
					RakBot::app()->exit();
					return;
			}

			char szBuf[128];
			ZeroMemory(szBuf, sizeof(szBuf));

			if (player->isInStream()) {
				switch (vars.adminActionNear) {
					case 1:
						RakBot::app()->log("[RAKBOT] Админ рядом, ПЕРЕПОДКЛЮЧЕНИЕ");
						bot->disconnect(false);
						bot->reconnect(vars.adminReconnectDelay);
						return;

					case 2:
						RakBot::app()->log("[RAKBOT] Админ рядом, ВЫХОД");
						bot->disconnect(false);
						RakBot::app()->exit();
						return;
				}

				strcat_s(szBuf, sizeof(szBuf), "!");
				iCountNear++;
			}
			sprintf_s(szBuf, sizeof(szBuf), "%s[%d] | L:%d\n", player->getName().c_str(), i, player->getInfo()->getScore());
			strcat_s(szAdminList, sizeof(szAdminList), szBuf);
			iCount++;
		}
	}
	if (iCount == 0) {
		SetWindowText(g_hWndAdmins, "Нет админов онлайн");
		SetWindowText(g_hWndAdminsArea, "Админы онлайн (0/0)");
	} else {
		SetWindowText(g_hWndAdmins, szAdminList);
		sprintf_s(szAdminList, sizeof(szAdminList), "Админы онлайн (%d/%d)", iCountNear, iCount);
		SetWindowText(g_hWndAdminsArea, szAdminList);
	}
}

int LoaderStep = -1;
int BagCount = 0;
bool BotWithBag = false;
uint32_t BotTakenBagTime = UINT32_MAX;

void BotLoader() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.botLoaderEnabled || !bot->isSpawned() || vars.coordMasterEnabled || SampRpFuncs::isBotSuspended())
		return;

	if (!vars.sendBadSync)
		vars.sendBadSync = true;

	if (LoaderStep == BOTLOADER_STEP_STARTWORK) {
		if (bot->getSkin() == 260 || bot->getSkin() == 16 || bot->getSkin() == 27) {
			LoaderStep = BOTLOADER_STEP_TAKEBAG;
			bot->sync(2231.10f, -2285.39f, 11.88f);
			return;
		}

		int pickupId = FindNearestPickup(1275);
		if (pickupId == PICKUP_ID_NONE)
			return;

		RakBot::app()->log("[RAKBOT] Начало работы грузчика...");
		SampRpFuncs::pickUpPickup(pickupId);
		return;
	}

	if (LoaderStep == BOTLOADER_STEP_TAKEBAG) {
		/* if ((BagCount >= vars.botLoaderCount) && (BagCount % vars.botLoaderCount == 0)) {
			bot->sync(2160.f, -2265.f, 14.08f);
			LoaderStep = BOTLOADER_STEP_GETPAY;
			return;
		} */

		if (BotWithBag) {
			LoaderStep = BOTLOADER_STEP_WAITING;
			return;
		}

		SampRpFuncs::takeCheckpoint();
		LoaderStep = BOTLOADER_STEP_WAITING;
		return;
	}

	if (LoaderStep == BOTLOADER_STEP_WAITING) {
		if (!BotWithBag) {
			LoaderStep = BOTLOADER_STEP_TAKEBAG;
			return;
		}

		if (BotWithBag && (BotTakenBagTime == UINT32_MAX)) {
			bot->sync(2231.10f, -2285.39f, 11.88f);
			BotTakenBagTime = GetTickCount();
			return;
		}

		if (BotWithBag && (static_cast<int>(GetTickCount() - BotTakenBagTime) > (vars.botLoaderDelay - 3000))) {
			LoaderStep = BOTLOADER_STEP_PUTBAG;
			BotTakenBagTime = UINT32_MAX;
			return;
		}
		return;
	}

	if (LoaderStep == BOTLOADER_STEP_PUTBAG) {
		if (!BotWithBag) {
			LoaderStep = BOTLOADER_STEP_TAKEBAG;
			bot->sync(2231.10f, -2285.39f, 11.88f);
			return;
		}

		SampRpFuncs::takeCheckpoint();
		LoaderStep = BOTLOADER_STEP_TAKEBAG;
		return;
	}

	/* if (LoaderStep == BOTLOADER_STEP_GETPAY) {
		int pickupId = FindNearestPickup(1274);
		if (pickupId == PICKUP_ID_NONE)
			return;

		RakBot::app()->log("[RAKBOT] Получение ЗП грузчика...");
		SampRpFuncs::pickUpPickup(pickupId);
		bot->sync(2160.f, -2265.f, 14.08f);
		LoaderStep = BOTLOADER_STEP_STARTWORK;
		BagCount = 0;
		return;
	} */

	/* int iPickupID = ;
	if (iPickupID != -1) {
		switch (LoaderStep) {
			case 0:
			{
				if (bot->getSkin() != 260 && bot->getSkin() != 16 && bot->getSkin() != 27) {
					LoaderStep = 6;
					break;
				} else if (BagCount >= vars.botLoaderCount) {
					BagCount = 0;
					LoaderStep = 5;
					bot->sync(2160.f, -2265.f, 14.08f);
					Sleep(500);
					int iGetPayPickup = FindNearestPickup(1274);
					SampRpFuncs::pickUpPickup(iGetPayPickup);
					RakBot::app()->log("[RAKBOT] Получение ЗП грузчика...");
				} else {
					bot->sync(2231.10f, -2285.39f, 11.88f);
					LoaderStep = 1;
				}
				Sleep((uint32_t)(vars.botLoaderDelay * 0.1875f));
			}
			break;

			case 1:
			{
				bot->sync(2231.11f, -2285.40f, 14.38f);
				LoaderStep = 2;
			}
			break;

			case 2:
			{
				bot->sync(2231.10f, -2285.39f, 11.88f);
				LoaderStep = 3;
				Sleep((uint32_t)((vars.botLoaderDelay * 0.625f) - 250));
			}
			break;

			case 3:
			{
				bot->sync(2171.69f, -2255.39f, 10.80f);
				LoaderStep = 4;
				Sleep((uint32_t)(vars.botLoaderDelay * 0.1875f));
			}
			break;

			case 4:
			{
				int iLoadPickupID = FindNearestPickup(19197);
				if (iLoadPickupID != -1 && vars.botLoaderCheckVans) {
					RakBot::app()->log("[RAKBOT] Сдача мешка в машину");
					SampRpFuncs::pickUpPickup(iLoadPickupID);
				} else
					bot->sync(2171.70f, -2255.40f, 13.30f);

				bot->sync(2195.07f, -2269.02f, -10.00f);
				Sleep(250);
				LoaderStep = 0;
			}
			break;

			case 5:
			{
				SampRpFuncs::pickUpPickup(FindNearestPickup(1274));
				LoaderStep = 0;
				BagCount = 0;
			}
			break;

			case 6:
			{
				RakBot::app()->log("[RAKBOT] Начало работы грузчика...");
				SampRpFuncs::pickUpPickup(iPickupID);
				LoaderStep = 0;

				if (vars.savedTeleportEnabled) {
					vect3_copy(vars.savedCoords, vars.coordMasterTarget);
					vars.coordMasterEnabled = true;
					RakBot::app()->log("[RAKBOT] Телепорт на сохраненные координаты");
				}
			}
			break;
		}
	} else {
		BagCount = 0;
		LoaderStep = 6;
	} */
}

void BusBot() {
	Bot *bot = RakBot::app()->getBot();

	if (bot->getPlayerState() == PLAYER_STATE_DRIVER)
		return;

	if (vars.coordMasterEnabled)
		return;

	if (!vars.busWorkerRoute)
		return;

	static uint32_t timer = 0;
	if (GetTickCount() - timer < 1500)
		return;
	timer = GetTickCount();

	bot->sync();

	int busId = FindNearestVehicleByModel(vars.busWorkerBusModel);
	Vehicle *vehicle = RakBot::app()->getVehicle(busId);
	if (vehicle == nullptr)
		return;

	for (int i = 0; i < 3; i++)
		bot->setPosition(i, vehicle->getPosition(i));
	bot->sync();
	bot->wait(500);
	bot->enterVehicle(busId, 0);
}

float FarmFieldPos[5][3] =
{
	{ -281.77f, -1369.35f, 9.68f },
	{ -200.11f, 45.97f, 3.12f },
	{ -1022.36f, -1025.35f, 129.22f },
	{ 50.33f, -40.53f, 0.88f },
	{ 1922.23f, 194.19f, 34.25f }
};

float FarmPos[5][3] =
{
	{ -398.97f, -1425.89f, 26.32f },
	{ -108.58f, -3.05f, 3.12f },
	{ -1060.43f, -1195.93f, 129.56f },
	{ -2.98f, 74.82f, 3.12f },
	{ 1918.44f, 173.35f, 37.27f }
};

bool FarmWork = false;
int FarmCount = 0;
int FarmIndex = 0;
bool ChangeFarm = false;
bool FarmGetPay = 0;

#define FARM_FIRST_PICK 901

void FarmerBot() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.botFarmerEnabled || vars.coordMasterEnabled || !bot->isSpawned() || SampRpFuncs::isBotSuspended())
		return;

	static uint32_t timer = 0;
	if (GetTickCount() - timer < 1000)
		return;
	timer = GetTickCount();

	if (!vars.sendBadSync)
		vars.sendBadSync = true;

	if (FarmWork) {
		if (!checkpoint.active) {
			int pickupId = FindNearestPickup(19197);
			Pickup *pickup = RakBot::app()->getPickup(pickupId);

			if (pickup != nullptr) {
				if (bot->distanceTo(pickup) < 50.0f) {
					bot->pickUpPickup(pickupId, FALSE);
					RakBot::app()->log("[RAKBOT] Сдача куста в машину %d", pickupId);
				} else {
					for (int i = 0; i < 3; i++)
						vars.coordMasterTarget[i] = pickup->getPosition(i);
					vars.coordMasterTarget[2] -= 5.0f;
					vars.coordMasterEnabled = true;
					RakBot::app()->log("[RAKBOT] Телепорт к машине");
				}
			} else {
				static uint32_t notFoundCarsTimer = 0;

				if (vars.botFarmerAutomated && vars.bQuestEnabled) {
					RakBot::app()->log("[RAKBOT] Не удается найти машины! Смена фермы...");

					FarmGetPay = 1;
					vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
					vars.coordMasterEnabled = 1;
					FarmWork = 0;
					FarmCount = 0;
					ChangeFarm = 1;
				} else if (GetTickCount() - notFoundCarsTimer > 8000) {
					RakBot::app()->log("[RAKBOT] Не удается найти машины!");
					notFoundCarsTimer = GetTickCount();
				}
				return;
			}
		} else {
			bot->takeCheckpoint();
		}
	} else {
		int pickupId = FARM_FIRST_PICK + (FarmIndex * 2);
		Pickup *pickup = RakBot::app()->getPickup(pickupId);

		if (pickup) {
			if (bot->distanceTo(pickup) < 50.0f) {
				bot->pickUpPickup(pickupId, FALSE);
				RakBot::app()->log("[RAKBOT] Пользуемся раздевалкой...");
			} else {
				for (int i = 0; i < 3; i++)
					vars.coordMasterTarget[i] = pickup->getPosition(i);
				vars.coordMasterTarget[2] -= 5.0f;
				vars.coordMasterEnabled = 1;
				RakBot::app()->log("[RAKBOT] Телепорт к раздевалке");
			}
		} else {
			vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
			vars.coordMasterTarget[2] -= 5.0f;
			vars.coordMasterEnabled = 1;
			RakBot::app()->log("[RAKBOT] Телепорт к раздевалке");
		}
	}
}

void CoordMaster() {
	if (!vars.coordMasterEnabled || !vars.syncAllowed)
		return;

	Bot *bot = RakBot::app()->getBot();

	if (static_cast<int>(GetTickCount() - BotSpawnedTime) < vars.afterSpawnDelay)
		return;

	if (!bot->isSpawned())
		return;

	bot->sync();

	static uint32_t timer = 0;
	if (static_cast<int>(GetTickCount() - timer) < vars.coordMasterDelay)
		return;

	float position[3];
	for (int i = 0; i < 3; i++)
		position[i] = bot->getPosition(i);

	float fDist = vect3_dist(position, vars.coordMasterTarget);
	float fDistXY = vect2_dist(position, vars.coordMasterTarget);

	if (fDist == 0.f)
		return;

	if (fDist > vars.coordMasterDist) {
		if (fDistXY > vars.coordMasterDist) {
			if (position[2] != vars.coordMasterHeight) {
				if (abs(vars.coordMasterHeight - position[2]) > vars.coordMasterDist)
					position[2] +=
					(vars.coordMasterDist * (position[2] > vars.coordMasterHeight ? -1.0f : 1.0f));
				else
					position[2] = vars.coordMasterHeight;
			} else {
				float fDistSin = (vars.coordMasterTarget[0] - position[0]) / fDistXY;
				float fDistCos = (vars.coordMasterTarget[1] - position[1]) / fDistXY;

				position[0] += fDistSin * vars.coordMasterDist;
				position[1] += fDistCos * vars.coordMasterDist;
			}
		} else if (fDistXY != 0.0f) {
			position[0] = vars.coordMasterTarget[0];
			position[1] = vars.coordMasterTarget[1];
		} else {
			if (abs(vars.coordMasterTarget[2] - position[2]) > vars.coordMasterDist)
				position[2] +=
				(vars.coordMasterDist * (position[2] > vars.coordMasterTarget[2] ? -1.0f : 1.0f));
			else
				position[2] = vars.coordMasterTarget[2];
		}
	} else {
		vect3_copy(vars.coordMasterTarget, position);
		RakBot::app()->log("[RAKBOT] Вы достигли места назначения!");

		std::thread coordMasterOffThread([]() {
			Sleep(vars.coordMasterDelay);
			vars.coordMasterEnabled = false;
			RakBot::app()->log("[RAKBOT] Коордмастер завершил работу");
		});
		coordMasterOffThread.detach();
	}

	for (int i = 0; i < 3; i++)
		bot->setPosition(i, position[i]);
	bot->sync();

	timer = GetTickCount();
}

void GetBalance() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.getBalanceEnabled || vars.coordMasterEnabled)
		return;

	static uint32_t timer = 0;
	if (GetTickCount() - timer < 3000)
		return;

	bot->sendInput("/atm");
	timer = GetTickCount();
}

void Stick() {
	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return;

	if (!vars.stickEnabled || vars.followEnabled)
		return;

	Player *player = RakBot::app()->getPlayer(vars.followPlayerID);
	if (player == nullptr)
		return;

	if (!player->isInStream())
		return;

	for (int i = 0; i < 4; i++)
		bot->setQuaternion(i, player->getQuaternion(i));

	for (int i = 0; i < 3; i++)
		bot->setPosition(i, player->getPosition(i));

	/* for (int i = 0; i < 3; i++)
		bot->setSpeed(i, player->getSpeed(i)); */

	bot->sync();
}

void AntiAFK() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.antiAfkEnabled)
		return;

	static float k = 1.f;
	static uint32_t timer = 0;
	if (GetTickCount() - timer < vars.antiAfkDelay)
		return;
	timer = GetTickCount();

	float offset = (vars.antiAfkOffset != 0.f) ? vars.antiAfkOffset : 0.01f;
	bot->setPosition(0, bot->getPosition(0) + (offset * k));
	bot->sync();

	if (k < 1.f)
		k += 0.5f;
	else
		k *= -1.f;
}

void AutoLicensePass() {
	/* Bot *bot = RakBot::getInstance()->getBot();

	static uint32_t dwTime = 0;

	if (vars.botAutoSchoolEnabled && !vars.coordMasterEnabled) {
		if (GetTickCount() - dwTime > 3000) {
			if (localPlayerID == 65535) {
				return;
			}

			if (localVehicleId != 65535) {
				return;
			}

			if (!raceCheckpoint.isActive) {
				int iDoorPickupID = FindNearestPickup(1318);

				if (pickups[iDoorPickupID].position[2] < 1000.0f &&
					vect3_dist(pickups[iDoorPickupID].position, players[localPlayerID].onfootData.position) < 50.0f) {
					if (vars.botAutoSchoolActive) {
						for (VEHICLEID i = 0; i < MAX_VEHICLES; i++) {
							if (vehicles[i].isExists) {
								if (vehicles[i].model == 426 && vehicles[i].firstColor == 79 && vehicles[i].secondColor == 79) {
									vect3_copy(vehicles[i].position, players[localPlayerID].onfootData.position);
									OnfootSync();

									Sleep(1000);

									RakNet::BitStream bsSend;
									bsSend.Write(i);
									bsSend.Write(0);
									rakClient->RPC(&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

									RakBot::app()->log("[RAKBOT] Начало сдачи экзамена на машине с ID %d", i);
									localVehicleId = i;
									DriverSync();
									vars.checkPointMaster = 1;
									break;
								}
							}
						}
					} else {
						RakBot::app()->log("[RAKBOT] Вход в здание автошколы...");
						SampPickUpPickup(iDoorPickupID, TRUE);
					}
				} else if (vect3_dist(pickups[iDoorPickupID].position, players[localPlayerID].onfootData.position) < 50.0f &&
					(vars.botAutoSchoolActive == 1 || vars.botAutoSchoolEnabled == 0)) {
					RakBot::app()->log("[RAKBOT] Выход из здания автошколы...");
					SampPickUpPickup(iDoorPickupID, TRUE);
				} else if (checkpoint.isActive) {
					RakBot::app()->log("[RAKBOT] Начало сдачи экзамена...");
					GetCheckpoint();

					if (vars.botAutoSchoolFinished)
						vars.botAutoSchoolEnabled = 0;
				}
			}
			dwTime = GetTickCount();
		}
	} */
}

void Flood() {
	Bot *bot = RakBot::app()->getBot();

	if (vars.floodEnabled) {
		static int timer = 0, id = 0;
		if ((int)GetTickCount() - timer > vars.floodDelay) {
			switch (vars.floodMode) {
				case 1:
					bot->sendInput(std::string(vars.floodText));
					break;

				case 2:
					char buf[256];
					snprintf(buf, sizeof(buf), "/sms %d %s", id, vars.floodText.c_str());
					bot->sendInput(std::string(buf));
					id = id < 1000 ? id + 1 : 0;
					break;
			}
			timer = GetTickCount();
		}
	}
}

void CheckPickUp() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.autoPickEnabled || vars.coordMasterEnabled)
		return;

	if (!bot->isSpawned())
		return;

	static int iLastPickup = -1;
	for (int i = 0; i < MAX_PICKUPS; i++) {
		Pickup *pickup = RakBot::app()->getPickup(i);
		if (pickup == nullptr)
			continue;

		if (bot->distanceTo(pickup) < 1.0f) {
			if (i != iLastPickup) {
				if (GetTickCount() - vars.lastChangePos > 3000) {
					bot->pickUpPickup(i, true);
					RakBot::app()->log("[RAKBOT] Автоматическое поднятие пикапа %d", i);
					iLastPickup = i;
				}
			}
			return;
		}
	}
	iLastPickup = -1;
}

void CheckPointMaster() {
	/* Bot *bot = RakBot::getInstance()->getBot();

	if (!vars.checkPointMaster) {
		return;
	}

	DriverSync();

	if (!raceCheckpoint.isActive) {
		return;
	}

	if (localPlayerID == 65535) {
		return;
	}

	if (localVehicleId == 65535) {
		return;
	}

	static uint32_t dwCount = 0;
	static bool bFirstJump = 1;

	if (GetTickCount() < dwCount) {
		return;
	}

	float fDist = vect3_dist(players[localPlayerID].onfootData.position, raceCheckpoint.fCurPos);

	if (fDist > vars.fCoordDist) {
		float fDistXY = vect2_dist(players[localPlayerID].onfootData.position, raceCheckpoint.fCurPos);
		float fDistSin = (raceCheckpoint.fCurPos[0] - players[localPlayerID].onfootData.position[0]) / fDistXY;
		float fDistCos = (raceCheckpoint.fCurPos[1] - players[localPlayerID].onfootData.position[1]) / fDistXY;
		float fHeightSin = fDistXY / fDist;
		float fHeightCos = (raceCheckpoint.fCurPos[2] - 5.0f - players[localPlayerID].onfootData.position[2]) / fDist;

		players[localPlayerID].onfootData.position[0] += fDistSin * fHeightSin * vars.fCoordDist;
		players[localPlayerID].onfootData.position[1] += fDistCos * fHeightSin * vars.fCoordDist;

		if (bFirstJump) {
			players[localPlayerID].onfootData.position[2] -= vars.fCoordDist / 2;
			bFirstJump = 0;
		} else
			players[localPlayerID].onfootData.position[2] += fHeightCos * vars.fCoordDist;
	} else {
		vect2_copy(raceCheckpoint.fCurPos, players[localPlayerID].onfootData.position);
		players[localPlayerID].onfootData.position[2] = raceCheckpoint.fCurPos[2] - 5.0f;
		bFirstJump = 1;
	}

	dwCount = GetTickCount() + vars.iCoordTime; */
}

void SetWork() {
	/* Bot *bot = RakBot::getInstance()->getBot();

	if (!vars.iSetWorkIndex || vars.coordMasterEnabled) {
		return;
	}

	if (localPlayerID == 65535) {
		return;
	}

	static uint32_t dwTimer = 0;

	if (GetTickCount() - dwTimer > 1000) {
		int iPickupID = FindNearestPickup(1318);

		if (pickups[iPickupID].byteActive && pickups[iPickupID].position[2] < 1000.0f) {
			if (pickups[iPickupID].byteActive)
				SampPickUpPickup(iPickupID, TRUE);
		} else
			GetCheckpoint();

		dwTimer = GetTickCount();
	} */
}

void Bank() {
	/* Bot *bot = RakBot::getInstance()->getBot();

	if (vars.iBankPutMoney && !vars.coordMasterEnabled) {
		static uint32_t dwTimer = 0;

		if (GetTickCount() - dwTimer > 1000) {
			if (localPlayerID == 65535) {
				return;
			}

			int iPickupID = FindNearestPickup(1318);
			if (pickups[iPickupID].byteActive && pickups[iPickupID].position[1] < -1500.0f &&
				vect3_dist(pickups[iPickupID].position, players[localPlayerID].onfootData.position) < 50.0f) {
				SampPickUpPickup(iPickupID, TRUE);
			} else if (pickups[iPickupID].byteActive) {
				char buf[512];
				sprintf(buf, "/bank %d", vars.iBankPutMoney);
				SendServerCommand(buf);
				vars.iBankPutMoney = 0;
				if (vars.bQuestEnabled)
					vars.bQuestSpawn = 1;
				Spawn();
			}
			dwTimer = GetTickCount();
		}
	} */
}

void GetSkin() {
	/* Bot *bot = RakBot::getInstance()->getBot();

	if (vars.bBuySkin && !vars.coordMasterEnabled) {
		static uint32_t dwTimer = 0;

		if (GetTickCount() - dwTimer > 1000) {
			if (localPlayerID == 65535) {
				return;
			}

			int iPickupID = FindNearestPickup(1275);
			if (iPickupID != -1) {
				iPickupID--;
				if (pickups[iPickupID].byteActive) {
					SampPickUpPickup(iPickupID, TRUE);
				}
			} else {
				iPickupID = FindNearestPickup(1318);
				if (iPickupID != -1) {
					if (pickups[iPickupID].byteActive)
						SampPickUpPickup(iPickupID, TRUE);
				}
			}
			dwTimer = GetTickCount();
		}
	} */
}

void KeepOnline() {
	Bot *bot = RakBot::app()->getBot();

	if (vars.keepOnlineEnabled) {
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		if (!vars.keepOnlineBeginAfterEnd &&
			(sysTime.wMinute >= vars.keepOnlineBegin && sysTime.wMinute < vars.keepOnlineEnd)) {
			if (vars.keepOnlineWait) {
				vars.keepOnlineWait = false;
			}
		} else if (vars.keepOnlineBeginAfterEnd &&
			(sysTime.wMinute >= vars.keepOnlineBegin || sysTime.wMinute < vars.keepOnlineEnd)) {
			if (vars.keepOnlineWait) {
				vars.keepOnlineWait = false;
			}
		} else {
			if (!vars.keepOnlineWait) {
				vars.keepOnlineWait = true;
				bot->disconnect(false);
				bot->reconnect(vars.reconnectDelay);
			}
		}
	}
}

void SleepAnim() {
	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned() || !vars.sleepEnabled)
		return;

	bot->getAnimation()->setAnimId(390);
	bot->getAnimation()->setAnimFlags(4356);
}

void FuncsLoop() {
	Flood();
	CoordMaster();
	CheckPointMaster();
	BusBot();
	BotLoader();
	FarmerBot();
	GetBalance();
	AutoLicensePass();
	Bank();
	CheckPickUp();
	SetWork();
	Follow();
	Stick();
	GetSkin();
	CheckOnlineAndId();
	SleepAnim();
}

void FuncsOff() {
	BagCount = 0;
	LoaderStep = 0;
	FarmCount = 0;
	FarmGetPay = 0;
	FarmWork = 0;

	vars.sendBadSync = false;
	vars.smartInvis = false;
	raceCheckpoint.active = false;
	checkpoint.active = false;
	vars.coordMasterEnabled = false;
	vars.stickEnabled = false;
	vars.followEnabled = false;
	vars.routeEnabled = false;
	vars.syncAllowed = false;
}