#include "StdAfx.h"

#include "RakBot.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "RakNet.h"
#include "Settings.h"
#include "Pickup.h"
#include "Vehicle.h"
#include "Timer.h"
#include "Events.h"

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

	if (!BotSpawnedTimer.isElapsed(vars.afterSpawnDelay, false))
		return;

	static Timer timer;
	if (!timer.isElapsed(vars.noAfkDelay, true))
		return;

	Bot *bot = RakBot::app()->getBot();
	bot->sync();
}

void RoutePlay() {
	vars.routeIndex = 0;

	while (vars.routeEnabled && !vars.botOff) {
		Sleep(static_cast<uint32_t>(vars.routeSpeed));

		Bot *bot = RakBot::app()->getBot();
		RakClientInterface *rakClient = RakBot::app()->getRakClient();
		BitStream data;

		if (bot->isSpawned()) {
			if (vars.routeIndex >= vars.routeData.size()) {
				if (!vars.routeLoop) {
					vars.routeEnabled = false;
					vars.syncAllowed = true;
					RakBot::app()->log("[RAKBOT] ����������� �������: ����������");
					break;
				} else {
					RakBot::app()->log("[RAKBOT] ����������� �������: ������");
					vars.routeIndex = 0;
				}
			}

			for (int i = 0; i < 4; i++)
				bot->setQuaternion(i, vars.routeData[vars.routeIndex].quaternion[i]);

			for (int i = 0; i < 3; i++)
				bot->setPosition(i, vars.routeData[vars.routeIndex].position[i]);

			for (int i = 0; i < 3; i++)
				bot->setSpeed(i, vars.routeData[vars.routeIndex].speed[i]);

			bot->getAnimation()->setAnimFlags(vars.routeData[vars.routeIndex].animFlags);
			bot->getAnimation()->setAnimId(vars.routeData[vars.routeIndex].animId);
			bot->getKeys()->setKeys(vars.routeData[vars.routeIndex].keys);
			bot->getKeys()->setLeftRightKey(vars.routeData[vars.routeIndex].leftRightKey);
			bot->getKeys()->setUpDownKey(vars.routeData[vars.routeIndex].upDownKey);
			bot->sync();
			vars.routeIndex++;
		}
	}
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

			vars.lastChangePos.reset();
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

	static Timer timer;
	if (!timer.isElapsed(500, true))
		return;

	if (bot->isConnected()) {
		char botInfo[1024];
		snprintf(botInfo, sizeof(botInfo),
			"������: %d\n����: %d\n����������:\n X: %0.2f\n Y: %0.2f\n Z: %0.2f\n��������: %d\n������: %d\n��������: %d/%d\n�������: %d\n������: %s",
			RakBot::app()->getPlayersCount(), bot->getInfo()->getPing(), bot->getPosition(0), bot->getPosition(1), bot->getPosition(2),
			bot->getHealth(), bot->getMoney(),
			bot->getAnimation()->getAnimId(), bot->getAnimation()->getAnimFlags(), bot->getInfo()->getScore(),
			bot->getPlayerStateName().c_str());

		if ((bot->getPlayerState() == PLAYER_STATE_DRIVER) || (bot->getPlayerState() == PLAYER_STATE_PASSENGER)) {
			std::stringstream ss;
			ss << "\n���������: " << bot->getVehicle()->getVehicleId();
			strcat(botInfo, ss.str().c_str());
		}

		SetWindowText(g_hWndTitle, botInfo);
	} else {
		SetWindowText(g_hWndTitle, "RakBot " RAKBOT_VERSION "\n�����: MishaN");
	}
}

void CheckOnlineAndId() {
	Bot *bot = RakBot::app()->getBot();

	if (bot->isConnected()) {
		if (vars.checkIdEnabled) {
			uint16_t playerId = bot->getPlayerId();
			if (playerId > vars.maxId || playerId < vars.minId) {
				RakBot::app()->log("[RAKBOT] ������������ ID (%d). ���������������...", playerId);
				bot->disconnect(false);
				bot->reconnect(vars.reconnectDelay);
			}
		}

		if (!GameInitedTimer.isElapsed(5000, false))
			return;

		if (vars.checkOnlineEnabled) {
			int playersCount = RakBot::app()->getPlayersCount();
			if (playersCount > vars.maxOnline || playersCount < vars.minOnline) {
				RakBot::app()->log("[RAKBOT] ������������ ������ (%d). ���������������...", playersCount);
				bot->disconnect(false);
				bot->reconnect(vars.reconnectDelay);
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
	static Timer timer;
	if (!timer.isElapsed(500, true))
		return;

	Bot *bot = RakBot::app()->getBot();
	if (!bot->isConnected()) {
		SetWindowText(g_hWndAdmins, "�������� �������...");
		SetWindowText(g_hWndAdminsArea, "������ ������");
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
					RakBot::app()->log("[RAKBOT] ����� � ����, ���������������");
					bot->disconnect(false);
					bot->reconnect(vars.adminReconnectDelay);
					return;

				case 2:
					RakBot::app()->log("[RAKBOT] ����� � ����, �����");
					bot->disconnect(false);
					RakBot::app()->exit();
					return;
			}

			char szBuf[128];
			ZeroMemory(szBuf, sizeof(szBuf));

			if (player->isInStream()) {
				switch (vars.adminActionNear) {
					case 1:
						RakBot::app()->log("[RAKBOT] ����� �����, ���������������");
						bot->disconnect(false);
						bot->reconnect(vars.adminReconnectDelay);
						return;

					case 2:
						RakBot::app()->log("[RAKBOT] ����� �����, �����");
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
		SetWindowText(g_hWndAdmins, "��� ������� ������");
		SetWindowText(g_hWndAdminsArea, "������ ������ (0/0)");
	} else {
		SetWindowText(g_hWndAdmins, szAdminList);
		sprintf_s(szAdminList, sizeof(szAdminList), "������ ������ (%d/%d)", iCountNear, iCount);
		SetWindowText(g_hWndAdminsArea, szAdminList);
	}
}

int LoaderStep = -1;
int BagCount = 0;
bool BotWithBag = false;
Timer BotTakenBagTimer = UINT32_MAX;

void BotLoader() {
	static Timer afterGetPayTimer(0);
	if (!afterGetPayTimer.isElapsed(10000, false))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!vars.botLoaderEnabled || !bot->isSpawned() || vars.coordMasterEnabled || SampRpFuncs::isBotSuspended())
		return;

	if (!vars.sendBadSync)
		vars.sendBadSync = true;

	if (LoaderStep == BOTLOADER_STEP_STARTWORK) {
		if (bot->getSkin() == 260 || bot->getSkin() == 16 || bot->getSkin() == 27) {
			LoaderStep = BOTLOADER_STEP_TAKEBAG;
			bot->teleport(2231.10f, -2285.39f, 11.88f);
			return;
		}

		Pickup *pickup = FindNearestPickup(1275);
		if (pickup == nullptr)
			return;

		RakBot::app()->log("[RAKBOT] ������ ������ ��������...");
		SampRpFuncs::pickUpPickup(pickup);
		return;
	}

	if (LoaderStep == BOTLOADER_STEP_TAKEBAG) {
		if (vars.botLoaderCount > 0) {
			if ((BagCount >= vars.botLoaderCount) && (BagCount % vars.botLoaderCount == 0)) {
				bot->teleport(2160.f, -2265.f, 14.08f);
				LoaderStep = BOTLOADER_STEP_GETPAY;
				return;
			}
		}

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

		if (BotWithBag && (BotTakenBagTimer.getTimer() == UINT32_MAX)) {
			bot->teleport(2231.10f, -2285.39f, 11.88f);
			BotTakenBagTimer.reset();
			return;
		}

		if (BotWithBag && (BotTakenBagTimer.isElapsed(vars.botLoaderDelay - 3000, false))) {
			LoaderStep = BOTLOADER_STEP_PUTBAG;
			BotTakenBagTimer.setTimer(UINT32_MAX);
			return;
		}
		return;
	}

	if (LoaderStep == BOTLOADER_STEP_PUTBAG) {
		if (!BotWithBag) {
			LoaderStep = BOTLOADER_STEP_TAKEBAG;
			bot->teleport(2231.10f, -2285.39f, 11.88f);
			return;
		}

		SampRpFuncs::takeCheckpoint();
		LoaderStep = BOTLOADER_STEP_TAKEBAG;
		return;
	}

	if (LoaderStep == BOTLOADER_STEP_GETPAY) {
		Pickup *pickup = FindNearestPickup(1274);
		if (pickup == nullptr)
			return;

		RakBot::app()->log("[RAKBOT] ��������� �� ��������...");
		SampRpFuncs::pickUpPickup(pickup);
		bot->teleport(2231.10f, -2285.39f, 11.88f);
		LoaderStep = BOTLOADER_STEP_TAKEBAG;
		BagCount = 0;
		afterGetPayTimer.reset();
		return;
	}

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
					RakBot::app()->log("[RAKBOT] ��������� �� ��������...");
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
					RakBot::app()->log("[RAKBOT] ����� ����� � ������");
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
				RakBot::app()->log("[RAKBOT] ������ ������ ��������...");
				SampRpFuncs::pickUpPickup(iPickupID);
				LoaderStep = 0;

				if (vars.savedTeleportEnabled) {
					vect3_copy(vars.savedCoords, vars.coordMasterTarget);
					vars.coordMasterEnabled = true;
					RakBot::app()->log("[RAKBOT] �������� �� ����������� ����������");
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
	static Timer timer;
	if (!timer.isElapsed(1500, true))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (bot->getPlayerState() == PLAYER_STATE_DRIVER)
		return;

	if (vars.coordMasterEnabled)
		return;

	if (!vars.busWorkerRoute)
		return;

	bot->sync();

	Vehicle *vehicle = FindNearestVehicle(vars.busWorkerBusModel);
	if (vehicle == nullptr)
		return;

	for (int i = 0; i < 3; i++)
		bot->setPosition(i, vehicle->getPosition(i));
	bot->sync();
	bot->wait(500);
	bot->enterVehicle(vehicle, 0);
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
	static Timer timer;
	if (!timer.isElapsed(1000, true))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!vars.botFarmerEnabled || vars.coordMasterEnabled || !bot->isSpawned() || SampRpFuncs::isBotSuspended())
		return;

	if (!vars.sendBadSync)
		vars.sendBadSync = true;

	if (FarmWork) {
		if (!checkpoint.active) {
			Pickup *pickup = FindNearestPickup(19197);
			if (pickup != nullptr) {
				if (bot->distanceTo(pickup) < 50.0f) {
					bot->pickUpPickup(pickup, FALSE);
					RakBot::app()->log("[RAKBOT] ����� ����� � ������ %d", pickup->getPickupId());
				} else {
					for (int i = 0; i < 3; i++)
						vars.coordMasterTarget[i] = pickup->getPosition(i);
					vars.coordMasterTarget[2] -= 5.0f;
					vars.coordMasterEnabled = true;
					RakBot::app()->log("[RAKBOT] �������� � ������");
				}
			} else {
				static Timer notFoundCarsTimer;
				if (vars.botFarmerAutomated && vars.bQuestEnabled) {
					RakBot::app()->log("[RAKBOT] �� ������� ����� ������! ����� �����...");

					FarmGetPay = 1;
					vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
					vars.coordMasterEnabled = 1;
					FarmWork = 0;
					FarmCount = 0;
					ChangeFarm = 1;
				} else if (notFoundCarsTimer.isElapsed(8000, true)) {
					RakBot::app()->log("[RAKBOT] �� ������� ����� ������!");
					notFoundCarsTimer.reset();
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
				bot->pickUpPickup(pickup, FALSE);
				RakBot::app()->log("[RAKBOT] ���������� �����������...");
			} else {
				for (int i = 0; i < 3; i++)
					vars.coordMasterTarget[i] = pickup->getPosition(i);
				vars.coordMasterTarget[2] -= 5.0f;
				vars.coordMasterEnabled = 1;
				RakBot::app()->log("[RAKBOT] �������� � ����������");
			}
		} else {
			vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
			vars.coordMasterTarget[2] -= 5.0f;
			vars.coordMasterEnabled = 1;
			RakBot::app()->log("[RAKBOT] �������� � ����������");
		}
	}
}

void CoordMaster() {
	static Timer timer;
	if (!timer.isElapsed(vars.coordMasterDelay, true))
		return;

	if (!vars.coordMasterEnabled || !vars.syncAllowed)
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!BotSpawnedTimer.isElapsed(vars.afterSpawnDelay, false))
		return;

	if (!bot->isSpawned())
		return;

	bot->sync();

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
		RakBot::app()->log("[RAKBOT] �� �������� ����� ����������!");

		std::thread coordMasterOffThread([]() {
			Sleep(vars.coordMasterDelay);
			vars.coordMasterEnabled = false;
			RakBot::app()->log("[RAKBOT] ����������� �������� ������");
			RakBot::app()->getEvents()->onCoordMasterComplete();
		});
		coordMasterOffThread.detach();
	}

	for (int i = 0; i < 3; i++)
		bot->setPosition(i, position[i]);
	bot->sync();
}

void GetBalance() {
	static Timer timer;
	if (!timer.isElapsed(3000, true))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!vars.getBalanceEnabled || vars.coordMasterEnabled)
		return;

	bot->sendInput("/atm");
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
	static Timer timer;
	if (!timer.isElapsed(vars.antiAfkDelay, true))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!vars.antiAfkEnabled)
		return;

	static float k = 1.f;

	float offset = (vars.antiAfkOffset != 0.f) ? vars.antiAfkOffset : 0.01f;
	bot->setPosition(0, bot->getPosition(0) + (offset * k));
	bot->sync();

	if (k < 1.f)
		k += 0.5f;
	else
		k *= -1.f;
}

void AutoLicensePass() {
	static Timer timer;
	if (!timer.isElapsed(3000, true))
		return;

	if (!vars.botAutoSchoolEnabled || vars.coordMasterEnabled)
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return;

	if (bot->getPlayerState() != PLAYER_STATE_ONFOOT)
		return;

	if (!raceCheckpoint.active) {
		Pickup *doorPickup = FindNearestPickup(1318);
		if (doorPickup == nullptr)
			return;

		if (bot->distanceTo(doorPickup) < 50.f) {
			if (doorPickup->getPosition(2) < 1000.f) {
				if (vars.botAutoSchoolActive) {
					Vehicle *vehicle = FindNearestVehicle(1, 426, 79, 79);
					if (vehicle != nullptr) {
						bot->enterVehicle(vehicle, 0);
					}
				} else {
					RakBot::app()->log("[RAKBOT] ���� � ������ ���������...");
					bot->pickUpPickup(doorPickup, true);
				}
				return;
			}

			if ((vars.botAutoSchoolActive == 1) || (vars.botAutoSchoolEnabled == 0)) {
				RakBot::app()->log("[RAKBOT] ����� �� ������ ���������...");
				bot->pickUpPickup(doorPickup, true);
				return;
			}

			if (checkpoint.active) {
				RakBot::app()->log("[RAKBOT] ������ ����� ��������...");
				bot->takeCheckpoint();

				if (vars.botAutoSchoolFinished)
					vars.botAutoSchoolEnabled = 0;

				return;
			}
		}
	}
}

void Flood() {
	static Timer timer;
	if (!timer.isElapsed(vars.floodDelay, true))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (vars.floodEnabled) {
		static int id = 0;
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
				if (vars.lastChangePos.isElapsed(1500, false)) {
					bot->pickUpPickup(pickup, true);
					RakBot::app()->log("[RAKBOT] �������������� �������� ������ %d", i);
					iLastPickup = i;
				}
			}
			return;
		}
	}
	iLastPickup = -1;
}

void CheckPointMaster() {
	Bot *bot = RakBot::app()->getBot();

	if (!vars.checkPointMaster || vars.coordMasterEnabled)
		return;

	if (!raceCheckpoint.active && !checkpoint.active)
		return;

	float *pos = checkpoint.active ? checkpoint.position : raceCheckpoint.position;
	DoCoordMaster(true, pos[0], pos[1], pos[2] - 5.f);
}

void SetWork() {
	static Timer timer;
	if (!timer.isElapsed(1000, true))
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!vars.iSetWorkIndex || vars.coordMasterEnabled)
		return;

	if (!bot->isSpawned())
		return;

	Pickup *pickup = FindNearestPickup(1318);
	if (pickup != nullptr) {
		if (pickup->getPosition(2) < 1000.0f)
			bot->pickUpPickup(pickup);
	} else {
		bot->takeCheckpoint();
	}
}

void Bank() {
	static Timer timer;
	if (!timer.isElapsed(1000, true))
		return;

	if (!vars.iBankPutMoney || vars.coordMasterEnabled)
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return;

	Pickup *pickup = FindNearestPickup(1318);
	if (pickup == nullptr)
		return;

	if (pickup->getPosition(1) < -1500.0f) {
		bot->pickUpPickup(pickup, true);
		return;
	}

	std::string cmd = "/bank " + std::to_string(vars.iBankPutMoney);
	bot->sendInput(cmd);
	vars.iBankPutMoney = 0;
	if (vars.bQuestEnabled)
		vars.bQuestSpawn = 1;
	bot->spawn();
}

void GetSkin() {
	static Timer timer;
	if (!timer.isElapsed(1000, true))
		return;

	if (!vars.bBuySkin || vars.coordMasterEnabled)
		return;

	Bot *bot = RakBot::app()->getBot();

	if (!bot->isSpawned())
		return;

	Pickup *pickup;

	pickup = FindNearestPickup(1275);
	if (pickup != nullptr) {
		bot->pickUpPickup(pickup, true);
		return;
	}

	pickup = FindNearestPickup(1318);
	if (pickup != nullptr) {
		bot->pickUpPickup(RakBot::app()->getPickup(pickup->getPickupId() - 1), true);
		return;
	}
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